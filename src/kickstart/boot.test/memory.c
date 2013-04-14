#include "types.h"
#include "sysbase.h"
#include "list.h"
#include "memory.h"
#include "exec_funcs.h"

extern SysBase *g_SysBase;
#define SysBase g_SysBase

#define ALIGN(x)        (((x)+8-1)&-8)

void test_new_memory()
{
	pMemCHead node;	
    struct MemHeader *mh=(struct MemHeader *)SysBase->MemList.lh_Head;
    
	ForeachNode(&mh->mh_ListUsed, node)
	{
		Task *task = node->mch_Task;
		DPrintF("Used Memory at %x, size %x, task [%s]\n", node, node->mch_Size, task->Node.ln_Name);		
	}

#if 0
	APTR memstart = AllocVec(0x50000, MEMF_PUBLIC);
	if (memstart == NULL) 
	{
		DPrintF("no memory");
		while(1);
	}
	
	pMemHeader mem = CreateMemoryHead((UINT32)memstart, (UINT32)memstart+0x50000, MEMF_FAST);
	if (mem == NULL)
	{
		DPrintF("Couldnt allocate Memoryheader\n");
		while(1);
	}
	pMemCHead node;	
	APTR testalloc[5];
	testalloc[0] = Alloc(0x500, 0, mem);
	testalloc[1] = Alloc(0x50, 0, mem);
	testalloc[2] = Alloc(0x100, 0, mem);
	testalloc[3] = Alloc(0x300, 0, mem);
	testalloc[4] = Alloc(0x200, 0, mem);
	DPrintF("FreeMem: %x\n", mem->mh_Free);
	for (int i=0; i<5; i++)
	{
		DPrintF("Alloc Mem %d = %x\n", i, testalloc[i]);
	}
	ForeachNode(&mem->mh_ListUsed, node)
	{
		DPrintF("Used Memory at %x, size %x, task %x\n", node, node->mch_Size, node->mch_Task);
	}
	Dealloc(testalloc[0], mem);
	Dealloc(testalloc[1], mem);
	Dealloc(testalloc[3], mem);
	DPrintF("FreeMem: %x\n", mem->mh_Free);
	ForeachNode(&mem->mh_List, node)
	{
		//if (node->mch_Size >= size) return node;
		DPrintF("Found Hole at: %x size: %x [%x]\n", node, node->mch_Size, node->mch_Task);
	}

	Dealloc(testalloc[2], mem);
	DPrintF("FreeMem: %x\n", mem->mh_Free);
	ForeachNode(&mem->mh_List, node)
	{
		//if (node->mch_Size >= size) return node;
		DPrintF("Found Hole at: %x size: %x [%x]\n", node, node->mch_Size, node->mch_Task);
	}
DPrintF("----------------------------------\n");
	Dealloc(testalloc[4], mem);
	DPrintF("FreeMem: %x\n", mem->mh_Free);
	ForeachNode(&mem->mh_List, node)
	{
		//if (node->mch_Size >= size) return node;
		DPrintF("Found Hole at: %x size: %x [%x]\n", node, node->mch_Size, node->mch_Task);
		pMemCFoot foot = (pMemCFoot) ((UINT32)node+node->mch_Size-sizeof(MemCHead)-sizeof(MemCFoot));
		DPrintF("Footer of Hole at: %x, Header in Footer %x\n",foot, foot->mcf_Head);
	}
	while(1);
#endif
}

