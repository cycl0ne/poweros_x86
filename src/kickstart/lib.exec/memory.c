#include "types.h"
#include "sysbase.h"
#include "memory.h"
#include "arch_config.h"
#include "exec_funcs.h"

//64 Bit align for the ARM
#define ALIGN(x)        (((x)+8-1)&-8)

static inline void removechunk(pMemCHead node)
{
   node->mch_Node.mln_Succ->mln_Pred = node->mch_Node.mln_Pred;
   node->mch_Node.mln_Pred->mln_Succ = node->mch_Node.mln_Succ;
}

static inline void addchunktail(List *list, Node *node)
{
   node->ln_Succ = (Node *)&list->lh_Tail;
   node->ln_Pred = list->lh_TailPred;
   list->lh_TailPred->ln_Succ = node;
   list->lh_TailPred          = node;
}

static inline void EnqueueMemChunk(struct List *list, pMemCHead node)
{
	pMemCHead next;
	ForeachNode(list, next)
	{
		if (node->mch_Size < next->mch_Size) break;
	}
	node->mch_Node.mln_Pred				= next->mch_Node.mln_Pred;
	node->mch_Node.mln_Succ				= (struct MinNode *)next;
	next->mch_Node.mln_Pred->mln_Succ 	= (struct MinNode *)node;
	next->mch_Node.mln_Pred 			= (struct MinNode *)node;	
}

static inline void NewListT(List *list, UINT8 type)
{
    list->lh_Tail = NULL;
    list->lh_Head = (Node*)&list->lh_Tail;
    list->lh_TailPred = (Node*)&list->lh_Head;
    list->lh_Type = type;
}

static inline pMemCHead find_smallest_hole(UINT32 size, pMemHeader mh)
{
	pMemCHead node;
	ForeachNode(&mh->mh_List, node)
	{
		if (node->mch_Size >= size) return node;
	}
	return NULL;
}

APTR lib_Allocate(SysBase *SysBase, pMemHeader mh, UINT32 size)
{
	size = ALIGN(size);
	UINT32		new_size = ALIGN(size + sizeof(MemCHead) + sizeof(MemCFoot));
//	if (SysBase != NULL) DPrintF("newSize: %x %b\n", new_size, new_size);
	pMemCHead	found = find_smallest_hole(new_size, mh);
	
	if (found != NULL) 
	{
		UINT32 orig_hole_pos = (UINT32)found;
		UINT32 orig_hole_size= found->mch_Size;
		
		if (orig_hole_size - new_size < sizeof(MemCHead)+sizeof(MemCFoot))
		{
			size += orig_hole_size-new_size;
			new_size = orig_hole_size;
		}
		removechunk(found);
		pMemCHead block_header	= (pMemCHead) orig_hole_pos;
		block_header->mch_Magic	= MCHC_MAGIC;
		block_header->mch_Flags	= MCHF_BLOCK;
		block_header->mch_Size	= new_size;
		pMemCFoot block_footer	= (pMemCFoot)  (orig_hole_pos + sizeof(MemCHead) + size);
		block_footer->mcf_Magic	= MCHC_MAGIC;
		block_footer->mcf_Head	= block_header;
		addchunktail(&mh->mh_ListUsed, (struct Node *)block_header);
		mh->mh_Free			-= new_size;
		
		if (orig_hole_size - new_size > 0)
		{
			pMemCHead hole_header	= (pMemCHead) (orig_hole_pos + sizeof(MemCHead) + size + sizeof(MemCFoot));
			hole_header->mch_Magic	= MCHC_MAGIC;
			hole_header->mch_Flags	= MCHF_HOLE;
			hole_header->mch_Size	= orig_hole_size - new_size;
			pMemCFoot hole_footer	= (pMemCFoot) ( (UINT32)hole_header + orig_hole_size - new_size - sizeof(MemCFoot) );
			if ((UINT32) hole_footer < mh->mh_Upper)
			{
				hole_footer->mcf_Magic	= MCHC_MAGIC;
				hole_footer->mcf_Head	= hole_header;
			}
			EnqueueMemChunk(&mh->mh_List, hole_header);
		}
		return (void *) ((UINT32)block_header+sizeof(MemCHead));
	}
	return NULL;
}

void lib_Deallocate(SysBase *SysBase, pMemHeader mh, void *p)
{
	if (p == NULL) return;
	pMemCHead header = (pMemCHead) ((UINT32)p - sizeof(MemCHead));
	pMemCFoot footer = (pMemCFoot) ((UINT32)header + header->mch_Size - sizeof(MemCFoot));

	if (header->mch_Magic != MCHC_MAGIC) {DPrintF("No Magic in Header\n");return;}
	if (footer->mcf_Magic != MCHC_MAGIC) {DPrintF("No Magic in Footer\n");return;}

	mh->mh_Free			+= header->mch_Size;
	header->mch_Flags	= MCHF_HOLE;
	header->mch_Task	= NULL;
	BOOL do_add 		= TRUE;
	removechunk(header); // Remove from Used List
	
	pMemCFoot test_footer = (pMemCFoot) ( (UINT32)header - sizeof(MemCFoot) );
	if  (test_footer->mcf_Magic == MCHC_MAGIC && 
		(test_footer->mcf_Head->mch_Flags & MCHF_HOLE))
	{
		UINT32 cache_size 	= header->mch_Size;
		header->mch_Magic	= 0;
		header				= test_footer->mcf_Head;
		footer->mcf_Head	= header;
		header->mch_Size	+= cache_size;
		test_footer->mcf_Magic = 0;
		do_add				= FALSE;
	}

	pMemCHead test_header = (pMemCHead) ( (UINT32)footer + sizeof(MemCFoot) );
	if  (test_header->mch_Magic == MCHC_MAGIC && 
		(test_header->mch_Flags & MCHF_HOLE))
	{
		header->mch_Size 	+= test_header->mch_Size;
		test_footer			= (pMemCFoot) ( (UINT32)test_header + test_header->mch_Size - sizeof(MemCFoot) );
		if (test_footer->mcf_Magic != MCHC_MAGIC) DPrintF("No Magic found.\n");
		test_footer->mcf_Head = header;
		removechunk(test_header);
		test_header->mch_Magic = 0;
	}
	if (do_add) EnqueueMemChunk(&mh->mh_List, header);
}

pMemHeader CreateMemoryHead(UINT32 start_addr, UINT32 end_addr, UINT32 attr)
{
	pMemHeader mh = (pMemHeader)start_addr;
	NewListT(&mh->mh_List, NT_MEMORY);
	NewListT(&mh->mh_ListUsed, NT_MEMORY);
	start_addr += sizeof(MemHeader);
	// Align the Start
	start_addr =ALIGN(start_addr);

	mh->mh_Free = end_addr - start_addr;
	mh->mh_Lower= start_addr;
	mh->mh_Upper= end_addr;
	mh->mh_Attr	= attr;
	pMemCHead hole = (pMemCHead) start_addr;
	hole->mch_Size	= end_addr - start_addr;
	hole->mch_Magic	= MCHC_MAGIC;
	hole->mch_Flags = MCHF_HOLE;
	EnqueueMemChunk(&mh->mh_List,hole);
	return mh;
}

void lib_AddMemList(SysBase *SysBase, UINT32 size, UINT32 attribute, INT32 pri, APTR base, STRPTR name)
{
	pMemHeader mem = CreateMemoryHead((UINT32)base, (UINT32)base + size, attribute);

	mem->mh_Node.ln_Pri  = pri;
	mem->mh_Node.ln_Name = name;
	mem->mh_Node.ln_Type = NT_MEMORY;
	mem->mh_Attr = attribute;  
	UINT32 ipl = Disable();
	Enqueue(&SysBase->MemList, &mem->mh_Node);
	Enable(ipl);
}

APTR lib_AllocVec(SysBase *SysBase, UINT32 byteSize, UINT32 requirements)
{
    UINT8 *ret = NULL;
    struct MemHeader *mh=(struct MemHeader *)SysBase->MemList.lh_Head;
	Forbid();
	while(mh->mh_Node.ln_Succ != NULL)
	{
		if (mh->mh_Free >= byteSize)
		{
			ret = Allocate(mh, byteSize);
			pMemCHead node = (pMemCHead)((UINT32)ret-sizeof(MemCHead));
			if (node->mch_Magic != MCHC_MAGIC) DPrintF("ERROR: AllocVec->NoMagic\n");
			node->mch_Task	= FindTask(NULL);
//			AddTail(&mh->mh_ListUsed, (struct Node *)node);
			if(requirements & MEMF_CLEAR) memset(ret, '\0', byteSize);
			Permit();
			return ret;
		}
		mh=(struct MemHeader *)mh->mh_Node.ln_Succ;
	}	
	Permit();
	return ret;
}

void lib_FreeVec(SysBase *SysBase, APTR memoryBlock)
{
    if(!memoryBlock) return;
    struct MemHeader *mh=(struct MemHeader *)SysBase->MemList.lh_Head;
    while(mh->mh_Node.ln_Succ)
    {
    	/* Test if the memory belongs to this MemHeader. */
	    if(mh->mh_Lower <= (UINT32)memoryBlock && mh->mh_Upper > (UINT32)memoryBlock)
	    {
			Forbid();
			Deallocate(mh, memoryBlock);
			Permit();
			return;
		}
		mh=(struct MemHeader *)mh->mh_Node.ln_Succ;
	}
}

#define MEMF_PUBLIC		(1L<<0)
#define MEMF_CHIP		(1L<<1)
#define MEMF_FAST		(1L<<2)

#define MEMF_FREE		(1L<<29)
#define MEMF_LARGEST	(1L<<30)
#define MEMF_TOTAL		(1L<<31)

UINT32 lib_AvailMem(SysBase *SysBase, UINT32 attributes)
{
	UINT32	ret = 0;
	UINT32	physflags = (MEMF_PUBLIC|MEMF_CHIP|MEMF_FAST) & attributes;

	pMemHeader MHNode;
	Forbid();
	ForeachNode(&SysBase->MemList, MHNode)
	{
		if (MHNode->mh_Attr & physflags)
		{
			if (attributes & MEMF_FREE)	ret	+= MHNode->mh_Free;
			if (attributes & MEMF_TOTAL)ret	+= (MHNode->mh_Upper - MHNode->mh_Lower);
			if (attributes & MEMF_LARGEST)
			{
				pMemCHead chunk = GetTail(&MHNode->mh_List);
				if (chunk->mch_Size > ret) ret = chunk->mch_Size;
			}
		}
	}
	Permit();
	return ret;
}

void *lib_CopyMemQuick(SysBase *SysBase, const APTR src, APTR dest, int n) 
{
//	const UINT32 *f = src;
//	UINT32 *t = dest;
//	while (n-- >0) *t++ = *f++;
	memcpy32(dest, src, n);
	return dest;
}

void *lib_CopyMem(SysBase *SysBase,const APTR src,  APTR dest, int n) 
{
//    const char *f = src;
//    char *t = dest;
//    while (n-- > 0) *t++ = *f++;
	memcpy(dest, src, n);
	return dest;
}

extern void *asm_memset(void* m, int c, UINT32 n);

void *lib_MemSet(SysBase *SysBase, void* m, int c, UINT32 len) 
{
	//char *bb;
	//for (bb = (char *)m; len--; ) *bb++ = c;
	memset(m, c, len);
	return m;
}

