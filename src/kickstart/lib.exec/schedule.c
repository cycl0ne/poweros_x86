#include "types.h"
#include "tasks.h"
#include "sysbase.h"
#include "context.h"
#include "exec_funcs.h"

extern SysBase *g_SysBase;

//UINT32 interrupts_disable(void);
//UINT32 interrupts_enable(void);
//void interrupts_restore(UINT32 ipl);
//UINT32 interrupts_read(void);
//BOOL interrupts_disabled(void);
//void lib_Print_uart0(const char *s);
void	arch_Before_Task_Runs(SysBase *SysBase, Task *Task);

static void MakeTaskReady(SysBase *SysBase, Task *Task)
{
	if (Task) {
		Task->State = READY;
		Task->CPU_Usage++;
		Task->TDNestCnt = SysBase->TDNestCnt;
		Task->IDNestCnt = SysBase->IDNestCnt;
		if (Task->Switch) Task->Switch(SysBase);
		Enqueue(&SysBase->TaskReady, &Task->Node);
	}
}

#if 0
Not used anymore!
static void MakeTaskWait(SysBase *SysBase, Task *Task)
{
	if (Task) {
		Task->CPU_Usage++;
		Task->TDNestCnt = SysBase->TDNestCnt;
		Task->IDNestCnt = SysBase->IDNestCnt;
		if (Task->Switch) Task->Switch(SysBase);
		Enqueue(&SysBase->TaskWait, &Task->Node);
	}
}
#endif

static void MakeTaskRun(SysBase *SysBase, Task *Task)
{
	if (Task) {
		Task->State = RUN;
		SysBase->TDNestCnt = Task->TDNestCnt;
		SysBase->IDNestCnt = Task->IDNestCnt;
		if (Task->Launch) Task->Launch(SysBase);
		UINT32 stack = (UINT32)Task->Stack;
		if (stack < Task->tc_SPLower) { DPrintF("Stack Overrun!\n"); }
	} else
	{
		DPrintF("PANIC: No Task Ready, where is IDLE?");
		for(;;);
	}
}

static void MakeTaskDestroy(SysBase *SysBase, Task *Task)
{
	// Here do some Garbage Collection
	// Task schould be in no list, so just delete all infos of him
	FreeVec(Task->Stack); // was allocated through us
	FreeVec(Task); // this too
	// Finshed
}

// RASPI extern APTR supervisor_sp;
UINT32 debug_schedule = 0;


static void BeforeTaskRuns(SysBase *SysBase, Task *Task)
{
	arch_Before_Task_Runs(SysBase, Task);
/*
	//-RASPi Code
	
	UINT8 *stck;
	
	stck = (UINT8 *)&Task->Stack[Task->StackSize - SP_DELTA];
	supervisor_sp = (APTR) stck;
*/
}

#define debug_schedule 0

void lib_Dispatch(void)
{
	SysBase *SysBase = g_SysBase;
	Task *Task = SysBase->thisTask;
#if debug_schedule
	if (debug_schedule) 
	{
		DPrintF("[DISPATCH] %s (%x)\n", Task->Node.ln_Name, Task->State);
		DPrintF("Task: %x\n", Task);
	}
#endif	
	if (Task) {
		switch(Task->State) {
		case RUN:
			MakeTaskReady(SysBase, Task);
			break;
		case REMOVED:
			MakeTaskDestroy(SysBase, Task);
			break;
		case WAIT:
			// Do nothing
			//MakeTaskWait(SysBase, Task);
			DPrintF("[Wait] here!\n");
			break;
		case DEAD:
		default:
			{
			DPrintF("[DEAD/Unknown] (%s)(%x)\n", Task->Node.ln_Name, Task->State);
			for(;;);
			break;
			}
		}
		Task = NULL;
	}
	Task = (struct Task *)RemHead(&SysBase->TaskReady);
	//DPrintF("[DISPATCH] %s (%x)\n", Task->Node.ln_Name, Task->State);
	MakeTaskRun(SysBase, Task);
	SysBase->thisTask = Task;
	BeforeTaskRuns(SysBase, Task);
	context_restore(&Task->SavedContext);
}

void lib_Schedule(SysBase *SysBase)
{
	VUINT32 ipl = Disable();
#if debug_schedule
	if (debug_schedule) 
	{
		Task *tmp_Task = SysBase->thisTask;
		DPrintF("[SCHEDULE] SysBase %x\n", SysBase);
		DPrintF("TDNestCnt [%d] IDNestCnt [%d]\n", SysBase->TDNestCnt, SysBase->IDNestCnt);
		DPrintF("List Ready: %x\n", (IsListEmpty(&SysBase->TaskReady)));
		DPrintF("Stack Check: %x, Size: %x (%x), Stack Actual: %x", tmp_Task->Stack, tmp_Task->StackSize,
				tmp_Task->Stack+tmp_Task->StackSize, tmp_Task->SavedContext.sp); 
	}
#endif	
	if ((SysBase->TDNestCnt >= 0) || (IsListEmpty(&SysBase->TaskReady))) {
#if debug_schedule
		if (debug_schedule) 
		{
			DPrintF("ListEmpty or TDNestCnt\n");
		}
#endif
		Enable(ipl);
		return;
	}

	Task *this = SysBase->thisTask;
#if debug_schedule
	if (debug_schedule) DPrintF("Getting Task %s\n", SysBase->thisTask->Node.ln_Name);
#endif
	if (SysBase->thisTask) 
	{
		if (!context_save(&this->SavedContext)) 
		{
			//lib_Print_uart0("Coming Home\n");
			Enable(this->SavedContext.ipl);
			//lib_Print_uart0("Leaving Sched\n");
			return;
		}
		this->SavedContext.ipl = ipl;
	}
	
#if debug_schedule
	if (debug_schedule) 
	{
		DPrintF("Going to Dispatch %s\n", SysBase->thisTask->Node.ln_Name);
		Task *tmp_Task = SysBase->thisTask;
		DPrintF("Stack Check: %x, Size: %x (%x), Stack Actual: %x", tmp_Task->Stack, tmp_Task->StackSize,
				tmp_Task->Stack+tmp_Task->StackSize, tmp_Task->SavedContext.sp); 
	}
#endif

	context_save(&SysBase->CPU_Context);
	context_set(&SysBase->CPU_Context, FADDR(lib_Dispatch), &SysBase->CPU_Stack, _CPUSTACK_);
	context_restore(&SysBase->CPU_Context);
}

