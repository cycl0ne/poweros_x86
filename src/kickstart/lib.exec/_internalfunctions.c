/*
**
** Exec Initialisation Code & INTERN Functions
** This Code runs now in an absolut perfect Stack
*/
#include "exec_funcs.h"
#include "sysbase.h"
#include "memory.h"
#include "arch_config.h"
#include "_debug.h"


// We need this in the initialisation phase to be called directly.
APTR lib_Allocate(SysBase *SysBase, pMemHeader mh, UINT32 nbytes);

SysBase *g_SysBase;
extern APTR FuncTab[];

static void INTERN_MakeFunctions(APTR target, APTR functionArray)
{
	INT32 n = 1;
	APTR vector;
	
	void **fp = (void **)functionArray;
	while(*fp != (void*)-1)
	{
		vector = (APTR)((UINT32)target-n*4); // EVIL on 64bit, this should be 8!
		*(UINT32 *)(((UINT32)vector)) = (UINT32) *fp;
		fp++;
		n++;
	}
}

static UINT32 INTERN_CountFunc(APTR functionArray)
{
	UINT32 n=0;
	void **fp=(void **)functionArray;

	/* -1 terminates the array */
	while(*fp!=(void *)-1)
	{
		fp++;
		n++;
	}
	return n*4; //Evil, on 64 bit this should be 8 ! :-P
}

pMemHeader CreateMemoryHead(UINT32 start_addr, UINT32 end_addr, UINT32 attr);

SysBase *INTERN_CreateSysBase(arch_config *config)
{
	struct SysBase *SysBase = NULL;
	pMemHeader memHead = NULL;	
	UINT32 negativeLibrarySize = INTERN_CountFunc(&FuncTab);
	
//	monitor_write("[PANIC] CreateMH.................");	
	memHead = CreateMemoryHead((UINT32)config->memory_base, (UINT32)config->memory_base+config->memory_size, config->memory_attribute);
//	monitor_write("ok\n");
	
	memHead->mh_Node.ln_Name= config->memory_name;
	memHead->mh_Node.ln_Pri	= config->memory_prio;
//	monitor_write("[PANIC] Allocate.................");	
//	monitor_write_hex((UINT32) memHead);
//	monitor_write("....");
	SysBase = lib_Allocate(NULL, memHead, negativeLibrarySize + sizeof(struct SysBase));
//	monitor_write_hex((UINT32) sizeof(struct SysBase));
//	monitor_write("....ok\n");

	if (SysBase == NULL)
	{
		monitor_write("[PANIC] No Memory for SysBase\n");
		for(;;);
	}
	
	SysBase = (struct SysBase *)((UINT32) SysBase + negativeLibrarySize);
//	monitor_write_hex((UINT32) SysBase);
//	monitor_write("....ok\n");
	INTERN_MakeFunctions(SysBase, &FuncTab);
	
	// FROM NOW ON YOU CAN USE JUMPTABLE SYSBASE BUT BE CAREFUL, NOT EVERYTHING INITIALISED!
   	NewListType(&SysBase->TaskReady	,NT_TASK);
   	NewListType(&SysBase->TaskWait	,NT_TASK);  	
   	NewListType(&SysBase->PortList	,NT_MSGPORT);
   	NewListType(&SysBase->MemList		,NT_MEMORY);
   	NewListType(&SysBase->DevList		,NT_DEVICE);
   	NewListType(&SysBase->LibList		,NT_LIBRARY);
   	NewListType(&SysBase->SemaphoreList,NT_SEMAPHORE);
   	NewListType(&SysBase->ResourceList	,NT_RESOURCE);
	NewListType(&SysBase->ResidentList	,NT_RESIDENT);

	// Create Exc/Trap/Irq Vectors in SysBase ALL NULL = Nothing is called
	for(int i = 0; i<64; i++) SysBase->ExcVector[i] = NULL;
	
	// Enqueue the Memory to our List
	Enqueue(&SysBase->MemList, &memHead->mh_Node);
   
   	SysBase->LibNode.lib_Node.ln_Pri  = -127;
   	SysBase->LibNode.lib_Node.ln_Type = NT_LIBRARY;
   	SysBase->LibNode.lib_Version      = 0;
   	SysBase->LibNode.lib_Revision     = 1;
   	SysBase->LibNode.lib_Node.ln_Name = "exec.library";
   	SysBase->LibNode.lib_IDString     = "\0$VER: exec.library 0.1 ("__DATE__")\r\n";
   	SysBase->LibNode.lib_PosSize      = sizeof(struct SysBase);
   	SysBase->LibNode.lib_NegSize      = negativeLibrarySize;
   	SysBase->LibNode.lib_OpenCnt      = 1;
	
	SysBase->TDNestCnt = 0;
	SysBase->IDNestCnt = -1;
	SysBase->thisTask = NULL;

	monitor_write(config->arch_name);
	monitor_write("______________________________________\n");

	//SysBase->CPU_Stack = AllocVec(4096, MEMF_FAST|MEMF_CLEAR); Not needed anymore, stack is now in SysBase structure

	// Init Exception Vectors Lists
	for (int i=0; i<16; i++) NewListType((struct List *)&SysBase->IntVectorList[i], NT_INTERRUPT);

	// Enqueue our exec.library to the library list
	Enqueue(&SysBase->LibList, &SysBase->LibNode.lib_Node);
	g_SysBase = SysBase;
	return SysBase;
}

