/**
 * @file tasks.c
 * All Tasks related functions
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

#define TF_CREATETASKALLOC (1 << 7)
extern void * g_SysBase;

pTask lib_FindTask(SysBase *SysBase, STRPTR name)
{
	IRQMask ipl;
	pTask task=NULL;

	if (name == NULL) return SysBase->thisTask;
	ipl = Disable();
	task = (pTask)FindName(&SysBase->TaskReady, name);
	if (task == NULL) task = (pTask)FindName(&SysBase->TaskWait, name);
	Restore(ipl);
	return task;
}

static void _TaskRun(SysBase *SysBase)
{
//	SysBase *SysBase = g_SysBase;
//	KPrintF("Received: %x\n", dysBase);
//	for(;;);
	pTask	this = SysBase->thisTask;
	Enable();
	//KPrintF("Starting Task\n");
	if (this->tcb_TaskFunc != NULL) this->tcb_TaskFunc(SysBase, this->tcb_TaskArg);
	//KPrintF("Ending Task\n");
	this->tcb_State = TASK_FREE;
	Reschedule();
	for(;;); // Never reached
}

SysCall lib_AddTask(SysBase *SysBase, pTask task, Task_Function codeStart, APTR data)
{
	if (task != NULL)
	{
		task->tcb_State			= TASK_SUSPENDED;
		task->tcb_CPUUsage		= 0;
		task->tcb_TDNestCnt		= -1;
		task->tcb_IDNestCnt		= -1;
		task->tcb_TaskArg		= data;
		task->tcb_TaskFunc		= codeStart;

		UINT32 *stack = (UINT32*) (task->tcb_Stack + task->tcb_StackSize);
		stack[-1] = (UINT32)SysBase;

		context_save(&task->tcb_SavedContext);
		context_set(&task->tcb_SavedContext, FADDR(_TaskRun), (UINTPTR) task->tcb_Stack, task->tcb_StackSize);
		IRQMask ipl = Disable();
		task->tcb_SavedContext.ipl = ipl; //arch_Int_Read();
		Enqueue(&SysBase->TaskWait,&task->tcb_Node);
		Restore(ipl);
		return OK;
	}
	return SYSERR;
}

SysCall lib_StackSwapRun(SysBase *SysBase, UINT32 stackSize, APTR entry)
{
	SysCall	ret = SYSERR;
	pTask	this = FindTask(NULL);
	
	UINT8 *newStackMem = AllocVec(stackSize, MEMF_FAST|MEMF_CLEAR);
	
	if (newStackMem)
	{
		UINT32 oldSize = this->tcb_StackSize;
		UINT32 oldUpper = this->tcb_SPUpper;
		UINT32 oldLower = this->tcb_SPLower;
		UINT8* oldStack = this->tcb_Stack;

		UINT32 *newStack = (UINT32*) (newStackMem + stackSize - 4);
		newStack[0] = (UINT32) SysBase;
		
		this->tcb_StackSize = stackSize;
		this->tcb_Stack		= (UINT8*) newStackMem;
		this->tcb_SPLower	= (UINT32) newStackMem;
		this->tcb_SPUpper	= (UINT32) (newStackMem + stackSize);

		asm volatile
		(
			/* Save original ESP by setting up a new stack frame */
			"	push	%%ebp\n"
			"	movl	%%esp, %%ebp\n"
			/* Actually change the stack */
			"	movl	%2, %%esp\n\t"

			/* Call our function */
			"	call	*%1\n"

			/* Restore original ESP. Function's return value is in EAX. */
			"	movl	%%ebp, %%esp\n"
			"	pop	%%ebp\n"
			: "=a"(ret)
			: "r"(entry), "r"(newStack), "a"(SysBase)
			: "ecx", "edx", "cc"
		);
//KPrintF("mem: %x",(newStackMem + stackSize));
		this->tcb_StackSize = oldSize;
		this->tcb_Stack		= oldStack;
		this->tcb_SPLower	= oldLower;
		this->tcb_SPUpper	= oldUpper;
		FreeVec(newStackMem);
	}
	return ret;
}

pTask lib_CreateTask(SysBase *SysBase, STRPTR name, Task_Function codeStart, APTR data, UINT32 stackSize, INT8 pri)
{
	pTask	newTask = AllocVec(sizeof(Task), MEMF_FAST|MEMF_CLEAR);

	//DPrintF("newTask [%s] %x\n", name, newTask);
	if (newTask != NULL) 
	{
		newTask->tcb_Flags = TF_CREATETASKALLOC;

		newTask->tcb_Stack = AllocVec(stackSize, MEMF_FAST|MEMF_CLEAR);
		if (newTask->tcb_Stack != NULL) 
		{
			newTask->tcb_Node.ln_Pri= pri;
			newTask->tcb_Node.ln_Type= NT_TASK;
			
			newTask->tcb_Switch		= NULL;
			newTask->tcb_Launch		= NULL;
			newTask->tcb_StackSize 	= stackSize;
			newTask->tcb_SPLower	= (UINT32) newTask->tcb_Stack;
			newTask->tcb_SPUpper	= (UINT32) newTask->tcb_Stack + stackSize;

			if (name == NULL) 	newTask->tcb_Node.ln_Name = "UnknownTask";
			else newTask->tcb_Node.ln_Name = name;

			if (AddTask(newTask, codeStart, data) != SYSERR)
			{
				#ifdef DEBUGTASKS
				DPrintF("[TaskCreate] Name: %s\n", name);
				DPrintF("[TaskCreate] stackSize: %x\n", stackSize);	
				DPrintF("[TaskCreate] Stack: %x\n", newTask->Stack);
				#endif
				return newTask;
			}
			FreeVec(newTask->tcb_Stack);
		}
		FreeVec(newTask);
	}
	return NULL;
}

INT8 lib_SetTaskPri(SysBase *SysBase, struct Task *task, INT8 pri)
{
	INT8 old;
	//KPrintF("[Change Pri] FindTask");
	if (task == NULL) task = FindTask(NULL);

	//KPrintF("[Change Pri] Forbid()");
	Forbid();
	old = task->tcb_Node.ln_Pri;
	task->tcb_Node.ln_Pri = (INT8)pri;

//	DPrintF("[Change Pri] Task-State: %d", task->State);
	if (task->tcb_State == TASK_READY)
	{
    	Remove(&task->tcb_Node);
    	Enqueue(&SysBase->TaskReady, &task->tcb_Node);
	}
	//KPrintF("[Change Pri] Permit()");
	Permit();
//	DPrintF("[Change Pri] Schedule\n");
	Reschedule();
	return old;
}

SysCall lib_ReadyTask(SysBase *SysBase, pTask task, BOOL resch)
{
	if (task == NULL) //return SYSERR;
	{
		KPrintF("ReadyTask: Syserr\n");
		return SYSERR;
	}
	IRQMask	im = Disable();
   	Remove(&task->tcb_Node);
	task->tcb_State = TASK_READY;
   	Enqueue(&SysBase->TaskReady, &task->tcb_Node);
	Restore(im);

    if (resch == TRUE) 
    {
		//KPrintF("ReadyTask-Reschedule\n");
		Reschedule();
    }
    return OK;	
}

SysCall lib_Forbid(SysBase *SysBase)
{
	IRQMask im = Disable();
	if (SysBase->nDefer++ == 0) SysBase->attemptDefer = FALSE;
//	KPrintF("---------Forbid-------------");
	Restore(im);
	return OK;
}

SysCall lib_Permit(SysBase *SysBase)
{
	IRQMask im = Disable();
	if (SysBase->nDefer <= 0) 
	{
		Restore(im);
		return SYSERR;
	}

	if (--SysBase->nDefer == 0)
	{
		if (SysBase->attemptDefer) 
		{
			Restore(im);
//			Reschedule();
			return OK;
		}
	}
//	KPrintF("------------Permit--------");
	Restore(im);
	return OK;	
}
