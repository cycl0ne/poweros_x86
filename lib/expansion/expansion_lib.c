/**
* File: /expansion_lib．c
* User: cycl0ne
* Date: 2014-11-18
* Time: 11:52 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "expansion.h"
#include "residents.h"
#include "dos_io.h"
#include "pci.h"

#define LIBRARY_VERSION_STRING "\0$VER: expansion.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const char Name[] = "expansion.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static pExpansionBase OpenLib(pExpansionBase ExpansionBase)
{
	ExpansionBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;

	return ExpansionBase;
}

static pExpansionBase CloseLib(pExpansionBase ExpansionBase)
{
	ExpansionBase->Library.lib_OpenCnt--;
	return NULL;
}

static pExpansionBase ExpungeLib(pExpansionBase ExpansionBase)
{
	if (ExpansionBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pExpansionBase ExtFuncLib(pExpansionBase ExpansionBase)
{
	if (ExpansionBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pExpansionBase InitLibrary(pExpansionBase ExpansionBase, pSegment segment, pSysBase SysBase)
{
	ExpansionBase->lib_SysBase = SysBase;

	NewListType(&ExpansionBase->BoardList, NT_PCILIST);
	InitSemaphore(&ExpansionBase->BoardListLock);
	NewListType(&ExpansionBase->MountList, NT_DOSLIST);	
	InitSemaphore(&ExpansionBase->MountListLock);
	
	return ExpansionBase;
}

/*******************

Prototypes

********************/
BOOL exp_AddDosNode(pExpansionBase ExpansionBase, INT32 bootPri, UINT32 flags, pDosEntry deviceNode);
struct DeviceNode *exp_MakeDosNode(pExpansionBase ExpansionBase, struct ExpDosNode *parameter);

UINT32 PCI_ConfigRead32(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset);
UINT16 PCI_ConfigRead16(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset);
UINT8 PCI_ConfigRead8(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset);
void PCI_ConfigWrite32(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset, UINT32 data);
void PCI_ConfigWrite16(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset, UINT16 data);
void PCI_ConfigWrite8(pExpansionBase ExpBase, const PCIAddress *addr, UINT16 offset, UINT8 data);
BOOL PCI_ScanBus(pExpansionBase ExpBase, PCIScanState *state);
BOOL PCI_FindDevice(pExpansionBase ExpBase, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut);
void PCI_SetBAR(pExpansionBase ExpBase, const PCIAddress *addr, INT32 index, UINT32 value);
UINT32 PCI_GetBARAddr(pExpansionBase ExpBase, const PCIAddress *addr, INT32 index);
void PCI_SetMemEnable(pExpansionBase ExpBase, const PCIAddress *addr, BOOL enable);
UINT8 PCI_GetIntrLine(pExpansionBase ExpBase, const PCIAddress *addr);
UINT8 PCI_GetIntrPin(pExpansionBase ExpBase, const PCIAddress *addr);
BOOL PCI_FindDeviceByUnit(pExpansionBase ExpBase, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut, INT32 unit);

/*******************

Function Table

********************/

static volatile APTR FuncTab[] =
{
	(void(*)) OpenLib,
	(void(*)) CloseLib,
	(void(*)) ExpungeLib,
	(void(*)) ExtFuncLib,
	(void(*)) exp_AddDosNode,
	(void(*)) exp_MakeDosNode,
	(void(*)) PCI_ConfigRead8,
	(void(*)) PCI_ConfigRead16,
	(void(*)) PCI_ConfigRead32,
	(void(*)) PCI_ConfigWrite8,
	(void(*)) PCI_ConfigWrite16,
	(void(*)) PCI_ConfigWrite32,

	(void(*)) PCI_ScanBus,
	(void(*)) PCI_FindDevice,
	(void(*)) PCI_SetBAR,
	(void(*)) PCI_GetBARAddr,
	(void(*)) PCI_SetMemEnable,

	(void(*)) PCI_GetIntrLine,
	(void(*)) PCI_GetIntrPin,
	(void(*)) PCI_FindDeviceByUnit,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct ExpansionBase LibBaseData =
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
	(APTR)sizeof(ExpansionBase_t),
	(APTR)FuncTab,
	(APTR)&LibBaseData,
	(APTR)InitLibrary
};

static const char EndResident = 0;

static const volatile RESIDENT_TAG ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_SINGLETASK, //RTF_COLDSTART,
	LIBRARY_VERSION,
	NT_LIBRARY,
	110,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};



