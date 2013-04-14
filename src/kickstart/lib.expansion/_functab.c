#include "expansionbase.h"
#include "resident.h"

#include "exec_funcs.h"
#include "expansion_protos.h"
#include "expansion_funcs.h"

#define LIBRARY_VERSION_STRING "\0$VER: expansion.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "expansion.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static struct ExpansionBase *exp_Init(struct ExpansionBase *ExpBase, UINT32 *segList, APTR SysBase);
APTR exp_OpenLib(struct ExpansionBase *ExpBase);
APTR exp_CloseLib(struct ExpansionBase *ExpBase);
APTR exp_ExpungeLib(struct ExpansionBase *ExpBase);
APTR exp_ExtFuncLib(void);

UINT32 PCI_ConfigRead32(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset);
UINT16 PCI_ConfigRead16(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset);
UINT8 PCI_ConfigRead8(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset);
void PCI_ConfigWrite32(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT32 data);
void PCI_ConfigWrite16(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT16 data);
void PCI_ConfigWrite8(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT8 data);
BOOL PCI_ScanBus(ExpansionBase *ExpBase, PCIScanState *state);
BOOL PCI_FindDevice(ExpansionBase *ExpBase, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut);
void PCI_SetBAR(ExpansionBase *ExpBase, const PCIAddress *addr, INT32 index, UINT32 value);
UINT32 PCI_GetBARAddr(ExpansionBase *ExpBase, const PCIAddress *addr, INT32 index);
void PCI_SetMemEnable(ExpansionBase *ExpBase, const PCIAddress *addr, BOOL enable);

static volatile APTR FuncTab[] =
{
	(void(*)) exp_OpenLib,
	(void(*)) exp_CloseLib,
	(void(*)) exp_ExpungeLib,
	(void(*)) exp_ExtFuncLib,
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
	(APTR) ((UINT32)-1)
};

static const struct ExpansionBase ExpansionLibData =
{
  .Library.lib_Node.ln_Name = (APTR)&name[0],
  .Library.lib_Node.ln_Type = NT_LIBRARY,
  .Library.lib_Node.ln_Pri = 110,

  .Library.lib_OpenCnt = 0,
  .Library.lib_Flags = 0,
  .Library.lib_NegSize = 0,
  .Library.lib_PosSize = 0,
  .Library.lib_Version = LIBRARY_VERSION,
  .Library.lib_Revision = LIBRARY_REVISION,
  .Library.lib_Sum = 0,
  .Library.lib_IDString = (APTR)&version[7]
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(struct ExpansionBase),
	(APTR)FuncTab,
	(APTR)&ExpansionLibData,
	(APTR)exp_Init
};

static const volatile struct Resident ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT | RTF_SINGLETASK,
	LIBRARY_VERSION,
	NT_LIBRARY,
	110,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

static struct ExpansionBase *exp_Init(struct ExpansionBase *ExpansionBase, UINT32 *segList, APTR SysBase)
{
	ExpansionBase->SysBase	= SysBase;
	ExpansionBase->DosBase	= NULL; // For later use

	NewListType(&ExpansionBase->BoardList, NT_PCILIST);
	InitSemaphore(&ExpansionBase->BoardListLock);

	NewListType(&ExpansionBase->MountList, NT_DOSLIST);
	InitSemaphore(&ExpansionBase->MountListLock);

	return ExpansionBase;
}

static const char EndResident = 0;

