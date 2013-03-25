#ifndef sysbase_h
#define sysbase_h

#include "types.h"
#include "library.h"
#include "tasks.h"
#include "context.h"
#include "irq.h"

typedef void(*IRQHandler)(APTR, UINT32);


typedef struct SysBase {
	Library	LibNode;
	Task		*thisTask;
	UINT32	IdleCnt;
	UINT32	DispCnt;
	UINT32	Quantum;
	UINT32	QUsed;
	UINT32	SysFlags;
	INT8		TDNestCnt;
	INT8		IDNestCnt;
	INT16	Padding;
	List		TaskReady;
	List		TaskWait;
	List		PortList;
	List		MemList;
	List		DevList;
	List		LibList;
	List		SemaphoreList;
	List		ResourceList;
	List		ResidentList;
	struct Interrupt	*ExcVector[64];
	List		IntVectorList[16];
	UINT8			*CPU_Stack;
	context_t			CPU_Context;
} SysBase;

#define _CPUSTACK_ 0x1000

#endif
