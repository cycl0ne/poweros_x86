#ifndef sysbase_h
#define sysbase_h

#include "types.h"
#include "lists.h"
#include "tasks.h"
#include "interrupts.h"
#include "devices.h"
#include "libraries.h"
#include "residents.h"
#include "semaphores.h"
#include "memory.h"
#include "ports.h"

#define SYSFLG_RESCHEDULE	1<<1
#define SYSFLG_NOSCHEDULE	1<<2

#define DBGLOGSIZE 4096*2

typedef struct SysBase 
{
	Library		LibNode;
	pTask		thisTask;
	UINT32		IdleCnt;
	UINT32		DispCnt;
	UINT32		SysFlags;
	INT8		TDNestCnt;
	INT8		IDNestCnt;
	INT16		nDefer;
	BOOL		attemptDefer;
	List		TaskReady;
	List		TaskWait;
	List		PortList;
	List		MemList;
	List		DevList;
	List		LibList;
	List		SemaphoreList;
	List		ResourceList;
	List		ResidentList;
	pInterrupt	ExcVector[64];
	List		IntVectorList[16];
	Task		InitTask;
	pTask		ExecTask;
	pTask		IdleTask;
	UINT8		CPU_Stack[2048];
	Context		CPU_Context;
	uint8_t		DBG_Log[DBGLOGSIZE];
	uint32_t	DBG_Cnt;
	uint32_t	IRQ_Flags;			//IRQ Flags
} SysBase;



// Internal Prototypes
//execlib.c

APTR lib_OpenLib(struct SysBase *SysBase);
APTR lib_CloseLib(struct SysBase *SysBase);
APTR lib_ExpungeLib(struct SysBase *SysBase);
APTR lib_ExtFuncLib(void);

// Tasks.c
SysCall lib_AddTask(SysBase *SysBase, pTask task, Task_Function codeStart, APTR data);
pTask lib_FindTask(SysBase *SysBase, STRPTR name);
pTask lib_CreateTask(SysBase *SysBase, STRPTR name, Task_Function codeStart, APTR data, UINT32 stackSize, INT8 pri);
INT8 lib_SetTaskPri(SysBase *SysBase, struct Task *task, INT8 pri);
SysCall lib_ReadyTask(SysBase *SysBase, pTask task, BOOL resch);
SysCall lib_Forbid(SysBase *SysBase);
SysCall lib_Permit(SysBase *SysBase);

//signal.c
Signal lib_WaitSignal(SysBase *SysBase, Signal signalSet);
Signal lib_SetSignal(SysBase *SysBase, Signal newSignals, Signal signalSet);
SysCall lib_SignalTask(SysBase *SysBase, pTask task, Signal signalSet);
INT8 lib_AllocSignal(SysBase *SysBase, INT32 signalNum);
SysCall lib_FreeSignal(SysBase *SysBase, INT32 signalNum);

//semaphores.c
SysCall lib_InitSemaphore(struct SysBase *SysBase, pSignalSemaphore signalSemaphore);
SysCall lib_AddSemaphore(struct SysBase *SysBase, const char *semName, struct SignalSemaphore *sigSem);
SysCall lib_RemSemaphore(struct SysBase *SysBase, struct SignalSemaphore *sigSem);
pSignalSemaphore lib_FindSemaphore(struct SysBase *SysBase, const char *name);
SysCall lib_ObtainSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem);
SysCall lib_AttemptSemaphore(struct SysBase *SysBase, struct SignalSemaphore *signalSemaphore);
SysCall lib_ReleaseSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem);
void lib_ObtainSemaphoreShared(struct SysBase *SysBase, struct SignalSemaphore *sigSem);
BOOL lib_AttemptSemaphoreShared(struct SysBase *SysBase, struct SignalSemaphore *sigSem);

//schedule.c
void lib_Reschedule(SysBase *SysBase);
SysCall lib_YieldCPU(SysBase *SysBase);

//resident.c
APTR lib_InitResident(SysBase *SysBase, struct Resident *resident, APTR segList);
SysCall lib_InitResidentCode(struct SysBase *SysBase, UINT32 startClass);
pResidentNode lib_FindResident(struct SysBase *SysBase, STRPTR name);

//rawio.c
INT32 lib_RawMayGetChar(struct SysBase *SysBase);
void lib_RawPutChar(struct SysBase *SysBase, UINT8 chr);
void lib_RawIOInit(struct SysBase *SysBase);
va_list lib_RawDoFmt(struct SysBase *SysBase, const INT8 *fmt, va_list ap, void (*PutCh)(INT32, APTR), APTR PutChData);

//ports.c
SysCall lib_AddPort(SysBase *SysBase, pMsgPort msgPort);
pMsgPort lib_FindPort(SysBase *SysBase, STRPTR name);
SysCall lib_RemPort(SysBase *SysBase, pMsgPort msgPort);
pMessage lib_WaitPort(SysBase *SysBase, pMsgPort msgPort);
pMsgPort lib_CreateMsgPort(SysBase *SysBase, pMsgPort port);
SysCall lib_DeleteMsgPort(SysBase *SysBase, pMsgPort msgPort);
pMessage lib_GetMsg(SysBase *SysBase, pMsgPort msgPort);
SysCall lib_PutMsg(SysBase *SysBase, pMsgPort msgPort, pMessage msg);
SysCall lib_ReplyMsg(SysBase *SysBase, pMessage msg);

//memory.c
APTR lib_Allocate(SysBase *SysBase, pMemHeader mh, UINT32 size);
void lib_Deallocate(SysBase *SysBase, pMemHeader mh, void *p);
pMemHeader _CreateMemoryHead(UINT32 start_addr, UINT32 end_addr, UINT32 attr);
void lib_AddMemList(SysBase *SysBase, UINT32 size, UINT32 attribute, INT32 pri, APTR base, STRPTR name);
APTR lib_AllocVec(SysBase *SysBase, UINT32 byteSize, UINT32 requirements);
void lib_FreeVec(SysBase *SysBase, APTR memoryBlock);
UINT32 lib_AvailMem(SysBase *SysBase, UINT32 attributes);
APTR lib_CopyMemQuick(SysBase *SysBase, const APTR src, APTR dest, int n);
APTR lib_CopyMem(SysBase *SysBase,const APTR src,  APTR dest, int n);
APTR lib_MemSet(SysBase *SysBase, void* m, int c, UINT32 len);
APTR lib_CreatePool(SysBase *SysBase, UINT32 requirements, UINT32 puddleSize, UINT32 threshSize);

void lib_DeletePool(SysBase *SysBase, pPool poolHeader);
APTR lib_AllocPooled(SysBase *SysBase, pPool poolHeader, UINT32 memSize);
void lib_FreePooled(SysBase *SysBase, pPool poolHeader, APTR memory, UINT32 memSize);

//list.c
void lib_AddHead(SysBase *SysBase, pList list, pNode node);
void lib_AddTail(SysBase *SysBase, pList list, pNode node);
void lib_Enqueue(SysBase *SysBase, pList list, pNode node);
void lib_Insert(SysBase *SysBase, pList list, pNode node, pNode pred);
void lib_NewList(SysBase *SysBase, pList list);
void lib_NewListType(SysBase *SysBase, pList list, UINT8 type);
void lib_Remove(SysBase *SysBase, pNode node);
pNode lib_RemTail(SysBase *SysBase, pList list);
pNode lib_RemHead(SysBase *SysBase, pList list);
pNode lib_FindName(SysBase *SysBase, pList liste, STRPTR name);

//libraries.c
struct Library *lib_OpenLibrary(SysBase *SysBase, STRPTR libName, UINT32 version);
SysCall lib_CloseLibrary(SysBase *SysBase,struct Library *library);
SysCall lib_AddLibrary(SysBase *SysBase,struct Library *library);
SysCall lib_RemLibrary(SysBase *SysBase, struct Library *library);
void lib_SumLibrary(SysBase *SysBase, struct Library *library);
SysCall lib_DisposeLibrary(SysBase *SysBase, struct Library* library);
APTR lib_SetFunction(struct SysBase *SysBase, struct Library *library, INT32 funcOffset, APTR newFunction);
struct Library *lib_MakeLibrary(SysBase *SysBase, APTR funcTable, APTR structInit, UINT32(*libInit)(struct Library*,APTR, struct SysBase*), UINT32 dataSize, UINT32 segList);
SysCall lib_MakeFunctions(SysBase *SysBase, APTR target, APTR functionArray);

//devices.c
SysCall lib_AddDevice(SysBase *SysBase, struct Device *device);
SysCall lib_RemDevice(SysBase *SysBase, struct Device *device);
SysCall lib_CloseDevice(SysBase *SysBase, pIOStdReq iORequest);
SysCall lib_OpenDevice(struct SysBase *SysBase, STRPTR devName, UINT32 unitNum, pIOStdReq iORequest, UINT32 flags);
pIOStdReq lib_CreateIORequest(SysBase *SysBase, pMsgPort ioReplyPort, UINT32 size);
SysCall lib_DeleteIORequest(SysBase *SysBase, pIOStdReq iorequest);
SysCall lib_AbortIO(SysBase *SysBase, pIOStdReq iORequest);
pIOStdReq lib_CheckIO(SysBase *SysBase, pIOStdReq iORequest);
SysCall lib_DoIO(SysBase *SysBase, pIOStdReq iORequest);
SysCall lib_SendIO(SysBase *SysBase, pIOStdReq io);
SysCall lib_WaitIO(SysBase *SysBase, pIOStdReq iORequest);


//interrupts.c
void lib_Enable(struct SysBase *SysBase);
UINT32 lib_Disable(struct SysBase *SysBase);
void lib_Restore(struct SysBase *SysBase, UINT32 ipl);
void lib_AddIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr);
void lib_RemIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr);
pInterrupt lib_SetExcVector(SysBase *SysBase, UINT32 excNumber, struct Interrupt *isr);
pInterrupt lib_CreateIntServer(SysBase *SysBase, const STRPTR name, INT8 pri, APTR handler, APTR data);

void lib_KPrintF(struct SysBase *SysBase, char *fmt, ...);

#endif
