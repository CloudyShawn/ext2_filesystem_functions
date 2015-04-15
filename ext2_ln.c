/*ext2_ln: 
This program takes three command line arguments. 
	The first is the name of an ext2 formatted virtual disk. 
	The other two are absolute paths on your ext2 formatted disk. 

The program should work like ln, creating a link from the first specified file 
to the second specified path. 

If the first location does not exist (ENOENT), if the second location already exists (EEXIST), 
or if either location refers to a directory (EISDIR), then your program should return the appropriate error. 

Note that this version of ln only works with files.*/

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
	if (argc != 4)
	{
		printf("Usage: ext2_ln <virtual_disk> <source_virtual_path> <dest_virtual_path>\n");
		return 0;
	}

	int fd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}
	// Get source and path file inode
	struct ext2_inode *sourceFile = inode_get(argv[2], disk );
	struct ext2_inode *destFile = inode_get(argv[3], disk );

	//error checking
	if (sourceFile == NULL) {
		printf("No such file or directory\n");
		exit(ENOENT);
	}
	else if ((destFile != NULL) && (destFile->i_mode & EXT2_S_IFREG)) {
		printf("Destination already exists\n");
		exit(EEXIST);
	}
	else if ((sourceFile->i_mode & EXT2_S_IFDIR) || (destFile != NULL && (destFile->i_mode & EXT2_S_IFDIR))) {
		printf("Cannot link directories\n");
		exit(EISDIR);
	}

	//no errors, create link
	struct ext2_inode *parentDir = inode_get(get_parent_path(argv[2]), disk);
	unsigned int *sourceBlock = parentDir->i_block;
	char *sourceName = &(argv[2][strlen(get_parent_path(argv[2]))]);
	int i;
	struct ext2_dir_entry_2 *sourceEntry;
	//get source file directory entry
	for (i = 0; i < 12 && sourceBlock[i]; i++) {
		unsigned int total_size = EXT2_BLOCK_SIZE;
        		struct ext2_dir_entry_2 *de = (struct ext2_dir_entry_2 *)(disk + ((sourceBlock[i] ) * EXT2_BLOCK_SIZE));
   		while(total_size > 0) {
			if (strncmp(sourceName, de->name, de->name_len) == 0) {
				sourceEntry = de;
				break;
			}
       		
       		total_size -= de->rec_len;
      		de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
    		}
    		
	    	if (total_size >0) {
				break;
			}
	}

	//get destination file's parent inode
	struct ext2_inode *destParent = inode_get(get_parent_path(argv[3]), disk);
	unsigned int *destBlock = destParent->i_block;
	char *destName = &(argv[3][strlen(get_parent_path(argv[3]))]);
	struct ext2_dir_entry_2 *de;
	unsigned int total_size;
	//find unused directory entry and create link
	for (i = 0; i < 12 && destBlock[i]; i++) {
		total_size = EXT2_BLOCK_SIZE;
        de = (struct ext2_dir_entry_2 *)(disk + (destBlock[i] * EXT2_BLOCK_SIZE));
       		
   		while(total_size) {
			int rounding_error = 0;
			if(strlen(destName) % 4)
			{
				rounding_error += 4 - (strlen(destName) % 4);
			}
			if(de->name_len % 4)
			{
				rounding_error += 4 - (de->name_len % 4);
			}

			if(de->rec_len >= (2 * sizeof(struct ext2_dir_entry_2)) + strlen(destName) + de->name_len + rounding_error)
			{
				break;
			}
       		
       		total_size -= de->rec_len;
      		de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
    	}
    		
    	if (total_size > 0) {
			break;
		}
	}

	if(destBlock[i])
	{
		unsigned int prev_size = sizeof(struct ext2_dir_entry_2) + de->name_len + (de->name_len % 4);
		struct ext2_dir_entry_2 *new_dir_entry = (struct ext2_dir_entry_2 *)((char *)de + prev_size);
		new_dir_entry->inode = sourceEntry->inode;
		new_dir_entry->rec_len = de->rec_len - prev_size;
		new_dir_entry->name_len = strlen(destName);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, destName, new_dir_entry->name_len);

		de->rec_len = prev_size;
	}
	else
	{
		destBlock[i] = first_free_block(disk);
		add_block_bitmap(destBlock[i], disk);
		struct ext2_dir_entry_2 *new_dir_entry = (struct ext2_dir_entry_2 *)(disk + (destBlock[i] * EXT2_BLOCK_SIZE));
		new_dir_entry->inode = sourceEntry->inode;
		new_dir_entry->rec_len = EXT2_BLOCK_SIZE;
		new_dir_entry->name_len = strlen(destName);
		new_dir_entry->file_type = EXT2_FT_DIR;
		strncpy(new_dir_entry->name, destName, new_dir_entry->name_len);

		destParent->i_size += EXT2_BLOCK_SIZE;
		destParent->i_blocks += 2;
	}	

	//link created, update link_count for source inode
	sourceFile->i_links_count++;
	return 0;
}