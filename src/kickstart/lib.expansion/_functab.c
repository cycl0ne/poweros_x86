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

static struct ExpansionBase *exp_Init(struct ExpansionBase *ExpansionBase, UINT32 *segList, APTR SysBase);
APTR exp_OpenLib(struct ExpansionBase *ExpansionBase);
APTR exp_CloseLib(struct ExpansionBase *ExpansionBase);
APTR exp_ExpungeLib(struct ExpansionBase *ExpansionBase);
APTR exp_ExtFuncLib(void);
BOOL exp_WritePCIConfig(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nOffset, INT32 nSize, UINT32 nValue);
void exp_EnablePCIMaster(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum);
struct PCINode *exp_FindPCIDevice(struct ExpansionBase *ExpansionBase, INT32 venid, INT32 devid);
UINT32 exp_GetPCIMemorySize(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nResource );
UINT32 exp_ReadPCIConfig(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nOffset, INT32 nSize);
void exp_SetPCILatency(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, UINT8 nLatency);
void exp_ScanPCIAll(struct ExpansionBase *ExpansionBase);
void exp_ScanPCIBus(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nBridgeFrom, INT32 nBusDev);


static volatile APTR FuncTab[] =
{
	(void(*)) exp_OpenLib,
	(void(*)) exp_CloseLib,
	(void(*)) exp_ExpungeLib,
	(void(*)) exp_ExtFuncLib,
	(void(*)) exp_AddDosNode,
	(void(*)) exp_MakeDosNode,

	(void(*)) exp_ScanPCIAll,
	(void(*)) exp_ScanPCIBus,
	(void(*)) exp_FindPCIDevice,
	(void(*)) exp_ReadPCIConfig,
	(void(*)) exp_WritePCIConfig,

	(void(*)) exp_GetPCIMemorySize,
	(void(*)) exp_SetPCILatency,
	(void(*)) exp_EnablePCIMaster,
	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(struct ExpansionBase),
	(APTR)FuncTab,
	(APTR)NULL,
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

#include "io_ports.h"
#define SysBase ExpansionBase->SysBase

static void CheckPCIInstructionSet(struct ExpansionBase *ExpansionBase)
{

	ExpansionBase->g_nPCIMethod = 0;

	// Do simple register checks to find the way how to access the pci bus

	// Check PCI method 2
	outb( 0x0, 0x0cf8 );
	outb( 0x0, 0x0cfa );
	if ( inb( 0x0cf8 ) == 0x0 && inb( 0x0cfa ) == 0x0 )
	{
		ExpansionBase->g_nPCIMethod = PCI_METHOD_2;
		DPrintF( "PCI: Using access method 2\n" );
		return;
	}

	// Check PCI method 1
	outl( 0x80000000, 0x0cf8 );
	if ( inl( 0x0cf8 ) == 0x80000000 )
	{
		outl( 0x0, 0x0cf8 );
		if ( inl( 0x0cf8 ) == 0x0 )
		{
			ExpansionBase->g_nPCIMethod = PCI_METHOD_1;
			DPrintF( "PCI: Using access method 1\n" );
			return;
		}
	}

	// Check for Virtual PC PCI method 1
	outl( 0x80000000, 0x0cf8 );
	if ( inl( 0x0cf8 ) == 0x80000000 )
	{
		outl( 0x0, 0x0cf8 );
		if ( inl( 0x0cf8 ) == 0x80000000 )
		{
			ExpansionBase->g_nPCIMethod = PCI_METHOD_1;
			DPrintF( "PCI: Detected Virtual PC, using access method 1\n" );
			return;
		}
	}

}

#undef SysBase

static struct ExpansionBase *exp_Init(struct ExpansionBase *ExpansionBase, UINT32 *segList, APTR SysBase)
{
	ExpansionBase->Library.lib_OpenCnt = 0;
	ExpansionBase->Library.lib_Node.ln_Pri = -100;
	ExpansionBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	ExpansionBase->Library.lib_Node.ln_Name = (STRPTR)name;
	ExpansionBase->Library.lib_Version = LIBRARY_VERSION;
	ExpansionBase->Library.lib_Revision = LIBRARY_REVISION;
	ExpansionBase->Library.lib_IDString = (STRPTR)&version[7];
	ExpansionBase->SysBase	= SysBase;
	ExpansionBase->DosBase	= NULL; // For later use

	NewListType(&ExpansionBase->BoardList, NT_PCILIST);
	InitSemaphore(&ExpansionBase->BoardListLock);

	NewListType(&ExpansionBase->MountList, NT_DOSLIST);
	InitSemaphore(&ExpansionBase->MountListLock);

	ExpansionBase->g_nPCINumBusses = 0;
	ExpansionBase->g_nPCINumDevices = 0;

	for (UINT32 i=0; i<MAX_PCI_BUSSES; i++) NewList(&ExpansionBase->pciBus[i].pci_Node);

	CheckPCIInstructionSet(ExpansionBase);

	if ( ExpansionBase->g_nPCIMethod == 0 )
	{
		DPrintF( "No PCI bus found\n" );
		return NULL;
	}
	else
	{
		ScanPCIAll();
	}

	return ExpansionBase;
}

static const char EndResident = 0;

