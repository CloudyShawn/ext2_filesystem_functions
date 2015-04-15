/*ext2_rm: 
This program takes two command line arguments. 
	The first is the name of an ext2 formatted virtual disk 
	The second is an absolute path to a file or link (not a directory) on that disk. 

The program should work like rm, removing the specified file from the disk. 

If the file does not exist or if it is a directory, then your program should return the appropriate error.*/

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
		printf("Usage: ext2_rm <virtual_disk> <virtual_path>\n");
		return 0;
	}

	int fd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}
	// Getting inode for file that was asked for
	struct ext2_inode *currentFile = inode_get(argv[2], disk );
	if (currentFile == NULL) {
		printf("No such file or directory\n");
		exit(ENOENT);
	}
	else if (currentFile->i_mode & EXT2_S_IFDIR) {
		printf("Cannot remove directory\n");
		exit(ENOENT);
	}
	//path is valid, removing file
	currentFile->i_links_count--;
	//remove parent link
	struct ext2_inode *parentDir = inode_get(get_parent_path(argv[2]), disk);
	char *fileName = &(argv[2][strlen(argv[2]) - (strlen(argv[2]) - strlen(get_parent_path(argv[2])))]);
	unsigned int *currentBlock = parentDir->i_block;
	unsigned int inodeNum;
	int i;
	for (i = 0; i < 12 && currentBlock[i]; i++) {
		unsigned int total_size = EXT2_BLOCK_SIZE;
		struct ext2_dir_entry_2 *de = (struct ext2_dir_entry_2 *)(disk + (currentBlock[i] * EXT2_BLOCK_SIZE));
       		while(total_size > 0) {
				if (strncmp(fileName, de->name, de->name_len) == 0) {
					struct ext2_dir_entry_2 *preventry = (struct ext2_dir_entry_2 *)(disk + (currentBlock[i] * EXT2_BLOCK_SIZE));
					while((struct ext2_dir_entry_2 *)((char *)preventry + preventry->rec_len) != de)
					{
						preventry = (struct ext2_dir_entry_2 *)((char *)preventry + preventry->rec_len);
					}
					preventry->rec_len += de->rec_len;
					inodeNum = de->inode;
					memset(de, 0, sizeof(struct ext2_dir_entry_2));
					break;

			}
     			total_size -= de->rec_len;
    			de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
		}
		if (total_size >0) {
			break;
		}


	}



	currentBlock = currentFile->i_block;
	// if no links exist then set inode = 0, change bitmaps
	if (currentFile->i_links_count == 0) {
		//removing inode bit from inode bitmap
		rem_inode_bitmap(inodeNum, disk);
		//removing inode block bits from block bitmap
		for (i = 0; i < 12 && currentBlock[i]; i++) {
			rem_block_bitmap(currentBlock[i], disk);
		}

		if (currentBlock[i]) {
			int j;
			unsigned int *single_indirect = (unsigned int *) (disk +  currentBlock[i] * EXT2_BLOCK_SIZE);
			for (j = 0; single_indirect[j] ; j++) {
				rem_block_bitmap(single_indirect[j], disk);
			}
			rem_block_bitmap(currentBlock[i], disk);
		}
		//set inode to 0
		currentFile = 0;
	}

	return 0;
}