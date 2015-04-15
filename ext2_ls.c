/*ext2_ls: 
This program takes two command line arguments. 
	The first is the name of an ext2 formatted virtual disk. 
	The second is an absolute path on the ext2 formatted disk. 

The program should work like ls -1, printing each directory entry on a separate line. 
Unlike ls -1, it should also print . and ... In other words, 
it will print one line for every directory entry in the directory specified by the absolute path. 
If the directory does not exist print "No such file or diretory".*/

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

int main(int argc, char *argv[]) {
	if (argc != 3) {
	    printf("Usage: ext2_ls <virtual_disk> <path>\n");
	    exit(1);
	}
	int fd = open(argv[1], O_RDWR);

	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(disk == MAP_FAILED) {
	    perror("mmap");
	    exit(1);
	}
	// Getting current directory inode
	struct ext2_inode *currentDir = inode_get(argv[2], disk );
	if (currentDir == NULL ) {
		printf("No such file or directory\n");
		exit(ENOENT);
	}
	else if (currentDir->i_mode & EXT2_S_IFREG) {
		printf("%s\n", &(argv[2][strlen(get_parent_path(argv[2]))]));
		return 0;
	}


	unsigned int *currentBlock = currentDir->i_block;
	int i = 0;
	// Getting direct block names
	for (i = 0; i < 12 && currentBlock[i]; i++) {
		unsigned int total_size = EXT2_BLOCK_SIZE;
        		struct ext2_dir_entry_2 *de = (struct ext2_dir_entry_2 *)(disk + (currentBlock[i] * EXT2_BLOCK_SIZE));
       		while(total_size > 0) {
 			char dirName[256];
 			strncpy(dirName, de->name, (int)de->name_len);
 			dirName[(int)de->name_len] = '\0';
            			printf("%s\n", dirName);
             		total_size -= de->rec_len;
            			de = (struct ext2_dir_entry_2 *)((char *)de + de->rec_len);
        		}
		
	}

	return 0;
}