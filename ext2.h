/* MODIFIED by Karen Reid for CSC369
 * to remove some of the unnecessary components */

/*
 * Copyright (C) 1992, 1993, 1994, 1995
 * Remy Card (card@masi.ibp.fr)
 * Laboratoire MASI - Institut Blaise Pascal
 * Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/include/linux/minix_fs.h
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#ifndef EXT2_H
 #define EXT2_H

/*
 * Special inode numbers
 */
#define	EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */

#define EXT2_GOOD_OLD_FIRST_INO	11

#define EXT2_BLOCK_SIZE     1024
#define	EXT2_ADDR_PER_BLOCK EXT2_BLOCK_SIZE / sizeof (unsigned int))


/*
 * Macro-instructions used to manage fragments
 */
#define EXT2_MIN_FRAG_SIZE		1024
#define	EXT2_MAX_FRAG_SIZE		4096
#define EXT2_MIN_FRAG_LOG_SIZE		  10
#define EXT2_FRAG_SIZE(s)		(EXT2_SB(s)->s_frag_size)
#define EXT2_FRAGS_PER_BLOCK(s)		(EXT2_SB(s)->s_frags_per_block)

/*
 * Structure of a blocks group descriptor
 */
struct ext2_group_desc
{
	unsigned int	bg_block_bitmap;		/* Blocks bitmap block */
	unsigned int	bg_inode_bitmap;		/* Inodes bitmap block */
	unsigned int	bg_inode_table;		/* Inodes table block */
	unsigned short	bg_free_blocks_count;	/* Free blocks count */
	unsigned short	bg_free_inodes_count;	/* Free inodes count */
	unsigned short	bg_used_dirs_count;	/* Directories count */
	unsigned short	bg_pad;
	unsigned int	bg_reserved[3];
};

#define EXT2_S_IFREG	0x8000	/* regular file */
#define EXT2_S_IFDIR	0x4000	/* directory */



/*
 * Structure of an inode on the disk
 */
struct ext2_inode {
	unsigned short	i_mode;		/* File mode */
	unsigned short	i_uid;		/* Low 16 bits of Owner Uid */
	unsigned int	i_size;		/* Size in bytes */
	unsigned int	i_atime;	/* Access time */
	unsigned int	i_ctime;	/* Creation time */
	unsigned int	i_mtime;	/* Modification time */
	unsigned int	i_dtime;	/* Deletion Time */
	unsigned short	i_gid;		/* Low 16 bits of Group Id */
	unsigned short	i_links_count;	/* Links count */
	unsigned int	i_blocks;	/* Blocks count IN DISK SECTORS*/
	unsigned int	i_flags;	/* File flags */
    unsigned int    osd1;		/* OS dependent 1 */
	unsigned int	i_block[15];/* Pointers to blocks */
    unsigned int  i_generation;   /* File version (for NFS) */
    unsigned int  i_file_acl;     /* File ACL */
    unsigned int  i_dir_acl;      /* Directory ACL */
    unsigned int  i_faddr;        /* Fragment address */
    unsigned int extra[3];
};



/*
 * Maximal mount counts between two filesystem checks
 */
#define EXT2_DFL_MAX_MNT_COUNT		20	/* Allow 20 mounts */
#define EXT2_DFL_CHECKINTERVAL		0	/* Don't use interval check */

/*
 * Behaviour when detecting errors
 */
#define EXT2_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT2_ERRORS_RO			2	/* Remount fs read-only */
#define EXT2_ERRORS_PANIC		3	/* Panic */
#define EXT2_ERRORS_DEFAULT		EXT2_ERRORS_CONTINUE

/*
 * Structure of the super block
 */
struct ext2_super_block {
	unsigned int	s_inodes_count;		/* Inodes count */
	unsigned int	s_blocks_count;		/* Blocks count */
	unsigned int	s_r_blocks_count;	/* Reserved blocks count */
	unsigned int	s_free_blocks_count;	/* Free blocks count */
	unsigned int	s_free_inodes_count;	/* Free inodes count */
	unsigned int	s_first_data_block;	/* First Data Block */
	unsigned int	s_log_block_size;	/* Block size */
	unsigned int	s_log_frag_size;	/* Fragment size */
	unsigned int	s_blocks_per_group;	/* # Blocks per group */
	unsigned int	s_frags_per_group;	/* # Fragments per group */
	unsigned int	s_inodes_per_group;	/* # Inodes per group */
	unsigned int	s_mtime;		/* Mount time */
	unsigned int	s_wtime;		/* Write time */
	unsigned int	s_mnt_count;		/* Mount count */
	unsigned short	s_max_mnt_count;	/* Maximal mount count */
	unsigned short	s_magic;		/* Magic signature */
	unsigned short	s_state;		/* File system state */
	unsigned short	s_errors;		/* Behaviour when detecting errors */
	unsigned short	s_minor_rev_level; 	/* minor revision level */
	unsigned int	s_lastcheck;		/* time of last check */
	unsigned int	s_checkinterval;	/* max. time between checks */
	unsigned int	s_creator_os;		/* OS */
	unsigned int	s_rev_level;		/* Revision level */
	unsigned short	s_def_resuid;		/* Default uid for reserved blocks */
	unsigned short	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 * 
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	unsigned int	s_first_ino; 		/* First non-reserved inode */
	unsigned short   s_inode_size; 		/* size of inode structure */
	unsigned short	s_block_group_nr; 	/* block group # of this superblock */
	unsigned int	s_feature_compat; 	/* compatible feature set */
	unsigned int	s_feature_incompat; 	/* incompatible feature set */
	unsigned int	s_feature_ro_compat; 	/* readonly-compatible feature set */
	unsigned char	s_uuid[16];		/* 128-bit uuid for volume */
	char	s_volume_name[16]; 	/* volume name */
	char	s_last_mounted[64]; 	/* directory where last mounted */
	unsigned int	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	unsigned char	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	unsigned char	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	unsigned short	s_padding1;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	unsigned char	s_journal_uuid[16];	/* uuid of journal superblock */
	unsigned int	s_journal_inum;		/* inode number of journal file */
	unsigned int	s_journal_dev;		/* device number of journal file */
	unsigned int	s_last_orphan;		/* start of list of inodes to delete */
	unsigned int	s_hash_seed[4];		/* HTREE hash seed */
	unsigned char	s_def_hash_version;	/* Default hash version to use */
	unsigned char	s_reserved_char_pad;
	unsigned short	s_reserved_word_pad;
 	unsigned int	s_first_meta_bg; 	/* First metablock block group */
	unsigned int	s_reserved[190];	/* Padding to the end of the block */
};

/*
 * Codes for operating systems
 */
#define EXT2_OS_LINUX		0


struct ext2_dir_entry {
	unsigned int	inode;			/* Inode number */
	unsigned short	rec_len;		/* Directory entry length */
	unsigned short	name_len;		/* Name length */
	char	name[];			/* File name, up to EXT2_NAME_LEN */
};

/*
 * The new version of the directory entry.  Since EXT2 structures are
 * stored in intel byte order, and the name_len field could never be
 * bigger than 255 chars, it's safe to reclaim the extra byte for the
 * file_type field.
 */
struct ext2_dir_entry_2 {
	unsigned int	inode;			/* Inode number */
	unsigned short	rec_len;		/* Directory entry length */
	unsigned char	name_len;		/* Name length */
	unsigned char	file_type;
	char	name[];			/* File name, up to EXT2_NAME_LEN */
};

/*
 * Ext2 directory file types.  Only the low 3 bits are used.  The
 * other bits are reserved for now.
 */
enum {
	EXT2_FT_UNKNOWN		= 0,
	EXT2_FT_REG_FILE	= 1,
	EXT2_FT_DIR		= 2,
	EXT2_FT_CHRDEV		= 3,
	EXT2_FT_BLKDEV		= 4,
	EXT2_FT_FIFO		= 5,
	EXT2_FT_SOCK		= 6,
	EXT2_FT_SYMLINK		= 7,
	EXT2_FT_MAX
};

#endif


