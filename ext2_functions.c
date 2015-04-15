#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ext2.h"
#include "ext2_functions.h"

struct ext2_inode *inode_get(char *path, unsigned char *disk)
{
    char *delim = "/";

    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
    struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (EXT2_BLOCK_SIZE * group_desc->bg_inode_table));

    struct ext2_inode *cur_inode = &(inode_table[EXT2_ROOT_INO - 1]);

    char *parentPath = malloc(sizeof(char) * (strlen(path) + 1));
    strncpy(parentPath, path, strlen(path));

    char *split_path = strtok(parentPath, delim);
    unsigned int block_num = 0;
    struct ext2_dir_entry_2 *dir_entry; 
    int total_size;
    while(split_path != NULL && block_num < 12 && (cur_inode->i_mode & EXT2_S_IFDIR) && cur_inode->i_block[block_num])
    {	
    	dir_entry = (struct ext2_dir_entry_2 *)(disk + (cur_inode->i_block[block_num] * EXT2_BLOCK_SIZE));
    	total_size = EXT2_BLOCK_SIZE;
    	while(total_size)
    	{
    		if(!strncmp(split_path, dir_entry->name, dir_entry->name_len))
    		{
    			cur_inode = &(inode_table[dir_entry->inode - 1]);
    			block_num = 0;
    			split_path = strtok(NULL, delim);
    			total_size = EXT2_BLOCK_SIZE;
    			break;
    		}
			total_size -= dir_entry->rec_len;
			dir_entry = (struct ext2_dir_entry_2 *)((char *)dir_entry + dir_entry->rec_len);
    	}

        if(total_size == 0)
        {
            block_num++;
        }
    }

    if(!split_path)
    {
        return cur_inode;
    }
    else
    {
        return NULL;
    }
}

void rem_block_bitmap(unsigned int block_num, unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    super_block->s_free_blocks_count++;

    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
    char *bitmap = (char *)(disk + (group_desc->bg_block_bitmap * EXT2_BLOCK_SIZE));
    char byte = bitmap[(block_num - 1) / 8];
    int pos = (block_num - 1) % 8;

    bitmap[(block_num - 1) / 8] = byte & ~(1 << pos);
}

void add_block_bitmap(unsigned int block_num, unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    super_block->s_free_blocks_count--;
    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
    char *bitmap = (char *)(disk + (group_desc->bg_block_bitmap * EXT2_BLOCK_SIZE));
    char byte = bitmap[(block_num - 1) / 8];
    int pos = (block_num - 1) % 8;

    bitmap[(block_num - 1) / 8] = byte | (1 << pos);
}

void rem_inode_bitmap(unsigned int inode_num, unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    super_block->s_free_inodes_count++;
    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
    char *bitmap = (char *)(disk + (group_desc->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    char byte = bitmap[(inode_num - 1) / 8];
    int pos = (inode_num - 1) % 8;

    bitmap[(inode_num - 1) / 8] = byte & ~(1 << pos);
}

void add_inode_bitmap(unsigned int inode_num, unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    super_block->s_free_inodes_count--;
    struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
    char *bitmap = (char *)(disk + (group_desc->bg_inode_bitmap * EXT2_BLOCK_SIZE));
    char byte = bitmap[(inode_num - 1) / 8];
    int pos = (inode_num - 1) % 8;

    bitmap[(inode_num - 1) / 8] = byte | (1 << pos);
}

unsigned int first_free_block(unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    if(super_block->s_free_blocks_count)
    {
        struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
        char *bitmap = (char *)(disk + (group_desc->bg_block_bitmap * EXT2_BLOCK_SIZE));
        int i = 0;
        int j = 0;
        while (i < super_block->s_blocks_count / 8)        
        {
            while(j < 8)
            {
                if((int)((bitmap[i] >> j) & 1) == 0)
                {
                    break;
                }
                j++;
            }
            if (j < 8)
            {
                break;
            }
            j = 0;
            i++;
        }

        return (i * 8) + j + 1;
    }
    return 0;
}

unsigned int first_free_inode(unsigned char *disk)
{
    struct ext2_super_block *super_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
    if(super_block->s_free_inodes_count)
    {
        struct ext2_group_desc *group_desc = (struct ext2_group_desc *)(disk + (2 * EXT2_BLOCK_SIZE));
        char *bitmap = (char *)(disk + (group_desc->bg_inode_bitmap * EXT2_BLOCK_SIZE));
        int i = 1;
        int j = (EXT2_GOOD_OLD_FIRST_INO - 1) % 8;
        while (i < super_block->s_inodes_count / 8)
        {
            while(j < 8)
            {
                if((int)((bitmap[i] >> j) & 1) == 0)
                {
                    break;
                }
                j++;
            }
            if (j < 8)
            {
                break;
            }
            j = 0;
            i++;
        }

        return (i * 8) + j + 1;
    }
    return 0;
}

char *get_parent_path(char *dirName) {
    int i;
    char *parentPath = malloc(sizeof(char) * strlen(dirName));
    strncpy(parentPath, dirName, strlen(dirName));
    for (i = strlen(dirName) - 2; i >= 0; i--)
    {
        if (parentPath[i] == '/') {
            parentPath[i + 1] = '\0';
            break;
        }
    }
    return parentPath;
}