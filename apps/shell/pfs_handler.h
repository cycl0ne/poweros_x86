/**
 * @file pfs__handler.h
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#define MAX_FILENAME_LENGTH		256
#define MAX_VOLNAME_LENGTH		32

#define BLOCKNUM	UINT64

#define MYFS_CLEAN   0x434c454e        /* 'CLEN', for flags field */ 
#define MYFS_DIRTY   0x44495254        /* 'DIRT', for flags field */ 

#define SUPER_BLOCK_MAGIC1	0x5355504552424C4B	//SUPERBLK
#define SUPER_BLOCK_MAGIC2	0x5355504552424C4B	//
#define SUPER_BLOCK_MAGIC3	0x5355504552424C4B	//
#define INODE_BLOCK_MAGIC1	0x494E4F4445424C4B	//INODEBLK

#define MAX_DIRECT_BLOCKS	32
#define MAX_INDIRECT_BLOCKS	32
#define MAX_INDIRECT2_BLOCKS	32
#define MAX_INDIRECT3_BLOCKS	16

typedef struct SuperBlock
{
	char		volname[MAX_VOLNAME_LENGTH]; // 8x UINT64
	INT64		magic1;
	INT64		block_size;
	INT64		block_shift;
	
	UINT64		num_blocks;
	UINT64		used_blocks;

	UINT64		num_inodes;
	BLOCKNUM	bitmap_start;
	UINT64		bitmap_len;
	INT64		magic2;
	INT64		flags;
	
	BLOCKNUM	root_inode;
	BLOCKNUM	journal_start;
	UINT64		journal_len;
	
	INT64		magic3;
} SuperBlock, *pSuperBlock;

typedef struct INode
{
	INT64		magic1; 		//8
	INT32		uid;			//4
	INT32		gid;			//4
	INT32		mode;			//4
	INT32		flags;			//4
	BLOCKNUM	this;			//8
	BLOCKNUM	parent;			//8
	DateStamp	create_time; 	//12
	DateStamp	last_modified_time; //12
	BLOCKNUM	direct[MAX_DIRECT_BLOCKS]; //8 *NUM (256) == MAX_DIRECT_BLOCKS*1024 Filedirect (32kb)
	BLOCKNUM	indirect[MAX_INDIRECT_BLOCKS];		// (256) == MAX_INDIRECT_BLOCKS*128*1024 Filedirect (4mb)
	BLOCKNUM	indirect2[MAX_INDIRECT2_BLOCKS];		//(256) == MAX_INDIRECT2_BLOCKS*128* (MAX_INDIRECT_BLOCKS*128*1024) (16Gb)
	BLOCKNUM	indirect3[MAX_INDIRECT3_BLOCKS];		//128 == MAX_INDIRECT3_BLOCKS*128* (MAX_INDIRECT2_BLOCKS*128* (MAX_INDIRECT_BLOCKS*128*1024)) (32TB)
	UINT64		size;			//8
} INode, *pINode;

typedef struct IndirectBlock
{
	BLOCKNUM	block[128];
}IBlock, * pIBlock;

typedef struct DirEntryBlock
{
	BLOCKNUM	inum;		//8
	UINT16		name_len;	//2
	char		name[1];	//1
} DEB, *pDEB;

typedef struct PFSData
{
	APTR		pd_sysBase;
	APTR		pd_dosBase;
	APTR		pd_utilBase;
	
	INT32		pd_FileErr;
	SuperBlock	pd_SuperBlock;

} PFSData, *pPFSData;

