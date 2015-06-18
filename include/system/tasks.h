#ifndef tasks_h
#define tasks_h

#include "types.h"
#include "lists.h"
#include "arch_context.h"

/* thread state constants                                               */
#define THRCURR     1           /**< thread is currently running        */
#define THRFREE     2           /**< thread slot is free                */
#define THRREADY    3           /**< thread is on ready queue           */
#define THRRECV     4           /**< thread waiting for message         */
#define THRSLEEP    5           /**< thread is sleeping                 */
#define THRSUSP     6           /**< thread is suspended                */
#define THRWAIT     7           /**< thread is on semaphore queue       */
#define THRTMOUT    8           /**< thread is receiving with timeout   */
#define THRMIGRATE  9           /**< thread is being migrated           */

#define TASK_RUN		1
#define TASK_READY		2
#define	TASK_WAIT		3
#define	TASK_FREE		4
#define TASK_SUSPENDED	5
#define TASK_WAITSIGNAL	6

// To be cleaned!
// Special Signals SYSTEMUSE
#define SIGB_ABORT	0
#define SIGB_CHILD	1
#define SIGB_BLIT	4	/* Note: same as SINGLE */
#define SIGB_SINGLE	4	/* Note: same as BLIT */
#define SIGB_SEMAPHORE	4	/* Note: same as both */
#define SIGB_INTUITION	5
#define SIGB_NET	7
#define SIGB_DOS	8

#define SIGF_SYSTEM  SIGF_DOS|SIGF_NET|SIGF_INTUITION|SIGF_SINGLE  //Reserved for Systemuse

#define SIGF_ABORT		(1L<<0)
#define SIGF_CHILD		(1L<<1)

#define SIGF_BLIT		(1L<<4)
#define SIGF_SINGLE		(1L<<4)
#define SIGF_SEMAPHORE	(1L<<4)

#define SIGF_INTUITION	(1L<<5)
#define SIGF_NET		(1L<<7)
#define SIGF_DOS		(1L<<8)

//TODO: CLEANIT !

typedef void (*Task_Function)(void *, void *);

typedef struct Task
{
	Node			tcb_Node;
	UINT8			tcb_Flags;
	UINT8			tcb_State;
	INT8			tcb_TDNestCnt;
	INT8			tcb_IDNestCnt;
	
	struct Task		*tcb_Parent;
	IRQMask			tcb_IntMask;
	UINT32			tcb_CPUUsage;
	Task_Function	tcb_Switch;
	Task_Function	tcb_Launch;
	Signal			tcb_SigAlloc;
	Signal			tcb_SigWait;
	Signal			tcb_SigRecvd;

	Task_Function	tcb_TaskFunc;
	APTR			tcb_TaskArg;
	Context			tcb_SavedContext;
	Context			tcb_WaitContext;
	UINT8			*tcb_Stack;
	uintptr_t		tcb_SPLower;
	uintptr_t		tcb_SPUpper;
	UINT32			tcb_StackSize;	
} Task, *pTask;

#endif
