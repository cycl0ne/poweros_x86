#include "regionbase.h"

#define LIBRARY_VERSION_STRING "\0$VER: region.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "region.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static RegionBase *region_Init(RegionBase *RegionBase, UINT32 *segList, APTR SysBase);
APTR region_OpenLib(RegionBase *RegionBase);
APTR region_CloseLib(RegionBase *RegionBase);
APTR region_ExpungeLib(RegionBase *RegionBase);
APTR region_ExtFuncLib(RegionBase *RegionBase);

void SVGA_Init(RegionBase *RegionBase);

static volatile APTR FuncTab[] = 
{
	(void(*)) region_OpenLib,
	(void(*)) region_CloseLib,
	(void(*)) region_ExpungeLib,
	(void(*)) region_ExtFuncLib,

	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(RegionBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)region_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK,
	LIBRARY_VERSION,
	NT_LIBRARY,
	95,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

static RegionBase *region_Init(RegionBase *RegionBase, UINT32 *segList, APTR SysBase)
{
	RegionBase->Library.lib_OpenCnt = 0;
	RegionBase->Library.lib_Node.ln_Pri = 0;
	RegionBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	RegionBase->Library.lib_Node.ln_Name = (STRPTR)name;
	RegionBase->Library.lib_Version = LIBRARY_VERSION;
	RegionBase->Library.lib_Revision = LIBRARY_REVISION;
	RegionBase->Library.lib_IDString = (STRPTR)&version[7];	
	RegionBase->SysBase	= SysBase;
	return RegionBase;
}

static const char EndResident = 0;
