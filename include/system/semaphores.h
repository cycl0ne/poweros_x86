#ifndef semaphores_h
#define semaphores_h

#include "types.h"
#include "lists.h"
#include "ports.h"
#include "tasks.h"

#define SS_TYPE_SIMPLE	1
#define SS_TYPE_SHARED	2

#define SM_EXCLUSIVE (0L)
#define SM_SHARED    (1L)

typedef struct SemaphoreRequest
{
	MinNode	sr_Link;
	pTask	sr_Waiter;
} SemRequest, *pSemRequest;

typedef struct SignalSemaphore
{
	Node		ss_Link;
	pTask		ss_Owner;
	INT32		ss_NestCount;
	INT32		ss_QueueCount;
	UINT32		ss_Type;
	MinList		ss_WaitQueue;
	SemRequest	ss_MultipleLink;
} SignalSemaphore, *pSignalSemaphore;

struct SemaphoreMessage
{
    Message			ssm_Message;
    SignalSemaphore	*ssm_Semaphore;
};

#endif
