/**
 * @file semaphore.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

SysCall lib_InitSemaphore(struct SysBase *SysBase, pSignalSemaphore signalSemaphore)
{
	NewListType((struct List *)&signalSemaphore->ss_WaitQueue, NT_SEMAPHORE);
	signalSemaphore->ss_Owner=0;
	signalSemaphore->ss_NestCount=0;
	signalSemaphore->ss_Link.ln_Type=NT_SIGNALSEM;
	signalSemaphore->ss_QueueCount=-1;
	return OK;
}

SysCall lib_AddSemaphore(struct SysBase *SysBase, const char *semName, struct SignalSemaphore *sigSem)
{
	InitSemaphore(sigSem);
	sigSem->ss_Link.ln_Name = (char *)semName;
	Forbid();
	Enqueue(&SysBase->SemaphoreList,&sigSem->ss_Link);
	Permit();
	return OK;
}

SysCall lib_RemSemaphore(struct SysBase *SysBase, struct SignalSemaphore *sigSem)
{
	Forbid();
	Remove(&sigSem->ss_Link);
	Permit();
	return OK;
}

struct SignalSemaphore *lib_FindSemaphore(struct SysBase *SysBase, const char *name)
{
	return (struct SignalSemaphore *)FindName(&SysBase->SemaphoreList, (STRPTR) name);
}

SysCall lib_ObtainSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem)
{
	pTask this = FindTask(NULL);

	Forbid();
	//QueueCount >0 = locked

	sigSem->ss_QueueCount++;
	if (sigSem->ss_QueueCount == 0)
	{
		sigSem->ss_Owner = this;
		sigSem->ss_NestCount++;
		sigSem->ss_Type = SS_TYPE_SIMPLE;
	} else if ( sigSem->ss_Owner == this ) 
	{
		sigSem->ss_NestCount++;
	} else
	{
		SemRequest sr;
		IRQMask ipl;
		sr.sr_Waiter = this;
		ipl = Disable();
		this->tcb_SigRecvd &= ~SIGF_SEMAPHORE;
		AddTail((struct List *)&sigSem->ss_WaitQueue, (struct Node *)&sr.sr_Link);
		Restore(ipl);
		WaitSignal(SIGF_SEMAPHORE);
		//sigSem->ss_Owner = this;
		//sigSem->ss_NestCount++;
		sigSem->ss_Type = SS_TYPE_SIMPLE;
    }
	/* All Done! */
	Permit();
	return OK;
}

SysCall lib_AttemptSemaphore(struct SysBase *SysBase, struct SignalSemaphore *signalSemaphore)
{
	pTask me = FindTask(NULL);

	Forbid();
	signalSemaphore->ss_QueueCount++;
	if (signalSemaphore->ss_QueueCount == 0)
	{
		signalSemaphore->ss_Owner = me;
		signalSemaphore->ss_NestCount++;
		signalSemaphore->ss_Type = SS_TYPE_SIMPLE;
		Permit();
		return OK;
	}

	if (signalSemaphore->ss_Owner == me)
	{
		signalSemaphore->ss_NestCount++;
		Permit();
		return OK;
	}
	signalSemaphore->ss_QueueCount--;
	Permit();
	return SYSERR;
}

SysCall lib_ReleaseSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem)
{
    Forbid();
    /* Release one on the nest count */
    sigSem->ss_NestCount--;
    sigSem->ss_QueueCount--;

    if(sigSem->ss_NestCount == 0)
    {
    	if((sigSem->ss_QueueCount >= 0) && !(IsListEmpty(&sigSem->ss_WaitQueue)))
	    {
			struct SemaphoreRequest *sr;

			sr = (pSemRequest)RemHead((pList)&sigSem->ss_WaitQueue);
			sigSem->ss_NestCount++;
			sigSem->ss_Owner = sr->sr_Waiter;
			SignalTask(sr->sr_Waiter, SIGF_SEMAPHORE);
	    }else
	    {
	        sigSem->ss_Owner = NULL;
	        sigSem->ss_QueueCount = -1;
			sigSem->ss_Type = 0;
	    }
    } else if(sigSem->ss_NestCount < 0)
    {
    	//Alert( AN_SemCorrupt );
    }
    Permit();
	return OK;
}

void lib_ObtainSemaphoreShared(struct SysBase *SysBase, struct SignalSemaphore *sigSem)
{
	struct Task *me = FindTask(NULL);

	Forbid();
	//QueueCount >0 = locked
	sigSem->ss_QueueCount++;
	if(sigSem->ss_QueueCount == 0)
	{
		sigSem->ss_Owner = NULL;
		sigSem->ss_Type = SS_TYPE_SHARED;
		sigSem->ss_NestCount++;
	} else if((sigSem->ss_Owner == me) || (sigSem->ss_Owner == NULL)) 
	{
		sigSem->ss_NestCount++;
	} else
	{
		struct SemaphoreRequest sr;
		UINT32 ipl;
		sr.sr_Waiter = me;
		ipl = Disable();
		me->tcb_SigRecvd &= ~SIGF_SEMAPHORE;
		Restore(ipl);
		AddTail((struct List *)&sigSem->ss_WaitQueue, (struct Node *)&sr);
		WaitSignal(SIGF_SEMAPHORE);
		// We are getting it now.
		sigSem->ss_Owner = NULL;
		sigSem->ss_Type = SS_TYPE_SHARED;
		sigSem->ss_NestCount++;		
    }
	/* All Done! */
	Permit();
}

BOOL lib_AttemptSemaphoreShared(struct SysBase *SysBase, struct SignalSemaphore *sigSem)
{
	struct Task *me;
	me=(struct Task *)FindTask(NULL);

	Forbid();
	sigSem->ss_QueueCount++;
	if (sigSem->ss_QueueCount==0)
	{
		sigSem->ss_Owner=NULL;
		sigSem->ss_NestCount++;
		sigSem->ss_Type = SS_TYPE_SHARED;
		Permit();
		return TRUE;
	}

	if ((sigSem->ss_Owner==me)||(sigSem->ss_Owner==NULL))
	{
		sigSem->ss_NestCount++;
		Permit();
		return TRUE;
	}
	sigSem->ss_QueueCount--;
	Permit();
	return FALSE;
}

