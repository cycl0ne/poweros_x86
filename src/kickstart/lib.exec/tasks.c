#include "types.h"
#include "tasks.h"
#include "sysbase.h"
#include "context.h"
#include "exec_funcs.h"

//void lib_Print_uart0(const char *s);
extern SysBase *g_SysBase;

__attribute__((no_instrument_function)) UINT32 arch_Int_Enable(void);
__attribute__((no_instrument_function)) UINT32 arch_Int_Read(void);

#undef DEBUGTASKS

//void lib_hexstrings(UINT32);

UINT32 interrupts_disable(void);
UINT32 interrupts_enable(void);
void interrupts_restore(UINT32 ipl);
UINT32 interrupts_read(void);
BOOL interrupts_disabled(void);

Task *lib_FindTask(SysBase *SysBase, STRPTR name)
{
	UINT32 ipl;
	if (name == NULL) return SysBase->thisTask;
	Task *task=NULL;

	ipl = Disable();
	task = (Task *)FindName((List*)&SysBase->TaskReady, name);
	if (task == NULL) task = (Task *)FindName((List*)&SysBase->TaskWait, name);
	Enable(ipl);
	return task;
}

static void TaskRun(void)
{
	SysBase *SysBase = g_SysBase;
	Task *itsme = SysBase->thisTask;
	APTR arg = itsme->TaskArg;
	void (*f)(void *, void *) = itsme->TaskFunc;
	arch_Int_Enable();
	f(arg, SysBase);
	if (itsme->Flags & TF_CREATETASKALLOC)
	{
		// Task was created with CreateTask so Dealloc Memory
		FreeVec(itsme->Stack);
		FreeVec(itsme);
	}
	//DPrintF("Left Main\n");
	itsme->State = REMOVED;
	Schedule();
	for(;;); //Not reached
}

Task *lib_TaskCreate(SysBase *SysBase, char *name, APTR codeStart, APTR data, UINT32 stackSize, INT8 pri)
{
	Task *newTask = AllocVec(sizeof(Task), MEMF_FAST|MEMF_CLEAR);
	if (newTask==NULL) return NULL;

	newTask->Stack = AllocVec(stackSize, MEMF_FAST|MEMF_CLEAR);
	if (newTask->Stack == NULL) 
	{
		FreeVec(newTask);
		return NULL;
	}
	newTask->StackSize = stackSize;
	newTask->tc_SPLower = (UINT32) newTask->Stack;
	newTask->tc_SPUpper = (UINT32) newTask->Stack + stackSize;
	if (name == NULL) 	newTask->Node.ln_Name = "UnknownTask";
	else newTask->Node.ln_Name = name;
	newTask->Node.ln_Pri = pri;
	return AddTask(newTask, codeStart, NULL, data);
}

Task *lib_AddTask(SysBase *SysBase, Task *newTask, APTR codeStart, APTR finalPC, APTR data) 
{
	if (newTask == NULL) return NULL;
	#ifdef DEBUGTASKS
	DPrintF("[TaskCreate] Name: %s\n", name);
	DPrintF("[TaskCreate] stackSize: %x\n", stackSize);	
	DPrintF("[TaskCreate] Stack: %x\n", newTask->Stack);
	#endif
	
	//YEAH WANTS IT !!! -> newTask->Node.ln_Type = NT_TASK; //Perhaps PowerDOS wants to add this?!
	if (newTask->Node.ln_Name == NULL) 	newTask->Node.ln_Name = "UnknownTask";
	newTask->State = READY;
	newTask->CPU_Usage = 0;
	newTask->TDNestCnt = -1; // TaskSched allowed
	newTask->IDNestCnt = -1; // Ints allowed
	newTask->TaskArg = data;
	newTask->TaskFunc = codeStart;
	
	context_save(&newTask->SavedContext);
	context_set(&newTask->SavedContext, FADDR(TaskRun), (UINTPTR) newTask->Stack, newTask->StackSize);
	UINT32 ipl = Disable();
	newTask->SavedContext.ipl = arch_Int_Read();
	Enqueue(&SysBase->TaskReady,&newTask->Node);
	Enable(ipl);

	return newTask;
}

INT8 lib_AllocSignal(SysBase *SysBase, INT32 signalNum)
{
	struct Task *thisTask;
	UINT32 mask;
	UINT32 mask1 = 1,mask2 = 1;

	thisTask = (struct Task *)FindTask(NULL);

	mask = (UINT32)thisTask->SigAlloc;

	/* Setze Startwert */
	if((signalNum<0) || (signalNum>31)) signalNum = 31;
	mask1=mask2<<signalNum;			/* 0b000...001 << 31 = 0b100...000 */

	// Here we Check if this Signal is free (0b100..00 & 0b000..01)
	while (signalNum > -1)
	{
		if (!(mask1 & mask)) break;
		signalNum--;
		mask1 = mask2<<signalNum;
	}

	if (signalNum<0) return (INT8)-1;

	// delete recieved signals.....
	thisTask->SigAlloc  |= mask1;

	mask2 = ~mask1;
	thisTask->SigRecvd  &= mask2;
	thisTask->SigExcept &= mask2;
	thisTask->SigWait   &= mask2;

	return (INT8)signalNum;
}

void lib_FreeSignal(SysBase *SysBase, INT32 signalNum)
{
  if (signalNum != -1)
  {
    struct Task *task;
    task = (struct Task *)FindTask(NULL);
    task->SigAlloc &= ~(1<<signalNum);
  }
}

INT8 lib_SetTaskPri(SysBase *SysBase, struct Task *task, INT16 pri)
{
//	lib_Print_uart0("Change Pri\n");

	INT8 old;
//	DPrintF("[Change Pri] FindTask");
	if (task == NULL) task = FindTask(NULL);
//	DPrintF("[Change Pri] Forbid()");
	Forbid();
	old = task->Node.ln_Pri;
	task->Node.ln_Pri = (INT8)pri;

//	DPrintF("[Change Pri] Task-State: %d", task->State);
	if (task->State == READY)
	{
    	Remove(&task->Node);
    	Enqueue(&SysBase->TaskReady, &task->Node);
	}
	Permit();
//	DPrintF("[Change Pri] Schedule\n");
	if ((SysBase->TDNestCnt < 0) && (!(IsListEmpty(&SysBase->TaskReady)))) Schedule();
	return old;
}


