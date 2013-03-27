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

#define IS_PRIORITY		0

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
	NewList((struct List *)&VgaGfxBase->Unit.unit_MsgPort.mp_MsgList);
	VgaGfxBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)name;
	VgaGfxBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	VgaGfxBase->Unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling

	VgaGfxBase->ExpansionBase = (ExpansionBase *)OpenLibrary("expansion.library", 0);
	if (VgaGfxBase->ExpansionBase == NULL) Alert((1<<31), "[vgagfx] Cant open expansion.library\n");

	VgaGfxBase->IS = CreateIntServer("IRQ vgagfx.device", IS_PRIORITY, vgagfx_handler, VgaGfxBase);
	//AddIntServer(IRQ_MOUSE, MDBase->IS);
	return VgaGfxBase;
}

#if 0
void SVGA_Init(void)
{
   if (!PCI_FindDevice(PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA2,
                       &gSVGA.pciAddr)) {
      Console_Panic("No VMware SVGA device found.");
   }

   /*
    * Use the default base address for each memory region.
    * We must map at least ioBase before using ReadReg/WriteReg.
    */

   PCI_SetMemEnable(&gSVGA.pciAddr, TRUE);
   gSVGA.ioBase = PCI_GetBARAddr(&gSVGA.pciAddr, 0);
   gSVGA.fbMem = (void*) PCI_GetBARAddr(&gSVGA.pciAddr, 1);
   gSVGA.fifoMem = (void*) PCI_GetBARAddr(&gSVGA.pciAddr, 2);

   /*
    * Version negotiation:
    *
    *   1. Write to SVGA_REG_ID the maximum ID supported by this driver.
    *   2. Read from SVGA_REG_ID
    *      a. If we read back the same value, this ID is supported. We're done.
    *      b. If not, decrement the ID and repeat.
    */

   gSVGA.deviceVersionId = SVGA_ID_2;
   do {
      SVGA_WriteReg(SVGA_REG_ID, gSVGA.deviceVersionId);
      if (SVGA_ReadReg(SVGA_REG_ID) == gSVGA.deviceVersionId) {
         break;
      } else {
         gSVGA.deviceVersionId--;
      }
   } while (gSVGA.deviceVersionId >= SVGA_ID_0);

   if (gSVGA.deviceVersionId < SVGA_ID_0) {
      Console_Panic("Error negotiating SVGA device version.");
   }

   /*
    * We must determine the FIFO and FB size after version
    * negotiation, since the default version (SVGA_ID_0)
    * does not support the FIFO buffer at all.
    */

   gSVGA.vramSize = SVGA_ReadReg(SVGA_REG_VRAM_SIZE);
   gSVGA.fbSize = SVGA_ReadReg(SVGA_REG_FB_SIZE);
   gSVGA.fifoSize = SVGA_ReadReg(SVGA_REG_MEM_SIZE);

   /*
    * Sanity-check the FIFO and framebuffer sizes.
    * These are arbitrary values.
    */

   if (gSVGA.fbSize < 0x100000) {
      SVGA_Panic("FB size very small, probably incorrect.");
   }
   if (gSVGA.fifoSize < 0x20000) {
      SVGA_Panic("FIFO size very small, probably incorrect.");
   }

   /*
    * If the device is new enough to support capability flags, get the
    * capabilities register.
    */

   if (gSVGA.deviceVersionId >= SVGA_ID_1) {
      gSVGA.capabilities = SVGA_ReadReg(SVGA_REG_CAPABILITIES);
   }

   /*
    * Optional interrupt initialization.
    *
    * This uses the default IRQ that was assigned to our
    * device by the BIOS.
    */

#ifndef REALLY_TINY
   if (gSVGA.capabilities & SVGA_CAP_IRQMASK) {
      uint8 irq = PCI_ConfigRead8(&gSVGA.pciAddr, offsetof(PCIConfigSpace, intrLine));

      /* Start out with all SVGA IRQs masked */
      SVGA_WriteReg(SVGA_REG_IRQMASK, 0);

      /* Clear all pending IRQs stored by the device */
      IO_Out32(gSVGA.ioBase + SVGA_IRQSTATUS_PORT, 0xFF);

      /* Clear all pending IRQs stored by us */
      SVGA_ClearIRQ();

      /* Enable the IRQ */
      Intr_SetHandler(IRQ_VECTOR(irq), SVGAInterruptHandler);
      Intr_SetMask(irq, TRUE);
   }
#endif

   SVGA_Enable();
}
#endif

