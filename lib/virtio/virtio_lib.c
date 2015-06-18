/**
* File: /fb_lib．c
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "virtio_lib.h"
#include "residents.h"
#include "dos_io.h"

#define LIBRARY_VERSION_STRING "\0$VER: virtio.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const char Name[] = "virtio.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static pVIOBase OpenLib(pVIOBase VIOBase)
{
	VIOBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return VIOBase;
}

static pVIOBase CloseLib(pVIOBase VIOBase)
{
	VIOBase->Library.lib_OpenCnt--;
	return NULL;
}

static pVIOBase ExpungeLib(pVIOBase VIOBase)
{
	if (VIOBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pVIOBase ExtFuncLib(pVIOBase VIOBase)
{
	if (VIOBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pVIOBase InitLibrary(pVIOBase VIOBase, pSegment segment, pSysBase SysBase)
{
	(void)segment;
	VIOBase->SysBase = SysBase;
	return VIOBase;
}

/*******************

Function Table

********************/

void virtio_Write8(void);
void virtio_Write16(void);
void virtio_Write32(void);
void virtio_Read8(void);
void virtio_Read16(void);
void virtio_Read32(void);
void virtio_ExchangeFeatures(void);
void virtio_AllocateQueues(void);
void virtio_InitQueues(void);
void virtio_FreeQueues(void);
void virtio_HostSupports(void);
void virtio_GuestSupports(void);

static volatile APTR FuncTab[] =
{
	(void(*)) OpenLib,
	(void(*)) CloseLib,
	(void(*)) ExpungeLib,
	(void(*)) ExtFuncLib,

	(void(*)) virtio_Write8,
	(void(*)) virtio_Write16,
	(void(*)) virtio_Write32,
	(void(*)) virtio_Read8,
	(void(*)) virtio_Read16,
	(void(*)) virtio_Read32,

	(void(*)) virtio_ExchangeFeatures,
	(void(*)) virtio_AllocateQueues,
	(void(*)) virtio_InitQueues,
	(void(*)) virtio_FreeQueues,
	(void(*)) virtio_HostSupports,
	(void(*)) virtio_GuestSupports,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct VIOBase VIOBaseData =
{
	.Library.lib_Node.ln_Name = (APTR)&Name[0],
	.Library.lib_Node.ln_Type = NT_LIBRARY,
	.Library.lib_Node.ln_Pri = 0,
	.Library.lib_OpenCnt = 0,
	.Library.lib_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.Library.lib_NegSize = 0,
	.Library.lib_PosSize = 0,
	.Library.lib_Version = LIBRARY_VERSION,
	.Library.lib_Revision = LIBRARY_REVISION,
	.Library.lib_Sum = 0,
	.Library.lib_IDString = (APTR)&Version[7],
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(VIOBase_t),
	(APTR)FuncTab,
	(APTR)&VIOBaseData,
	(APTR)InitLibrary
};

static const char EndResident = 0;

static const volatile RESIDENT_TAG ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_SINGLETASK,
	LIBRARY_VERSION,
	NT_LIBRARY,
	109,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};



