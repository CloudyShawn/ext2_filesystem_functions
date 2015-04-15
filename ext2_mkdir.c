/*ext2_mkdir: 
This program takes two command line arguments. 
	The first is the name of an ext2 formatted virtual disk. 
	The second is an absolute path on your ext2 formatted disk. 

The program should work like mkdir, creating the final directory on the specified path on the disk. 

If any component on the path to the location where the final directory is to be created does not exist or 
if the specified directory already exists, then your program should return the appropriate error (ENOENT or EEXIST).*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include "ext2_functions.h"

unsigned char *disk;

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: ext2_mkdir <virtual_disk> <virtual_path>\n");
		exit(1);
	}

	int fd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}

	struct ext2_inode *parent = inode_get(get_parent_path(argv[2]), disk);
	if(!parent)
	{
		printf("No such path exists\n");
		exit(ENOENT);
	}
	if(inode_get(argv[2], disk))
	{
		printf("Directory/File with that name already exists\n");
		exit(EEXIST);
	}

	//get and set inode
	unsigned int new_inode_num = first_free_inode(disk);
	add_inode_bitmap(new_inode_num, disk);

	struct ext2_group_desc *gd = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
	struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (gd->bg_inode_table * EXT2_BLOCK_SIZE));
	struct ext2_inode *new_inode = &(inode_table[new_inode_num - 1]);

	new_inode->i_mode = EXT2_S_IFDIR;

	//give it a block
	new_inode->i_size = EXT2_BLOCK_SIZE;
	new_inode->i_links_count = 2;
	new_inode->i_blocks = 2;
	unsigned int new_block_num = first_free_block(disk);
	add_block_bitmap(new_block_num, disk);
	new_inode->i_block[0] = new_block_num;
	int i;
	for(i = 1; i < 15; i++)
	{
		new_inode->i_block[i] = 0;
	}

	//set first dir_entry to itself
	struct ext2_dir_entry_2 *new_dir_entry = (struct ext2_dir_entry_2 *)(disk + (new_block_num * EXT2_BLOCK_SIZE));
	new_dir_entry->inode = new_inode_num;
	new_dir_entry->rec_len = sizeof(struct ext2_dir_entry_2) + 4;
	new_dir_entry->name_len = 1;
	new_dir_entry->file_type = EXT2_FT_DIR;
	strncpy(new_dir_entry->name, ".", 1);

	//set another dir_entry to = parent
	new_dir_entry = (struct ext2_dir_entry_2 *)((char *)new_dir_entry + new_dir_entry->rec_len);
	new_dir_entry->inode = ((struct ext2_dir_entry_2 *)(disk + (parent->i_block[0] * EXT2_BLOCK_SIZE)))->inode;
	new_dir_entry->rec_len = EXT2_BLOCK_SIZE - (sizeof(struct ext2_dir_entry_2) + 4);
	new_dir_entry->name_len = 2;
	new_dir_entry->file_type = EXT2_FT_DIR;
	strncpy(new_dir_entry->name, "..", 2);
	parent->i_links_count++;

	//find empty dir_entry in parent and set
	unsigned int total_size;
	struct ext2_dir_entry_2 *prev_entry;
	char *sourceName = &(argv[2][strlen(get_parent_path(argv[2]))]);
	for(i = 0; i < 12 && parent->i_block[i]; i++)
	{
		prev_entry = (struct ext2_dir_entry_2 *)(disk + (parent->i_block[i] * EXT2_BLOCK_SIZE));
		total_size = EXT2_BLOCK_SIZE;
		while(total_size)
		{
			int rounding_error = 0;
			if(strlen(sourceName) % 4)
			{
				rounding_error += 4 - (strlen(sourceName) % 4);
			}
			if(prev_entry->name_len % 4)
			{
				rounding_error += 4 - (prev_entry->name_len % 4);
			}

			if(prev_entry->rec_len >= (2 * sizeof(struct ext2_dir_entry_2)) + strlen(sourceName) + prev_entry->name_len + rounding_error)
			{
				break;
			}
			total_size -= prev_entry->rec_len;
			prev_entry = (struct ext2_dir_entry_2 *)((char *)prev_entry + prev_entry->rec_len);
		}
		if(total_size)
		{
			break;
		}
	}

	if(parent->i_block[i])
	{
		unsigned int prev_size = sizeof(struct ext2_dir_entry_2) + prev_entry->name_len;
		if(prev_entry->name_len % 4)
		{
			prev_size += 4 - (prev_entry->name_len % 4);
		}

		new_dir_entry = (struct ext2_dir_entry_2 *)((char *)prev_entry + prev_size);
		new_dir_entry->inode = new_inode_num;
		new_dir_entry->rec_len = prev_entry->rec_len - prev_size;
		new_dir_entry->name_len = strlen(sourceName);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, sourceName, new_dir_entry->name_len);

		prev_entry->rec_len = prev_size;
	}
	else
	{
		parent->i_block[i] = first_free_block(disk);
		add_block_bitmap(parent->i_block[i], disk);
		new_dir_entry = (struct ext2_dir_entry_2 *)(disk + (parent->i_block[i] * EXT2_BLOCK_SIZE));
		new_dir_entry->inode = new_inode_num;
		new_dir_entry->rec_len = EXT2_BLOCK_SIZE;
		new_dir_entry->name_len = strlen(sourceName);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, sourceName, new_dir_entry->name_len);

		parent->i_size += EXT2_BLOCK_SIZE;
		parent->i_blocks += 2;
	}	

	return 0;
}