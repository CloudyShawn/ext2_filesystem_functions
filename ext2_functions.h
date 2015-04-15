#include <stdio.h>
#include <stdlib.h>

#include "ext2.h"

// finds the inode at dirName in ext2 filesystem mounted at disk
struct ext2_inode *inode_get(char * dirName, unsigned char *disk );

// Splits the path to up to last occurance of "/"
char *get_parent_path(char* dirName);

// Add and removes blocks/inodes from bitmap
void rem_block_bitmap(unsigned int block_num, unsigned char *disk);
void add_block_bitmap(unsigned int block_num, unsigned char *disk);
void rem_inode_bitmap(unsigned int inode_num, unsigned char *disk);
void add_inode_bitmap(unsigned int inode_num, unsigned char *disk);

// Finds first free inode and block number
unsigned int first_free_inode(unsigned char *disk);
unsigned int first_free_block(unsigned char *disk);

