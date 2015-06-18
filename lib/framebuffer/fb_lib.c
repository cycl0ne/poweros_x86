/**
* File: /fb_lib．c
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "framebuffer.h"
#include "residents.h"
#include "dos_io.h"

#define LIBRARY_VERSION_STRING "\0$VER: framebuffer.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const char Name[] = "framebuffer.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static pFBBase OpenLib(pFBBase FBBase)
{
	FBBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return FBBase;
}

static pFBBase CloseLib(pFBBase FBBase)
{
	FBBase->Library.lib_OpenCnt--;
	return NULL;
}

static pFBBase ExpungeLib(pFBBase FBBase)
{
	if (FBBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pFBBase ExtFuncLib(pFBBase FBBase)
{
	if (FBBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pFBBase InitLibrary(pFBBase FBBase, pSegment segment, pSysBase SysBase)
{
	FBBase->fb_SysBase = SysBase;
	return FBBase;
}

/*******************

Function Table

********************/

APTR fb_Open(pFBBase FBBase);
void fb_Close(pFBBase FBBase, pSD psd);
void fb_AllocateMemGC(void);
void fb_FreeMemGC(void);
void fb_MapMemGC(void);

static volatile APTR FuncTab[] =
{
	(void(*)) OpenLib,
	(void(*)) CloseLib,
	(void(*)) ExpungeLib,
	(void(*)) ExtFuncLib,

	(void(*)) fb_Open,
	(void(*)) fb_Close,
	(void(*)) fb_AllocateMemGC,
	(void(*)) fb_FreeMemGC,
	(void(*)) fb_MapMemGC,

	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct FBBase FBBaseData =
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
	(APTR)sizeof(FBBase_t),
	(APTR)FuncTab,
	(APTR)&FBBaseData,
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
	100,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};



