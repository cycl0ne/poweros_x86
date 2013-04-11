#include "vgagfx.h"
#include "vmware.h"

#define LIBRARY_VERSION_STRING "\0$VER: vgagfx.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "vgagfx.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static VgaGfxBase *vglib_Init(VgaGfxBase *VgaGfxBase, UINT32 *segList, APTR SysBase);
APTR vglib_OpenLib(VgaGfxBase *VgaGfxBase);
APTR vglib_CloseLib(VgaGfxBase *VgaGfxBase);
APTR vglib_ExpungeLib(VgaGfxBase *VgaGfxBase);
APTR vglib_ExtFuncLib(VgaGfxBase *VgaGfxBase);

void SVGA_Init(VgaGfxBase *VgaGfxBase);

static volatile APTR FuncTab[] =
{
	(void(*)) vglib_OpenLib,
	(void(*)) vglib_CloseLib,
	(void(*)) vglib_ExpungeLib,
	(void(*)) vglib_ExtFuncLib,

	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(VgaGfxBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)vglib_Init
};

static const volatile struct Resident ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK | RTF_AUTOINIT,
	LIBRARY_VERSION,
	NT_LIBRARY,
	100,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

APTR g_VgaGfxBase = 0;

static VgaGfxBase *vglib_Init(VgaGfxBase *VgaGfxBase, UINT32 *segList, APTR SysBase)
{
	VgaGfxBase->Library.lib_OpenCnt = 0;
	VgaGfxBase->Library.lib_Node.ln_Pri = -100;
	VgaGfxBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	VgaGfxBase->Library.lib_Node.ln_Name = (STRPTR)name;
	VgaGfxBase->Library.lib_Version = LIBRARY_VERSION;
	VgaGfxBase->Library.lib_Revision = LIBRARY_REVISION;
	VgaGfxBase->Library.lib_IDString = (STRPTR)&version[7];
	VgaGfxBase->SysBase	= SysBase;

	struct ExpansionBase *ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
	VgaGfxBase->ExpansionBase = ExpansionBase;
	if (VgaGfxBase->ExpansionBase == NULL) {
		Alert((1<<31), "[vgagfx] Cant open expansion.library\n");
		return NULL;
	}

	if (!PCIFindDevice(PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA2, &VgaGfxBase->pciAddr)) {
		return NULL;
		Alert((1<<31), "No VMware SVGA device found.");
	}

	SVGA_CheckCapabilities(VgaGfxBase);
	g_VgaGfxBase = VgaGfxBase;
	return VgaGfxBase;
}

static const char EndResident = 0;
