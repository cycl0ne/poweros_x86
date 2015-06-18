/**
* File: /clib．c
* User: cycl0ne
* Date: 2014-11-16
* Time: 07:38 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "clib.h"
#include "residents.h"
#include "dos_io.h"

#define LIBRARY_VERSION_STRING "\0$VER: clib.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const char Name[] = "clib.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static pCLibBase OpenLib(pCLibBase CLibBase)
{
	CLibBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return CLibBase;
}

static pCLibBase CloseLib(pCLibBase CLibBase)
{
	CLibBase->Library.lib_OpenCnt--;
	return NULL;
}

static pCLibBase ExpungeLib(pCLibBase CLibBase)
{
	if (CLibBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pCLibBase ExtFuncLib(pCLibBase CLibBase)
{
	if (CLibBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pCLibBase InitLibrary(pCLibBase CLibBase, pSegment segment, pSysBase execBase)
{
	//pSysBase SysBase = execBase;
	CLibBase->lib_SysBase = execBase;
	return CLibBase;
}

/*******************

Function Table

********************/

static volatile APTR FuncTab[] =
{
	(void(*)) OpenLib,
	(void(*)) CloseLib,
	(void(*)) ExpungeLib,
	(void(*)) ExtFuncLib,

	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct CLIBBase CLibBaseData =
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
	(APTR)sizeof(CLibBase_t),
	(APTR)FuncTab,
	(APTR)&CLibBaseData,
	(APTR)InitLibrary
};

static const char EndResident = 0;

static const volatile RESIDENT_TAG ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	LIBRARY_VERSION,
	NT_LIBRARY,
	103,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};


