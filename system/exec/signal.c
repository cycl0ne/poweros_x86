/**
 * @file signal.c
 * In this file all signal handling is done.
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

Signal lib_WaitSignal(SysBase *SysBase, Signal signalSet)
{
	pTask	this	= FindTask(NULL);
	Signal	signal	= 0;
	IRQMask	im = Disable();
//	KPrintF("Wait: (%s) this->tcb_SigRecvd %x\n", this->tcb_Node.ln_Name, this->tcb_SigRecvd);
	if(! (this->tcb_SigRecvd & signalSet) )
	{
		this->tcb_SigWait = signalSet;
		this->tcb_State = TASK_WAITSIGNAL;
		AddHead(&SysBase->TaskWait, &this->tcb_Node);
		Reschedule();
	}
	signal = this->tcb_SigRecvd & signalSet;
	this->tcb_SigRecvd &= ~(signalSet);
	Restore(im);
	return signal;
}

Signal lib_SetSignal(SysBase *SysBase, Signal newSignals, Signal signalSet)
{
	Signal	old;
	pTask	task = FindTask(NULL);

	IRQMask ipl = Disable();
	old = task->tcb_SigRecvd;
	task->tcb_SigRecvd = (old&~signalSet)|(signalSet&newSignals);
	Restore(ipl);

	return old;
}

SysCall lib_SignalTask(SysBase *SysBase, pTask task, Signal signalSet)
{
	if (task == NULL) //return SYSERR;
	{
		KPrintF("SignalTask: SysErr\n");
		return SYSERR;
	}
	IRQMask ipl = Disable();

	task->tcb_SigRecvd |= signalSet;
	if (task->tcb_State == TASK_WAITSIGNAL)
	{
		//KPrintF("SignalTask: ReadyTask (%s) (%x)\n", task->tcb_Node.ln_Name, task);
		ReadyTask(task, TRUE);
	}
	Restore(ipl);
	return OK;
}

INT8 lib_AllocSignal(SysBase *SysBase, INT32 signalNum)
{
	struct Task *thisTask;
	UINT32 mask;
	UINT32 mask1 = 1,mask2 = 1;

	thisTask = (struct Task *)FindTask(NULL);

	mask = (UINT32)thisTask->tcb_SigAlloc;

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
	thisTask->tcb_SigAlloc  |= mask1;

	mask2 = ~mask1;
	thisTask->tcb_SigRecvd  &= mask2;
//	thisTask->tcb_SigExcept &= mask2;
	thisTask->tcb_SigWait   &= mask2;

	return (INT8)signalNum;
}

SysCall lib_FreeSignal(SysBase *SysBase, INT32 signalNum)
{
  if (signalNum != -1)
  {
    struct Task *task;
    task = (struct Task *)FindTask(NULL);
    task->tcb_SigAlloc &= ~(1<<signalNum);
	return OK;
  }
  return SYSERR;
}

