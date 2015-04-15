/*ext2_cp: 
This program takes three command line arguments. 
	The first is the name of an ext2 formatted virtual disk. 
	The second is the path to a file on your native operating system, 
	The third is an absolute path on your ext2 formatted disk. 

The program should work like cp, copying the file on your native file system 
onto the specified location on the disk. If the specified file or target location does not exist, 
then your program should return the appropriate error (ENOENT). */

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
	//check usage of file is correct
	if (argc != 4)
	{
		printf("Usage: ext2_cp <virtual_disk> <source_file_path> <dest_virtual_path>\n");
		return 0;
	}

	// attemps to open map out disk image
	int vfd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, vfd, 0);
	if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}

	// Declares super block and the group descriptor
	struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));

	// Get directory to place new file
	struct ext2_inode *parent = inode_get(get_parent_path(argv[3]), disk);

	// Opens existing file on physical system
	FILE *fd = fopen(argv[2], "r");
	// Check if file exists
	if(fd == NULL)
	{
		printf("Specified file already exists\n");
		exit(ENOENT);
	}
	//Check if directory to place new file exists
	if(parent == NULL)
	{
		printf("Target directory doesn't exist\n");
		exit(ENOENT);
	}
	// Check that new file does not already exist
	if(inode_get(argv[3], disk))
	{
		printf("Target file already exists\n");
		exit(EEXIST);
	}

	// Jump to end of file to grab size
	fseek(fd, 0L, SEEK_END);
	long int file_size = ftell(fd);

	// Calculate how many needed blocks
	int num_blocks = file_size / EXT2_BLOCK_SIZE;
	if(file_size % EXT2_BLOCK_SIZE)
	{
		num_blocks++;
	}
	if(num_blocks > 12)
	{
		num_blocks++;
	}

	// Checks that there is enough free blocks
	if(num_blocks > super_block->s_free_blocks_count)
	{
		printf("Not enough space on device\n");
		exit(ENOSPC);
	}

	// Returns the file pointer 
	fseek(fd, 0L, SEEK_SET);

	struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (group_desc->bg_inode_table * EXT2_BLOCK_SIZE));

	// Assign new inode and mark as used
	unsigned int new_inode_num = first_free_inode(disk);
	add_inode_bitmap(new_inode_num, disk);
	struct ext2_inode *new_inode = &(inode_table[new_inode_num - 1]);

	// Allocate new inode
	new_inode->i_mode = EXT2_S_IFREG;
	new_inode->i_size = file_size;
	new_inode->i_links_count = 1;
	new_inode->i_blocks = 2 * num_blocks;
	int i;
	for(i = 0; i < 15; i++)
	{
		new_inode->i_block[i] = 0;
	}

	// Allocate all needed blocks
	for(i = 0; i < 13 && i < num_blocks; i++)
	{
		new_inode->i_block[i] = first_free_block(disk);
		add_block_bitmap(new_inode->i_block[i], disk);
	}
	if(i == 13)
	{
		unsigned int *indirect_block = (unsigned int *)(disk + (new_inode->i_block[12] * EXT2_BLOCK_SIZE));
		int j;
		for(j = 0; i < num_blocks; i++, j++)
		{
			indirect_block[j] = first_free_block(disk);
			add_block_bitmap(indirect_block[j], disk);
		}
	}

	// Copy info into blocks
	void *block;
	for(i = 0; i < num_blocks && i < 12; i++)
	{
		block = (void *)(disk + (new_inode->i_block[i] * EXT2_BLOCK_SIZE));
		fread(block, sizeof(char), EXT2_BLOCK_SIZE / sizeof(char), fd);
	}
	if(num_blocks > 12)
	{
		unsigned *indirect_block = (unsigned int *)(disk + (new_inode->i_block[i++] * EXT2_BLOCK_SIZE));
		int j;
		for(j = 0; i < num_blocks; i++, j++)
		{
			block = (void *)(disk + (indirect_block[j] * EXT2_BLOCK_SIZE));
			fread(block, sizeof(char), EXT2_BLOCK_SIZE / sizeof(char), fd);
		}
	}

	// Create a directory entry in parent directory
	unsigned int total_size;
	struct ext2_dir_entry_2 *prev_entry;
	struct ext2_dir_entry_2 *new_dir_entry;
	char *taget_name = &(argv[3][strlen(get_parent_path(argv[3]))]);
	for(i = 0; i < 12 && parent->i_block[i]; i++)
	{
		prev_entry = (struct ext2_dir_entry_2 *)(disk + (parent->i_block[i] * EXT2_BLOCK_SIZE));
		total_size = EXT2_BLOCK_SIZE;
		while(total_size)
		{
			if(prev_entry->rec_len >= (2 * sizeof(struct ext2_dir_entry_2)) + strlen(taget_name) + (4 - (strlen(taget_name) % 4)) + prev_entry->name_len + (4 - (prev_entry->name_len % 4)))
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

	// If found space
	if(parent->i_block[i])
	{
		unsigned int prev_size = sizeof(struct ext2_dir_entry_2) + prev_entry->name_len + (prev_entry->name_len % 4);
		new_dir_entry = (struct ext2_dir_entry_2 *)((char *)prev_entry + prev_size);
		new_dir_entry->inode = new_inode_num;
		new_dir_entry->rec_len = prev_entry->rec_len - prev_size;
		new_dir_entry->name_len = strlen(taget_name);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, taget_name, new_dir_entry->name_len);

		prev_entry->rec_len = prev_size;
	}
	// Add block if no space in any used block
	else
	{
		parent->i_block[i] = first_free_block(disk);
		add_block_bitmap(parent->i_block[i], disk);
		new_dir_entry = (struct ext2_dir_entry_2 *)(disk + (parent->i_block[i] * EXT2_BLOCK_SIZE));
		new_dir_entry->inode = new_inode_num;
		new_dir_entry->rec_len = EXT2_BLOCK_SIZE;
		new_dir_entry->name_len = strlen(taget_name);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, taget_name, new_dir_entry->name_len);

		parent->i_size += EXT2_BLOCK_SIZE;
		parent->i_blocks += 2;
	}	

	return 0;
}