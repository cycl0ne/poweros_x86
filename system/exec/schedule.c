/**
 * @file schedule.c
 * Reschedule processor to highest priority task.
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

void	arch_Before_Task_Runs(SysBase *SysBase, Task *Task);
extern SysBase *g_SysBase;

static void _Dispatch(void)
{
	SysBase *SysBase = g_SysBase;
	pTask old = FindTask(NULL);
//	KPrintF("DISP-");
	if (old) {
		if (old->tcb_State == TASK_FREE)
		{
			FreeVec((APTR)old->tcb_SPLower);
			FreeVec(old);
		}
	}
	// Force context switch to the next process
	pTask newTask = (pTask)RemHead(&SysBase->TaskReady);
	newTask->tcb_State = TASK_RUN;
	newTask->tcb_CPUUsage++;
	SysBase->thisTask = newTask;
//	KPrintF("Call Launch.\n");
	if (newTask->tcb_Launch) newTask->tcb_Launch(SysBase, newTask);
//	KPrintF("End Launch, now Context Restore.\n");
	// Arm needs this for the SupervisorStack
	arch_Before_Task_Runs(SysBase, newTask);
	context_restore(&newTask->tcb_SavedContext);
}

/**
 * @ingroup Tasks
 *
 * Reschedule processor to the highest prio task, this routine can be called from ANYWHERE
 * Exception: Not from non reentrant IRQs! (this is why we have the SYSFLG_NOSCHEDULE)
 * 
 * @param SysBase
 *		pointer to the SysBase
*/
void lib_Reschedule(SysBase *SysBase)
{
	IRQMask im = Disable();
	if (SysBase->SysFlags & SYSFLG_NOSCHEDULE) 
	{
		// We missed a Reschedule call, put it on delay next time we can.
		SysBase->SysFlags |= SYSFLG_RESCHEDULE;
		Restore(im);
		return;
	}

	if (SysBase->nDefer > 0)
	{
		SysBase->attemptDefer = TRUE;
		Restore(im);
		return;
	}

	pTask oldTask = FindTask(NULL);
	if (oldTask != NULL)
	{
		oldTask->tcb_IntMask = im;
		
		if (oldTask->tcb_State == TASK_RUN)
		{
			// If we are the highest prio running Task, we keep running
			pTask head = (pTask)GetHead(&SysBase->TaskReady);
			if (oldTask->tcb_Node.ln_Pri > head->tcb_Node.ln_Pri)
			{
				//KPrintF("Has higher Prio (%d, %d)\n",oldTask->tcb_Node.ln_Pri,head->tcb_Node.ln_Pri);
				Restore(oldTask->tcb_IntMask);
				return;
			}
		
			// We have to reschedule us.
			if (!context_save(&oldTask->tcb_SavedContext)) 
			{
				// here we get if we get the Cpu back again.
				//KPrintF("we are back!\n");
				Restore(oldTask->tcb_IntMask);
				return;
			}
			// Call the Switch() routine
	//		KPrintF("Call Switch.\n");
			if (oldTask->tcb_Switch) oldTask->tcb_Switch(SysBase, oldTask);		
			oldTask->tcb_State = TASK_READY;
			Enqueue(&SysBase->TaskReady, &oldTask->tcb_Node);
	//		KPrintF("Enqueue.\n");
		} else
		{
			if (!context_save(&oldTask->tcb_SavedContext)) 
			{
				// here we get if we get the Cpu back again.
				//KPrintF("we are back!\n");
				Restore(oldTask->tcb_IntMask);
				return;
			}		
			if (oldTask->tcb_Switch) oldTask->tcb_Switch(SysBase, oldTask);		
		}
	}
	context_save(&SysBase->CPU_Context);
	context_set(&SysBase->CPU_Context, FADDR(_Dispatch), &SysBase->CPU_Stack, 2048);
	context_restore(&SysBase->CPU_Context);
}

/**
 * @ingroup Tasks
 *
 * Yield processor.
 * @return OK when the thread is context switched back
 */
SysCall lib_YieldCPU(SysBase *SysBase)
{
	IRQMask	im = Disable();
	Reschedule();
	Restore(im);
	return OK;
}
