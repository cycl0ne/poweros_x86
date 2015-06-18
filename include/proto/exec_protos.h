// Tasks.c
pTask FindTask(SysBase *SysBase, STRPTR name);
pTask CreateTask(SysBase *SysBase, STRPTR name, Task_Function codeStart, APTR data, UINT32 stackSize, INT8 pri);
INT8 SetTaskPri(SysBase *SysBase, struct Task *task, INT8 pri);
SysCall ReadyTask(SysBase *SysBase, pTask task, BOOL resch);
SysCall Forbid(SysBase *SysBase);
SysCall Permit(SysBase *SysBase);

//signal.c
Signal WaitSignal(SysBase *SysBase, Signal signalSet);
Signal SetSignal(SysBase *SysBase, Signal newSignals, Signal signalSet);
SysCall Signal(SysBase *SysBase, pTask task, Signal signalSet);

//ports.c
SysCall AddPort(SysBase *SysBase, pMsgPort msgPort);
pMsgPort FindPort(SysBase *SysBase, STRPTR name);
SysCall RemPort(SysBase *SysBase, pMsgPort msgPort);
pMessage WaitPort(SysBase *SysBase, pMsgPort msgPort);
pMsgPort CreateMsgPort(SysBase *SysBase);
SysCall DeleteMsgPort(SysBase *SysBase, pMsgPort msgPort);
pMessage GetMsg(SysBase *SysBase, pMsgPort msgPort);
SysCall PutMsg(SysBase *SysBase, pMsgPort msgPort, pMessage msg);
SysCall ReplyMsg(SysBase *SysBase, pMessage msg);

//list.c
void AddHead(SysBase *SysBase, pList list, pNode node);
void AddTail(SysBase *SysBase, pList list, pNode node);
void Enqueue(SysBase *SysBase, pList list, pNode node);
void Insert(SysBase *SysBase, pList list, pNode node, pNode pred);
void NewList(SysBase *SysBase, pList list);
void NewListType(SysBase *SysBase, pList list, UINT8 type);
void Remove(SysBase *SysBase, pNode node);
pNode RemTail(SysBase *SysBase, pList list);
pNode RemHead(SysBase *SysBase, pList list);
pNode FindName(SysBase *SysBase, pList liste, STRPTR name);

//memory.c
APTR Allocate(SysBase *SysBase, pMemHeader mh, UINT32 size);
void Deallocate(SysBase *SysBase, pMemHeader mh, void *p);
pMemHeader _CreateMemoryHead(UINT32 start_addr, UINT32 end_addr, UINT32 attr);
void AddMemList(SysBase *SysBase, UINT32 size, UINT32 attribute, INT32 pri, APTR base, STRPTR name);
APTR AllocVec(SysBase *SysBase, UINT32 byteSize, UINT32 requirements);
void FreeVec(SysBase *SysBase, APTR memoryBlock);
UINT32 AvailMem(SysBase *SysBase, UINT32 attributes);
APTR CopyMemQuick(SysBase *SysBase, const APTR src, APTR dest, int n); 
APTR CopyMem(SysBase *SysBase,const APTR src,  APTR dest, int n); 
APTR MemSet(SysBase *SysBase, void* m, int c, UINT32 len); 

//libraries.c
struct Library *OpenLibrary(SysBase *SysBase, STRPTR libName, UINT32 version);
SysCall CloseLibrary(SysBase *SysBase,struct Library *library);
SysCall AddLibrary(SysBase *SysBase,struct Library *library);
SysCall RemLibrary(SysBase *SysBase, struct Library *library);
void SumLibrary(SysBase *SysBase, struct Library *library);
SysCall DisposeLibrary(SysBase *SysBase, struct Library* library);
APTR SetFunction(struct SysBase *SysBase, struct Library *library, INT32 funcOffset, APTR newFunction);
struct Library *MakeLibrary(SysBase *SysBase, APTR funcTable, APTR structInit, UINT32(*libInit);(struct Library*,APTR, struct SysBase*);, UINT32 dataSize, UINT32 segList);
SysCall MakeFunctions(SysBase *SysBase, APTR target, APTR functionArray);

//devices.c
SysCall AddDevice(SysBase *SysBase, struct Device *device);
SysCall RemDevice(SysBase *SysBase, struct Device *device);
SysCall CloseDevice(SysBase *SysBase, pIOStdReq iORequest);
SysCall OpenDevice(struct SysBase *SysBase, STRPTR devName, UINT32 unitNum, pIOStdReq iORequest, UINT32 flags);
pIOStdReq CreateIORequest(SysBase *SysBase, pMsgPort ioReplyPort);
SysCall DeleteIORequest(SysBase *SysBase, pIOStdReq iorequest);
SysCall AbortIO(SysBase *SysBase, pIOStdReq iORequest);
pIORequest CheckIO(SysBase *SysBase, pIOStdReq iORequest);
SysCall DoIO(SysBase *SysBase, pIOStdReq iORequest);
SysCall SendIO(SysBase *SysBase, pIOStdReq io);
SysCall WaitIO(SysBase *SysBase, struct IORequest *iORequest);

//interrupts.c
void Enable(struct SysBase *SysBase);
UINT32 Disable(struct SysBase *SysBase);
void Restore(struct SysBase *SysBase, UINT32 ipl);
void AddIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr);
void RemIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr);
pInterrupt SetExcVector(SysBase *SysBase, UINT32 excNumber, struct Interrupt *isr);
pInterrupt CreateIntServer(SysBase *SysBase, const STRPTR name, INT8 pri, APTR handler, APTR data);

//resident.c
APTR InitResident(SysBase *SysBase, struct Resident *resident, APTR segList);
SysCall InitResidentCode(struct SysBase *SysBase, UINT32 startClass);

//rawio.c
INT32 RawMayGetChar(struct SysBase *SysBase);
void RawPutChar(struct SysBase *SysBase, UINT8 chr);
void RawIOInit(struct SysBase *SysBase);
va_list RawDoFmt(struct SysBase *SysBase, const char *fmt, va_list ap, void (*PutCh);(INT32, APTR);, APTR PutChData);

//semaphores.c
SysCall InitSemaphore(struct SysBase *SysBase, pSignalSemaphore signalSemaphore);
SysCall AddSemaphore(struct SysBase *SysBase, const char *semName, struct SignalSemaphore *sigSem);
SysCall RemSemaphore(struct SysBase *SysBase, struct SignalSemaphore *sigSem);
pSignalSemaphore FindSemaphore(struct SysBase *SysBase, const char *name);
SysCall ObtainSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem);
SysCall AttemptSemaphore(struct SysBase *SysBase, struct SignalSemaphore *signalSemaphore);
SysCall ReleaseSemaphore(struct SysBase *SysBase, pSignalSemaphore sigSem);

//schedule.c
void Reschedule(SysBase *SysBase);
SysCall YieldCPU(SysBase *SysBase);


