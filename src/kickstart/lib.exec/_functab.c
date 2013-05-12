#include "exec_proto_lib.h"
#include "exec_funcs.h"

APTR FuncTab[] =
{
	(void(*)) lib_OpenLib,
	(void(*)) lib_CloseLib,
	(void(*)) lib_ExpungeLib,
	(void(*)) lib_ExtFuncLib,
	
	(void(*)) lib_Permit,
	(void(*)) lib_Forbid,
	(void(*)) lib_Enable,
	(void(*)) lib_Disable,

	(void(*)) lib_NewList,
	(void(*)) lib_Enqueue,
	(void(*)) lib_FindName,
	(void(*)) lib_RemTail,
	(void(*)) lib_AddTail,
	(void(*)) lib_RemHead,
	(void(*)) lib_AddHead,
	(void(*)) lib_Remove,
	(void(*)) lib_Insert,
	(void(*)) lib_NewListType,

	(void(*)) lib_FindTask,
	(void(*)) lib_AddTask,
	(void(*)) lib_AddMemList,
	(void(*)) lib_Allocate,
	(void(*)) lib_Deallocate,
	(void(*)) lib_AllocVec,
	(void(*)) lib_FreeVec,
	
	(void(*)) lib_Schedule,
	(void(*)) NULL,  // Dispatch

	(void(*)) lib_GetMsg,
	(void(*)) lib_PutMsg,
	(void(*)) lib_ReplyMsg,
	(void(*)) lib_Signal,
	(void(*)) lib_Wait,

	(void(*)) lib_OpenDevice,
	(void(*)) lib_CloseDevice,
	(void(*)) lib_RemDevice,
	(void(*)) lib_AddDevice,

	(void(*)) lib_AbortIO,
	(void(*)) lib_CreateIORequest,
	(void(*)) lib_DeleteIORequest,
	(void(*)) lib_CheckIO,
	(void(*)) lib_DoIO,
	(void(*)) lib_SendIO,
	(void(*)) lib_WaitIO,

	(void(*)) lib_AddPort,
	(void(*)) lib_FindPort,
	(void(*)) lib_RemPort,
	(void(*)) lib_WaitPort,
	(void(*)) lib_CreateMsgPort,
	(void(*)) lib_DeleteMsgPort,
	
	(void(*)) lib_AllocSignal,
	(void(*)) lib_FreeSignal,

	(void(*)) lib_OpenLibrary,
	(void(*)) lib_CloseLibrary,
	(void(*)) lib_AddLibrary,
	(void(*)) lib_RemLibrary,
	(void(*)) lib_SumLibrary,
	(void(*)) lib_Alert,
	(void(*)) lib_MakeLibrary,
	(void(*)) lib_RomTagScanner,
	(void(*)) lib_InitResident,
	(void(*)) lib_MakeFunctions,
	(void(*)) lib_SetTaskPri,

	(void(*)) NULL, //lib_SNPrintF,
	(void(*)) NULL, //lib_VPrintF,
	(void(*)) NULL, //lib_VSNPrintF,
	(void(*)) lib_MemSet,
	(void(*)) lib_CopyMem,

	(void(*)) lib_SetSignal,


	(void(*)) lib_AddIntServer,
	(void(*)) lib_RemIntServer,
	(void(*)) lib_CreateIntServer,
	(void(*)) NULL, //lib_AddExcServer,
	(void(*)) lib_SetExcVector,

	(void(*)) lib_DPrintF,
	(void(*)) lib_TaskCreate,

	(void(*)) lib_InitSemaphore,
	(void(*)) lib_AddSemaphore,
	(void(*)) lib_RemSemaphore,
	(void(*)) lib_FindSemaphore,
	(void(*)) lib_ObtainSemaphore,
	(void(*)) lib_AttemptSemaphore,
	(void(*)) lib_ReleaseSemaphore,
	(void(*)) lib_ObtainSemaphoreShared,
	(void(*)) lib_AttemptSemaphoreShared,
	(void(*)) lib_InitResidentCode,
	(void(*)) lib_SetFunction,
	(void(*)) lib_RawDoFmt,
	(void(*)) lib_RawIOInit,
	(void(*)) lib_RawPutChar,
	(void(*)) lib_RawMayGetChar,
	(void(*)) lib_CopyMemQuick,
	(void(*)) lib_AvailMem,
	(void(*)) lib_SendQuickIO,
	(APTR) ((INT32)-1)
};

