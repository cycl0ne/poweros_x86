#include "intuitionbase.h"

#define LIBRARY_VERSION_STRING	"\0$VER: intuition.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = LIBRARY_NAME
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static IntuitionBase *intu_Init(IntuitionBase *IBase, UINT32 *segList, APTR SysBase);
APTR intu_OpenLib(IntuitionBase *IBase);
APTR intu_CloseLib(IntuitionBase *IBase);
APTR intu_ExpungeLib(IntuitionBase *IBase);
APTR intu_ExtFuncLib(IntuitionBase *IBase);

static volatile APTR FuncTab[] = 
{
	(void(*)) intu_OpenLib,
	(void(*)) intu_CloseLib,
	(void(*)) intu_ExpungeLib,
	(void(*)) intu_ExtFuncLib,
	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(IntuitionBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)intu_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTW_COLDSTART|RTF_AUTOINIT,
	LIBRARY_VERSION,
	NT_LIBRARY,
	20, // Fix
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

void initIntuition(pIntuitionBase IBase);

static IntuitionBase *intu_Init(IntuitionBase *IBase, UINT32 *segList, APTR SysBase)
{
	IBase->ib_Library.lib_OpenCnt = 0;
	IBase->ib_Library.lib_Node.ln_Pri = 0;
	IBase->ib_Library.lib_Node.ln_Type = NT_LIBRARY;
	IBase->ib_Library.lib_Node.ln_Name = (STRPTR)name;
	IBase->ib_Library.lib_Version = LIBRARY_VERSION;
	IBase->ib_Library.lib_Revision = LIBRARY_REVISION;
	IBase->ib_Library.lib_IDString = (STRPTR)&version[7];	
	IBase->ib_SysBase	= SysBase;

//	initIntuition(IBase);
	return IBase;
}

static const char EndResident = 0;

