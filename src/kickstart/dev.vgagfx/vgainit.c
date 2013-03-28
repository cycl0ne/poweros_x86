/*
**
**
**
*/

#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"

#include "vgagfx.h"

#include "sysbase.h"
#include "exec_funcs.h"

#include "pci.h"
#include "expansion_funcs.h"

#define DEVICE_VERSION_STRING "\0$VER: vgagfx.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static const char name[] = "vgagfx.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

APTR gfxdev_OpenDev(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR gfxdev_CloseDev(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq);
APTR gfxdev_ExpungeDev(VgaGfxBase *VgaGfxBase);
APTR gfxdev_ExtFuncDev(VgaGfxBase *VgaGfxBase);
void gfxdev_BeginIO(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq);
void gfxdev_AbortIO(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq);

static VgaGfxBase *gfxdev_Init(VgaGfxBase *VgaGfxBase, UINT32 *segList, struct SysBase *SysBase);
__attribute__((no_instrument_function)) BOOL vgagfx_handler(UINT32 number, VgaGfxBase *VgaGfxBase, APTR SysBase);


static APTR FuncTab[] = 
{
	(void(*)) gfxdev_OpenDev,
	(void(*)) gfxdev_CloseDev,
	(void(*)) gfxdev_ExpungeDev,
	(void(*)) gfxdev_ExtFuncDev,

	(void(*)) gfxdev_BeginIO,
	(void(*)) gfxdev_AbortIO,
	(APTR) ((UINT32)-1)
};

static const APTR InitTab[4]=
{
	(APTR)sizeof(VgaGfxBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)gfxdev_Init
};

static const struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	60,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

void SVGA_Init(VgaGfxBase *VgaGfxBase);
void SVGA_Enable(VgaGfxBase *VgaGfxBase);
void Screen_Init(VgaGfxBase *VgaGfxBase);

static VgaGfxBase *gfxdev_Init(VgaGfxBase *VgaGfxBase, UINT32 *segList, struct SysBase *SysBase)
{
	VgaGfxBase->Device.dd_Library.lib_OpenCnt = 0;
	VgaGfxBase->Device.dd_Library.lib_Node.ln_Pri = 0;
	VgaGfxBase->Device.dd_Library.lib_Node.ln_Type = NT_DEVICE;
	VgaGfxBase->Device.dd_Library.lib_Node.ln_Name = (STRPTR)name;
	VgaGfxBase->Device.dd_Library.lib_Version = DEVICE_VERSION;
	VgaGfxBase->Device.dd_Library.lib_Revision = DEVICE_REVISION;
	VgaGfxBase->Device.dd_Library.lib_IDString = (STRPTR)&version[7];
	
	VgaGfxBase->SysBase	= SysBase;
	
	// Initialise Unit Command Queue
	NewList((struct List *)&VgaGfxBase->Unit.unit.unit_MsgPort.mp_MsgList);
	VgaGfxBase->Unit.unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)name;
	VgaGfxBase->Unit.unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	VgaGfxBase->Unit.unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling

	struct ExpansionBase *ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
	VgaGfxBase->ExpansionBase = ExpansionBase;
	if (VgaGfxBase->ExpansionBase == NULL) {
		Alert((1<<31), "[vgagfx] Cant open expansion.library\n");
		return NULL;
	}

	if (!PCIFindDevice(PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA2, &VgaGfxBase->Unit.pciAddr)) {
		Alert((1<<31), "No VMware SVGA device found.");
	}
   
	SVGA_Init(VgaGfxBase);
	SVGA_Enable(VgaGfxBase);
	Screen_Init(VgaGfxBase);

	return VgaGfxBase;
}

