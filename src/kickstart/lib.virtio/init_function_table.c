#include "lib_virtio_internal.h"
#include "sysbase.h"
#include "resident.h"
#include "exec_funcs.h"

char LibVirtioLibName[] = "lib_virtio.library";
char LibVirtioLibVer[] = "\0$VER: lib_virtio.library 0.1 ("__DATE__")\r\n";

APTR lib_virtio_FuncTab[] =
{
	(void(*)) lib_virtio_OpenLib,
	(void(*)) lib_virtio_CloseLib,
	(void(*)) lib_virtio_ExpungeLib,
	(void(*)) lib_virtio_ExtFuncLib,

	(void(*)) lib_virtio_Write8,
	(void(*)) lib_virtio_Write16,
	(void(*)) lib_virtio_Write32,
	(void(*)) lib_virtio_Read8,
	(void(*)) lib_virtio_Read16,
	(void(*)) lib_virtio_Read32,

	(void(*)) lib_virtio_ExchangeFeatures,
	(void(*)) lib_virtio_AllocateQueues,
	(void(*)) lib_virtio_InitQueues,
	(void(*)) lib_virtio_FreeQueues,
	(void(*)) lib_virtio_HostSupports,
	(void(*)) lib_virtio_GuestSupports,

	(APTR) ((UINT32)-1)
};

struct LibVirtioBase *lib_virtio_InitLib(struct LibVirtioBase *LibVirtioBase, UINT32 *segList, struct SysBase *SysBase)
{
	LibVirtioBase->SysBase = SysBase;


	return LibVirtioBase;
}

static const struct LibVirtioBase LibVirtioLibData =
{
	.Library.lib_Node.ln_Name = (APTR)&LibVirtioLibName[0],
	.Library.lib_Node.ln_Type = NT_LIBRARY,
	.Library.lib_Node.ln_Pri = -50,

	.Library.lib_OpenCnt = 0,
	.Library.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED,
	.Library.lib_NegSize = 0,
	.Library.lib_PosSize = 0,
	.Library.lib_Version = LIB_VIRTIO_VERSION,
	.Library.lib_Revision = LIB_VIRTIO_REVISION,
	.Library.lib_Sum = 0,
	.Library.lib_IDString = (APTR)&LibVirtioLibVer[7],

	//more (specific to library)

};

//Init table
struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} lib_virtio_InitTab =
{
	sizeof(struct LibVirtioBase),
	lib_virtio_FuncTab,
	(APTR)&LibVirtioLibData,
	lib_virtio_InitLib
};

static APTR LibVirtioEndResident;

// Resident ROMTAG
struct Resident LibVirtioRomTag =
{
	RTC_MATCHWORD,
	&LibVirtioRomTag,
	&LibVirtioEndResident,
	RTF_AUTOINIT | RTF_SINGLETASK,
	LIB_VIRTIO_VERSION,
	NT_LIBRARY,
	-50,
	LibVirtioLibName,
	LibVirtioLibVer,
	0,
	&lib_virtio_InitTab
};

