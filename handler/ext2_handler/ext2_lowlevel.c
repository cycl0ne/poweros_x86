/**
* File: /ext2_lowlevel．c
* User: cycl0ne
* Date: 2014-10-13
* Time: 03:22 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "ext2_handler.h"
#include "bcache_interface.h"

APTR ext2_GetBlock(pGD gd, UINT32 block_no)
{
	if (!block_no) return NULL;
	APTR buffer = NULL;
	if (gd->gd_Cache != NULL)
	{
		//KPrintF("Fetch CacheBlock\n");
		INT32 err = GetBlock(gd->gd_Cache, &buffer, block_no);
		if (err < 0) KPrintF("ext2: GetBlock Error\n");
		return buffer;
	}

	gd->gd_BufferBlock = block_no;

	UINT32 bsize = gd->block_size;

	gd->gd_request->io_Command	= CMD_READ;
	gd->gd_request->io_Length	= bsize >> 9;// Shift Operation for Speed! Devices in PowerOS are real Block Devices (512byte)
	gd->gd_request->io_Data		= gd->gd_Buffer;
	gd->gd_request->io_Offset	= block_no * (bsize >>9);
	DoIO(gd->gd_request);
	return gd->gd_Buffer;
}

INT32 ext2_ReleaseBlock(pGD gd, UINT32 block_no)
{
	if (gd->gd_Cache != NULL)
	{
		PutBlock(gd->gd_Cache, block_no);
		//KPrintF("Release CacheBlock\n");
		return 0;
	}
	if (gd->gd_BufferBlock != block_no) KPrintF("ERROR: Release wrong Block %d..",block_no);
	return 0;
}

UINT32 ext2_GetBlockNumber(pGD gd, ext2_inodetable_t *inode, UINT32 iblock) 
{
	UINT32 p = gd->pointers_per_block;

	/* We're going to do some crazy math in a bit... */
	unsigned int a, b, c, d, e, f, g;
	UINT32 *block = NULL;
	UINT32 blknum = 0;
//	KPrintF("Maximum: %d\n",EXT2_DIRECT_BLOCKS + p + p * p + p);
	
	if (iblock < EXT2_DIRECT_BLOCKS) 
	{
		return inode->block[iblock];
	} else if (iblock < EXT2_DIRECT_BLOCKS + p) 
	{
		block	= ext2_GetBlock(gd, inode->block[EXT2_DIRECT_BLOCKS]);
		blknum	= block[iblock - EXT2_DIRECT_BLOCKS];
		ext2_ReleaseBlock(gd, inode->block[EXT2_DIRECT_BLOCKS]);
		return blknum;	
	} else if (iblock < EXT2_DIRECT_BLOCKS + p + p * p) 
	{
		UINT32 tmp;
		a = iblock - EXT2_DIRECT_BLOCKS;
		b = a - p;
		c = b / p;
		d = b - c * p;

		block	= ext2_GetBlock(gd, inode->block[EXT2_DIRECT_BLOCKS + 1]);
		tmp		= block[c];
		ext2_ReleaseBlock(gd, inode->block[EXT2_DIRECT_BLOCKS + 1]);

		block	= ext2_GetBlock(gd, tmp);
		blknum	= block[d];
		ext2_ReleaseBlock(gd, tmp);
		return blknum;		
	} else if (iblock < EXT2_DIRECT_BLOCKS + p + p * p + p) 
	{
		UINT32 tmp, tmp2;
		a = iblock - EXT2_DIRECT_BLOCKS;
		b = a - p;
		c = b - p * p;
		d = c / (p * p);
		e = c - d * p * p;
		f = e / p;
		g = e - f * p;

		block	= ext2_GetBlock(gd, inode->block[EXT2_DIRECT_BLOCKS + 2]);
		tmp		= block[d];
		ext2_ReleaseBlock(gd, inode->block[EXT2_DIRECT_BLOCKS + 2]);

		block	= ext2_GetBlock(gd, tmp);
		tmp2	= block[f];
		ext2_ReleaseBlock(gd, tmp);

		block	= ext2_GetBlock(gd, tmp2);
		blknum	= block[g];
		ext2_ReleaseBlock(gd, tmp2);
		return blknum;
	}

	KPrintF("EXT2 driver tried to read to a block number that was too high (%d)", iblock);
	return 0;
}

UINT8 *ext2_GetBlockINode(pGD gd, ext2_inodetable_t *inode, UINT32 block_no)
{
	if (block_no >= inode->blocks) return NULL;
	
	UINT32 real_block = ext2_GetBlockNumber(gd, inode, block_no);
	if (real_block == 0) return NULL;
	gd->gd_GBIN_RB = real_block;
	//KPrintF("RealBlock: %d\n", real_block);
	UINT8 *adr = ext2_GetBlock(gd, real_block);	

	return adr;
}

UINT32 ext2_ReleaseBlockINode(pGD gd)
{
	UINT32 real_block = gd->gd_GBIN_RB;
	if (real_block != 0) ext2_ReleaseBlock(gd, real_block);
	return 0;
}

#if 0
UINT32 ext2_ReadBlockfromINode(pGD gd, ext2_inodetable_t *inode, UINT32 block_no, UINT8 *buffer)
{
	if (block_no >= inode->blocks)
	{
		MemSet(buf, 0x00, gd->block_size);
		KPrintF("Tried to read an invalid block. Asked for %d, but inode only has %d!", block, inode->blocks);
		return 0;		
	}
	UINT32 real_block = ext2_GetBlockNumber(gd, inode, block_no);
	APTR adr = ext2_GetBlock(gd, real_block);
	memcpy(buffer, adr, gd->block_size);
	ext2_ReleaseBlock(gd, real_block);
	return real_block;
}
#endif

ext2_inodetable_t * ext2_GetINode(pGD gd, UINT32 inode) 
{
	UINT32 group = inode / gd->inodes_per_group;
	if (group > BGDS) return NULL;

	UINT32 inode_table_block = BGD[group].inode_table;
	inode -= group * gd->inodes_per_group;	// adjust index within group
	UINT32 block_offset		= ((inode - 1) * SB->inode_size) / gd->block_size;
	UINT32 offset_in_block    = (inode - 1) - block_offset * (gd->block_size / SB->inode_size);

	UINT8 *buf = NULL;
	ext2_inodetable_t *inodet   = &gd->gd_INodeBuffer;

	buf = ext2_GetBlock(gd, inode_table_block + block_offset);	
	ext2_inodetable_t *inodes = (ext2_inodetable_t *)buf;
	memcpy(inodet, (UINT8 *)((UINT32)inodes + offset_in_block * SB->inode_size), SB->inode_size);
	ext2_ReleaseBlock(gd, inode_table_block + block_offset);
	return inodet;
}

UINT32 ext2_ReleaseINode(pGD gd, UINT32 inode)
{
	return 0;	
}

#if 0
#define BLOCKBIT(n)  (buffer[((n) >> 3)] & (1 << (((n) % 8))))
#define BLOCKBYTE(n) (buffer[((n) >> 3)])
#define SETBIT(n)    (1 << (((n) % 8)))
#define EXT2_BGD_BLOCK 2
	
INT32 ext2_AllocateInodeBlock(pGD gd, ext2_inodetable_t * inode, UINT32 inode_no, UINT32 block) 
{
	KPrintF("Allocating block #%d for inode #%d", block, inode_no);
	UINT32 	block_no     = 0;
	UINT32 	block_offset = 0;
	UINT32 	group        = 0;
	UINT8 	*buffer;
	
	for (UINT32 i = 0; i < gd->block_group_count; ++i) 
	{
		if (gd->block_groups[i].free_blocks_count > 0) 
		{
			buffer = GetBlock(gd, gd->block_groups[i].block_bitmap);
			while (BLOCKBIT(block_offset)) 
			{
				++block_offset;
			}
			block_no = block_offset + gd->superblock->blocks_per_group * i + 1;
			group = i;
			ReleaseBlock(gd, gd->block_groups[i].block_bitmap);
			break;
		}
	}

	if (!block_no) 
	{
		KPrintF("No available blocks, disk is out of space!");
		return -1;
	}

	UINT8 b = BLOCKBYTE(block_offset);
	b |= SETBIT(block_offset);
	BLOCKBYTE(block_offset) = b;
/*
 * Now write block to BlockGroupDesc.Bitmap
 * SetBlockNumber in inode (block, block_no)
 * gd->block_groups[group].free_blocks_count--;
 * write block EXT2_BGD_BLOCK Buffer gd->block_groups
 * inode->blocks++
 * writeInode(gd, inode, inode_no);
 * 
 */
	return 0;
}

#endif


