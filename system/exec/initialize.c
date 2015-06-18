/**
 * @file initialize.c
 *
 * Initialize the System
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "arch_config.h"
#include "exec_interface.h"
#include "_debug.h"

// We need this in the initialisation phase to be called directly.
APTR lib_Allocate(SysBase *SysBase, pMemHeader mh, UINT32 nbytes);
SysCall lib_MakeFunctions(SysBase *SysBase, APTR target, APTR functionArray);
pMemHeader _CreateMemoryHead(UINT32 start_addr, UINT32 end_addr, UINT32 attr);
void arch_clk_init(SysBase *SysBase);
void arch_irq_init(void);
void arch_irq_create(SysBase *SysBase);

extern APTR FuncTab[];
extern SysBase *g_SysBase; // this is our savepoint for SysBase

arch_config config;
Context ctx;

#if 0

static inline UINT32 _CountFunc(APTR functionArray)
{
	UINT32 n=0;
	void **fp=(void **)functionArray;
	while(*fp++!=(void *)-1) n++;
	return n;
}

#endif

static inline UINT32 _CountFunc(APTR functionArray)
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

void _ExecTask(void *execBase, APTR arg)
{
	struct SysBase *SysBase = (struct SysBase *)execBase;
	//KPrintF("\n_ExecTask_Start %x\n", SysBase->nDefer);
	InitResidentCode(RTF_COLDSTART|RTF_AUTOINIT);
//	KPrintF("Finding DOS--------------------\n");
//	KPrintF("--------------RTF_DOS--------------------\n");
	pResidentNode rn =FindResident("dos.library");

	//KPrintF("(%x)", rn);
	//KPrintF("(%x)", rn->rn_Resident);
//	KPrintF("Starting DOS--------------------\n");
	InitResident(rn->rn_Resident, NULL);
//	KPrintF("RTF_TESTCASE--------------------\n");
	InitResidentCode(RTF_TESTCASE);
	KPrintF("[EXEC] Done. Leaving startup.\n");
	//KPrintF("\n_ExecTask_End %x\n", SysBase->nDefer);
}

void _Idle(void *execBase, APTR arg)
{
	struct SysBase *SysBase = (struct SysBase *)execBase;
//	KPrintF("\n_IdleTask_Start %x\n", SysBase->nDefer);
	SetTaskPri(NULL, -125);
	KPrintF("Idle HLT!\n");
	while(1) asm volatile("hlt");
	KPrintF("[IDLE] PONR\n");
}

BOOL _RomTagScanner(SysBase *SysBase, UINT32 *start, UINT32 *end)
{
    struct List         *mods= &SysBase->ResidentList;
    struct Resident     *res = NULL;
    struct ResidentNode *node = NULL;
    UINT8 *ptr = (UINT8 *)start;

    //KPrintF("RomTagScanner - Start: %x End: %x\n", start, end);

    while( ptr <= (UINT8 *)end)
    {
        res = (struct Resident *)ptr;

        // Check for a Resident Structure
        if (res->rt_MatchWord == RTC_MATCHWORD && res->rt_MatchTag == res)
        {
			KPrintF("RomTagScanner - Found Resident: %s\n",res->rt_Name);
			node = AllocVec(sizeof(struct ResidentNode), MEMF_FAST); //MEMF_CLEAR|MEMF_PUBLIC);
			if (!node) return FALSE;
			node->rn_Resident    = res;
			node->rn_Node.ln_Pri = res->rt_Pri;
			node->rn_Node.ln_Name= res->rt_Name;
			// Enqueue found Resident
			Enqueue(mods, &node->rn_Node);
        }
        ptr++;
    }
    return TRUE;
}

SysBase *_CreateSysBase(arch_config *config)
{
	struct SysBase *SysBase = NULL;
	pMemHeader memHead = NULL;

	UINT32 negativeLibrarySize = _CountFunc(&FuncTab);
	memHead = _CreateMemoryHead((UINT32)config->memory_base, (UINT32)config->memory_base+config->memory_size, config->memory_attribute);

	memHead->mh_Node.ln_Name= config->memory_name;
	memHead->mh_Node.ln_Pri	= config->memory_prio;
	SysBase = lib_Allocate(NULL, memHead, negativeLibrarySize + sizeof(struct SysBase));

	if (SysBase == NULL)
	{
		//monitor_write("[PANIC] No Memory for SysBase\n");
		for(;;);
	}

	SysBase = (struct SysBase *)((UINT32) SysBase + negativeLibrarySize);

	lib_MakeFunctions(NULL, SysBase, &FuncTab);
	SysBase->DBG_Cnt = 0;
	
	// FROM NOW ON YOU CAN USE JUMPTABLE SYSBASE BUT BE CAREFUL, NOT EVERYTHING INITIALISED!
   	NewListType(&SysBase->TaskReady		,NT_TASK);
   	NewListType(&SysBase->TaskWait		,NT_TASK);
   	NewListType(&SysBase->PortList		,NT_MSGPORT);
   	NewListType(&SysBase->MemList		,NT_MEMORY);
   	NewListType(&SysBase->DevList		,NT_DEVICE);
   	NewListType(&SysBase->LibList		,NT_LIBRARY);
   	NewListType(&SysBase->SemaphoreList	,NT_SEMAPHORE);
   	NewListType(&SysBase->ResourceList	,NT_RESOURCE);
	NewListType(&SysBase->ResidentList	,NT_RESIDENT);

	// Create Exc/Trap/Irq Vectors in SysBase ALL NULL = Nothing is called
	for(int i = 0; i<64; i++) SysBase->ExcVector[i] = NULL;

	// Enqueue the Memory to our List
	Enqueue(&SysBase->MemList, &memHead->mh_Node);

   	SysBase->LibNode.lib_Node.ln_Pri  = -127;
   	SysBase->LibNode.lib_Node.ln_Type = NT_LIBRARY;
   	SysBase->LibNode.lib_Version      = 0;
   	SysBase->LibNode.lib_Revision     = 3;
   	SysBase->LibNode.lib_Node.ln_Name = "exec.library";
   	SysBase->LibNode.lib_IDString     = "\0$VER: exec.library 0.3	("__DATE__")\r\n";
   	SysBase->LibNode.lib_PosSize      = sizeof(struct SysBase);
   	SysBase->LibNode.lib_NegSize      = negativeLibrarySize;
   	SysBase->LibNode.lib_OpenCnt      = 1;

	SysBase->TDNestCnt = 0;
	SysBase->IDNestCnt = -1;
	SysBase->thisTask = NULL;

	// Init Exception Vectors Lists
	for (int i=0; i<16; i++) NewListType((struct List *)&SysBase->IntVectorList[i], NT_INTERRUPT);

	//monitor_write(config->arch_name);
	//monitor_write("______________________________________\n");
	//SysBase->CPU_Stack = AllocVec(4096, MEMF_FAST|MEMF_CLEAR); Not needed anymore, stack is now in SysBase structure
	// Enqueue our exec.library to the library list
	Enqueue(&SysBase->LibList, &SysBase->LibNode.lib_Node);
	g_SysBase = SysBase;
	return SysBase;
}

#define FADDR(fptr)  ((APTR) (fptr))
#define _IDLE_TASK_STACK_ 4096
#define _EXEC_TASK_STACK_ 4096

extern UINT32 *__code;
extern UINT32 *__end;
extern UINT32 *__resident_tags_start;
extern UINT32 *__resident_tags_end;

pTask lib_CreateTask(SysBase *SysBase, STRPTR name, Task_Function codeStart, APTR data, UINT32 stackSize, INT8 pri);
//SysCall lib_ReadyTask(SysBase *SysBase, pTask task, BOOL resch)

void acpi_init(void);
void smp_init(void);

void ExecInit(void)
{
	SysBase *SysBase = _CreateSysBase(&config);
	if (SysBase == NULL)
	{
		//monitor_write("[PANIC] No SysBase created\n");
		for(;;);
	}
	KPrintF("\n%s ______________________________________\n", config.arch_name);

	// Disable Taskscheduling & set some vars.
	SysBase->nDefer			= 1;
	SysBase->attemptDefer	= FALSE;
	SysBase->SysFlags		= 0;

	arch_irq_init(); 			// Remap IRQ
	arch_irq_create(SysBase); 	// Create IRQ/Exc Handlers/Servers
	arch_clk_init(SysBase);		// create Clock

	acpi_init();
	smp_init();

	Enable();

	SysBase->ExecTask->tcb_Node.ln_Name = "InitTask";
	SysBase->thisTask = &SysBase->InitTask;			// Fake a Task for AllocVec

	// Create two clean Task, one IDLE Task and one Worker Task with Prio 100
	SysBase->ExecTask = CreateTask("ExecTask"	, _ExecTask	, SysBase, _EXEC_TASK_STACK_, 100);
	SysBase->IdleTask = CreateTask("idle"		, _Idle		, SysBase, _IDLE_TASK_STACK_, -124); //-124);
	ReadyTask(SysBase->ExecTask, FALSE);
	ReadyTask(SysBase->IdleTask, TRUE);
	
	// Old code
	//CreateInitTask(SysBase, &SysBase->ExecTask, "ExecTask"	, lib_ExecTask	, SysBase, _EXEC_TASK_STACK_, 100);
	//CreateInitTask(SysBase, &SysBase->IdleTask, "idle"		, lib_Idle		, SysBase, _IDLE_TASK_STACK_, -124);


	if (_RomTagScanner(SysBase, (UINT32 *)&__resident_tags_start, (UINT32 *)&__resident_tags_end) == FALSE)
	{
		KPrintF("[PANIC] RomTagScanner FAILED!\n");
		for(;;);
	}
	InitResidentCode(RTF_SINGLETASK|RTF_AUTOINIT);
	//KPrintF("[INIT] Activating Multitasking -> Leaving SingleTask\n");
	SysBase->thisTask		= NULL; // kill our small temp task.
	SysBase->nDefer			= 0;

	//KPrintF("[INIT] Schedule -> leaving Kernel Init\n");
	Reschedule();
	KPrintF("[INIT] PONR (point of no return)\n");
	asm volatile("int $0x1");
	for(;;);
}

// Our Jump Code to the real Code
__attribute__((no_instrument_function)) void main_bsp(void)
{
	config.cpu_count = 1;
	config.cpu_active = 1;
	config.base = (UINT32 *)&__code;
	config.kernel_size = (UINT32)&__end - (UINT32)&__code;//hardcoded_rom_size;
	config.stack_size = 4096;
	config.stack_base = (UINT32*) (&__end + config.stack_size);
	config.arch_name = "PowerOS x86";

	config.memory_base = (UINT32 *)0x200000;//config.stack_base;
	config.memory_size = (UINT32  )0x5000000;
	config.memory_prio = 0;
	config.memory_attribute = MEMF_FAST;
	config.memory_name = "Fast Memory\n";

#if 0
	monitor_write("[main_bsp] ExecInit\n");
	monitor_write_hex((UINT32)config.base);
	monitor_put('\n');
	monitor_write_hex((UINT32)config.kernel_size);
	monitor_put('\n');
	monitor_write_hex((UINT32)config.stack_base);
	monitor_put('\n');
#endif

	context_save(&ctx);
	context_set(&ctx, FADDR(ExecInit), config.stack_base, 4096);
	context_restore(&ctx);
}




