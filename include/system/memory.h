#ifndef memory_h
#define memory_h

#include "types.h"
#include "lists.h"
#include "tasks.h"

#define MEMF_ANY		(0L)	/* Any type of memory will do */
#define MEMF_PUBLIC		(1L<<0)
#define MEMF_CHIP		(1L<<1)
#define MEMF_FAST		(1L<<2)

#define MEMF_CLEAR		(1L<<16)

#define MEMF_FREE		(1L<<29)
#define MEMF_LARGEST	(1L<<30)
#define MEMF_TOTAL		(1L<<31)

typedef struct MemHeader
{
	Node	mh_Node;
	UINT32	mh_Attr;
	UINT32	mh_Lower;
	UINT32	mh_Upper;
	UINT32	mh_Free;
	UINT32	mh_MaxMemory;
	List	mh_List;
	List	mh_ListUsed;
}MemHeader, *pMemHeader;

#define MCHC_MAGIC 0x0b57ac1e5 //Obstacles are in our way :)
#define MCHF_BLOCK (1<<0)
#define MCHF_HOLE  (1<<1)

typedef struct MemChunkHead
{
	MinNode	mch_Node;	// 12 Bytes (3x4)
	UINT32	mch_Magic;	// 4 Bytes// MagicNumber to find MemHead in Memory
	pTask	mch_Task;	// 4 Bytes Task that allocated me
	UINT32	mch_Flags;	// 4 Bytes All kind of Flags, see MCHF_xxx
	UINT32	mch_Size;	// 4 Bytes	// Size of Memory incl. MemFoot
} MemCHead, *pMemCHead;

typedef struct MemChunkFoot
{
	UINT32		mcf_Magic;	// Magic number, same as in MemHead.
	pMemCHead	mcf_Head; 	// Pointer to the block header.
} MemCFoot, *pMemCFoot;

#define ALIGN_DOWN(s, a)  ((s) & ~((a) - 1))
#define ALIGN_UP(s, a)  (((s) + ((a) - 1)) & ~((a) - 1))

typedef struct Pool {
	MinList PuddleList;
	UINT32 MemoryFlags;
	UINT32 PuddleSize;
	UINT32 ThreshSize;
} Pool, *pPool;

#endif