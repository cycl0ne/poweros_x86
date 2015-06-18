/**
 * @file bcache.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "bcache.h"
#include "devices.h"
#include "dos.h"
#include "dos_io.h"

#define SysBase BCacheBase->bc_SysBase
#define ERROR_NOTINCACHEBLOCK	900
#define ERROR_READ_BLOCK		901
#define ERROR_WRITE_BLOCK		902

struct bcache_block 
{
	MinNode		node;
	UINT64		blocknum;
	INT32		ref_count;
	BOOL		is_dirty;
	void*		ptr;
};

struct bcache_stats
{
	UINT32	hits;
	UINT32	depth;
	UINT32	misses;
	UINT32	reads;
	UINT32	writes;
};

struct bcache
{
	struct IOStdReq*	io;
	UINT32				block_size;
	INT32				count;
	struct bcache_stats	stats;
	MinList				free_list;
	MinList				lru_list;
	struct bcache_block	*blocks;
};

static inline void bc_SetErr(pBCacheBase BCacheBase, UINT32 error)
{
	pProcess proc = (pProcess)FindTask(NULL);
	if (proc->pr_Task.tcb_Node.ln_Type == NT_PROCESS)
	{
		 proc->pr_Result2 = error;
	}
}

#define SetErr(a) bc_SetErr(BCacheBase, a)

static INT32 ReadBlocks(pBCacheBase BCacheBase, struct bcache *cache, UINT32 blockStart, UINT32 blockLen)
{
	if (!cache) return -1;
	if (!cache->io) return -1;
	
	uint8_t tmpBuf = AllocVec(256*1024*1024, MEMF_FAST);
	if (!tmpBuf) return -1;
	
	uint32_t realBlockStart = blockStart * (cache->block_size >>9);
	uint32_t realLen		= blockLen * (cache->block_size >>9);
	
	KPrintF("ReadBlocks: Wanted to read from: %d Len: %d\n", blockStart, blockLen);
	
	cache->io->io_Command	= CMD_READ;
	cache->io->io_Length	= realLen;
	cache->io->io_Data		= tmpBuf;
	cache->io->io_Offset	= realBlockStart;
	DoIO(cache->io);

	return cache->io->io_Actual; // Check for error?
}

static INT32 ReadBlock(pBCacheBase BCacheBase, struct bcache *cache, UINT8* buffer, UINT32 block)
{
	if (!cache) return -1;
	if (!cache->io) return -1;

	UINT32 bsize = cache->block_size;

//	KPrintF("Wanted to read: %d at block: %d at offset: %d\n", cache->block_size, )
	cache->io->io_Command	= CMD_READ;
	cache->io->io_Length	= bsize>>9;// Shift Operation for Speed! Devices in PowerOS are real Block Devices (512byte)
	cache->io->io_Data		= buffer;
	cache->io->io_Offset	= block * (bsize >>9);
	DoIO(cache->io);
	return cache->io->io_Actual; // Check for error?
}

static INT32 WriteBlock(pBCacheBase BCacheBase, struct bcache *cache, UINT8* buffer, UINT32 block)
{
	if (!cache) return -1;
	if (!cache->io) return -1;
	if (!block) return -1;
	
	UINT32 bsize = cache->block_size;

	cache->io->io_Command	= CMD_WRITE;
	cache->io->io_Length	= bsize>>9;// Shift Operation for Speed! Devices in PowerOS are real Block Devices (512byte)
	cache->io->io_Data		= buffer;
	cache->io->io_Offset	= block * (bsize >>9);
	DoIO(cache->io);
	return cache->io->io_Actual; // Check for error?
}

bcache_t bc_CreateCache(pBCacheBase BCacheBase, struct IOStdReq* io, INT32 block_size, INT32 block_count)
{
	struct bcache	*cache;
	cache = AllocVec(sizeof(struct bcache), MEMF_FAST|MEMF_CLEAR);
	if (!cache) KPrintF("Couldnt allocate memory for Cache\n");
	
	cache->io			= io;
	cache->block_size	= block_size;
	cache->count		= block_count;
	//MemSet(&cache->stats, 0, sizeof(cache->stats)); // Not needed, allocvec cleared memory for us
	NewList((pList)&cache->free_list);
	NewList((pList)&cache->lru_list);
	
	cache->blocks = AllocVec(sizeof(struct bcache_block) * block_count, MEMF_FAST|MEMF_CLEAR);
	if (!cache->blocks) KPrintF("Couldnt allocate memory for Cache\n");
	INT32 i;
	
	uint8_t *tempbuf = AllocVec(block_size*block_count, MEMF_FAST|MEMF_CLEAR);
//	KPrintF("tempbuf= %d\n", tempbuf);
	if (!tempbuf) KPrintF("Couldnt allocate Buffer\n");
	
	for (i=0; i < block_count; i++)
	{
		cache->blocks[i].ref_count	= 0;
		cache->blocks[i].is_dirty	= FALSE;
		cache->blocks[i].ptr		= tempbuf + (i*block_size);
//		KPrintF("Block %i: Adr: %d\n", i, cache->blocks[i].ptr);
//		cache->blocks[i].ptr		= AllocVec(block_size, MEMF_FAST|MEMF_CLEAR);
//		if (!cache->blocks[i].ptr) KPrintF("Couldnt allocate memory for Cache\n");
		AddHead((pList)&cache->free_list, (pNode)&cache->blocks[i].node);
	}

	KPrintF("bcache.library: Created Cache with: %d Blocksize and %d Blockcount\n", block_size, block_count);
	return (bcache_t)cache;
}

static int bc_FlushBlock(pBCacheBase BCacheBase, struct bcache *cache, struct bcache_block *block)
{
	INT32	rc = 0;
	
	rc = WriteBlock(BCacheBase, cache, block->ptr, block->blocknum);
	SetErr(ERROR_WRITE_BLOCK);
	if (rc < 0) return rc;
	
	block->is_dirty = FALSE;
	cache->stats.writes++;
	return 0;
}

void bc_DestroyCache(pBCacheBase BCacheBase, bcache_t _cache)
{
	struct bcache *cache = _cache;
	INT32	i;
	
	for (i=0; i < cache->count; i++)
	{
		if (!(cache->blocks[i].ref_count == 0)) KPrintF("warning: ref count not 0");
		if (cache->blocks[i].is_dirty) KPrintF("warning: freeing dirty block %d\n", cache->blocks[i].blocknum);
		FreeVec(cache->blocks[i].ptr);
	}
	FreeVec(cache);
}

static struct bcache_block *bc_FindBlock(pBCacheBase BCacheBase, struct bcache *cache, UINT64 blocknum)
{
	UINT32 depth = 0;
	struct bcache_block *block;
	
	block = NULL;
	ForeachNode(&cache->lru_list, block)
	{
		depth++;
		
		if (block->blocknum == blocknum)
		{
			Remove((pNode)&block->node);
			AddTail((pList)&cache->lru_list, (pNode)&block->node);
			cache->stats.hits++;
			cache->stats.depth += depth;
			return block;
		}
	}
	
	cache->stats.misses++;
	return NULL;
}

static struct bcache_block *bc_AllocBlock(pBCacheBase BCacheBase, struct bcache *cache)
{
	INT32 err;
	struct bcache_block *block;
	
	block = (struct bcache_block*)RemHead((pList)&cache->free_list);
	
	if (block) 
	{
		block->ref_count = 0;
		AddTail((pList)&cache->lru_list, (pNode)&block->node);
		return block;
	}
	
	ForeachNode(&cache->lru_list, block)
	{
		if (block->ref_count == 0)
		{
			if (block->is_dirty)
			{
				err = bc_FlushBlock(BCacheBase, cache, block);
				if (err) return NULL;
			}
			
			Remove((pNode)&block->node);
			AddTail((pList)&cache->lru_list, (pNode)&block->node);
			return block;
		}
	}
	return NULL;
}

static struct bcache_block *bc_FindOrFillBlock(pBCacheBase BCacheBase, struct bcache *cache, UINT64 blocknum)
{
	INT32 err;
	struct bcache_block *block = bc_FindBlock(BCacheBase,cache, blocknum);
	
	if (block == NULL)
	{
		block = bc_AllocBlock(BCacheBase, cache);
		block->blocknum = blocknum;
		err = ReadBlock(BCacheBase, cache, block->ptr, blocknum);
		if (err < 0) 
		{
			AddTail((pList)&cache->free_list, (pNode)&block->node);
			SetErr(ERROR_READ_BLOCK);
			return NULL;
		}
		cache->stats.reads++;
	}
	return block;
} 

int bc_ReadBlock(pBCacheBase BCacheBase, bcache_t _cache, void *buf, UINT64 blocknum)
{
	struct bcache *cache = _cache;
	struct bcache_block	*block = bc_FindOrFillBlock(BCacheBase,cache, blocknum);

	if (block == NULL) return -1;
	CopyMem(block->ptr, buf, cache->block_size);
	return 0;
}

int bc_GetBlock(pBCacheBase BCacheBase, bcache_t _cache, void **ptr, UINT64 blocknum)
{
	struct bcache *cache = _cache;
	struct bcache_block *block = bc_FindOrFillBlock(BCacheBase,cache, blocknum);
	
	if (block == NULL) return -1;

	// add ref_count to keep it from being freed
	block->ref_count++;
	*ptr = block->ptr;
	return 0;
}

int bc_PutBlock(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum)
{
	struct bcache *cache = _cache;
	struct bcache_block *block = bc_FindBlock(BCacheBase, cache, blocknum);
	
	block->ref_count--;
	return 0;
}

int bc_MarkBlockDirty(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum)
{
	struct bcache *cache = _cache;
	struct bcache_block *block;
	
	block = bc_FindBlock(BCacheBase, cache, blocknum);
	if (!block) 
	{
		SetErr(ERROR_NOTINCACHEBLOCK);
		return -1;
	}
	
	block->is_dirty = TRUE;
	return 0;
}

int bc_ZeroBlock(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum)
{
	struct bcache *cache = _cache;
	struct bcache_block *block = bc_FindBlock(BCacheBase, cache, blocknum);
	
	if (!block)
	{
		block = bc_AllocBlock(BCacheBase, cache);
		if (!block) return -1;
		
		block->blocknum = blocknum;
	}
	MemSet(block->ptr, 0, cache->block_size);
	block->is_dirty = TRUE;
	return 0;
}

int bc_FlushCache(pBCacheBase BCacheBase, bcache_t _cache)
{
	INT32	err;
	struct bcache *cache = _cache;
	struct bcache_block *block;

	ForeachNode(&cache->lru_list, block)
	{
		if (block->is_dirty)
		{
			err = bc_FlushBlock(BCacheBase,cache, block);
			if (err) return err;
		}
	}
	return 0;
}

void bc_DumpCache(pBCacheBase BCacheBase, bcache_t _cache)
{
	UINT32 finds;
	struct bcache *cache = _cache;
	
	finds = cache->stats.hits + cache->stats.misses;

	KPrintF("%s: hits=%u(%u%%) depth=%u misses=%u(%u%%) reads=%u writes=%u\n",
	   "bcache.library",
	   cache->stats.hits,
	   finds ? (cache->stats.hits * 100) / finds : 0,
	   cache->stats.hits ? cache->stats.depth / cache->stats.hits : 0,
	   cache->stats.misses,
	   finds ? (cache->stats.misses * 100) / finds : 0,
	   cache->stats.reads,
	   cache->stats.writes);
}