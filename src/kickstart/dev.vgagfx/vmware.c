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
#include "vmware.h"
#include "svga3d_reg.h"

__attribute__((no_instrument_function)) BOOL vgagfx_handler(UINT32 number, VgaGfxBase *VgaGfxBase, APTR SysBase);

#define SysBase			VgaGfxBase->SysBase
#define ExpansionBase	VgaGfxBase->ExpansionBase

UINT32 SVGA_ReadReg(VgaGfxBase *VgaGfxBase, UINT32 index)
{
   WRITE_PORT_ULONG(VgaGfxBase->Unit.ioBase + SVGA_INDEX_PORT, index);
   return READ_PORT_ULONG(VgaGfxBase->Unit.ioBase + SVGA_VALUE_PORT);
}

void SVGA_WriteReg(VgaGfxBase *VgaGfxBase, UINT32 index, UINT32 value)
{
   WRITE_PORT_ULONG(VgaGfxBase->Unit.ioBase + SVGA_INDEX_PORT, index);
   WRITE_PORT_ULONG(VgaGfxBase->Unit.ioBase + SVGA_VALUE_PORT, value);
}

/*
** 
*/

BOOL SVGA_HasFIFOCap(VgaGfxBase *VgaGfxBase, UINT32 cap)
{
	return (VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CAPABILITIES] & cap) != 0;
}

BOOL SVGA_IsFIFORegValid(VgaGfxBase *VgaGfxBase, UINT32 reg)
{
	return VgaGfxBase->Unit.fifoMem[SVGA_FIFO_MIN] > (reg << 2);
}

void Screen_Init(VgaGfxBase *VgaGfxBase)
{
	if (!SVGA_HasFIFOCap(VgaGfxBase, SVGA_FIFO_CAP_CURSOR_BYPASS_3)) 		Alert((1<<31), "NO SVGA_FIFO_CAP_CURSOR_BYPASS_3.");
	   
	if (!(	SVGA_HasFIFOCap(VgaGfxBase, SVGA_FIFO_CAP_SCREEN_OBJECT) ||
			SVGA_HasFIFOCap(VgaGfxBase, SVGA_FIFO_CAP_SCREEN_OBJECT_2))) {
		Alert((1<<31), "Virtual device does not have Screen Object support.");
	}
}

void SVGA_MoveCursor(VgaGfxBase *VgaGfxBase, UINT32 visible, UINT32 x, UINT32 y, UINT32 screenId)
{
   if (SVGA_HasFIFOCap(VgaGfxBase, SVGA_FIFO_CAP_SCREEN_OBJECT)) {
      VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CURSOR_SCREEN_ID] = screenId;
   }

   if (SVGA_HasFIFOCap(VgaGfxBase, SVGA_FIFO_CAP_CURSOR_BYPASS_3)) {
      VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CURSOR_ON] = visible;
      VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CURSOR_X] = x;
      VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CURSOR_Y] = y;
      VgaGfxBase->Unit.fifoMem[SVGA_FIFO_CURSOR_COUNT]++;
   }
}


void SVGA_Init(VgaGfxBase *VgaGfxBase)
{
	PCISetMemEnable(&VgaGfxBase->Unit.pciAddr, TRUE);
	VgaGfxBase->Unit.ioBase 	= 			PCIGetBARAddr(&VgaGfxBase->Unit.pciAddr, 0);
	VgaGfxBase->Unit.fbMem		= (void*)	PCIGetBARAddr(&VgaGfxBase->Unit.pciAddr, 1);
	VgaGfxBase->Unit.fifoMem	= (void*)	PCIGetBARAddr(&VgaGfxBase->Unit.pciAddr, 2);

	//DPrintF("SVGA_REG_ID %d\n", SVGA_ReadReg(VgaGfxBase, SVGA_REG_ID));
	VgaGfxBase->Unit.deviceVersionId = SVGA_ID_2;
	do {
		SVGA_WriteReg(VgaGfxBase, SVGA_REG_ID, VgaGfxBase->Unit.deviceVersionId);
		if (SVGA_ReadReg(VgaGfxBase, SVGA_REG_ID) == VgaGfxBase->Unit.deviceVersionId) {
			break;
		} else {
			VgaGfxBase->Unit.deviceVersionId--;
		}
	} while (VgaGfxBase->Unit.deviceVersionId >= SVGA_ID_0);

	if (VgaGfxBase->Unit.deviceVersionId < SVGA_ID_0) {
		Alert((1<<31), "Error negotiating SVGA device version.");
	}

	VgaGfxBase->Unit.vramSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_VRAM_SIZE);
	VgaGfxBase->Unit.fbSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_FB_SIZE);
	VgaGfxBase->Unit.fifoSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_MEM_SIZE);

	/*
	* Sanity-check the FIFO and framebuffer sizes.
	* These are arbitrary values.
	* 
	* QEMU has problems with them :( Tested V1.2
	*/

	//if (VgaGfxBase->Unit.fbSize < 0x100000) {
	//	Alert((1<<31), "FB size very small, probably incorrect.[%x]", VgaGfxBase->Unit.fbSize);
	//}
	//if (VgaGfxBase->Unit.fifoSize < 0x20000) {
	//	Alert((1<<31), "FIFO size very small, probably incorrect. [%x]", VgaGfxBase->Unit.fifoSize);
	//}

	if (VgaGfxBase->Unit.deviceVersionId >= SVGA_ID_1) {
		VgaGfxBase->Unit.capabilities = SVGA_ReadReg(VgaGfxBase, SVGA_REG_CAPABILITIES);
	}
	
	if (VgaGfxBase->Unit.capabilities & SVGA_CAP_IRQMASK) {
		UINT8 irq = PCIConfigRead8(&VgaGfxBase->Unit.pciAddr, offsetof(PCIConfigSpace, intrLine));
		/* Start out with all SVGA IRQs masked */
		SVGA_WriteReg(VgaGfxBase, SVGA_REG_IRQMASK, 0);
		/* Clear all pending IRQs stored by the device */
		WRITE_PORT_ULONG(VgaGfxBase->Unit.ioBase + SVGA_IRQSTATUS_PORT, 0xFF);
		/* Clear all pending IRQs stored by us */
		//SVGA_ClearIRQ();
		VgaGfxBase->IS = CreateIntServer("IRQ vgagfx.device", IS_PRIORITY, vgagfx_handler, VgaGfxBase);
		AddIntServer(irq, VgaGfxBase->IS);
	}	
}

void SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 width, UINT32 height, UINT32 bpp)
{
   VgaGfxBase->Unit.width = width;
   VgaGfxBase->Unit.height = height;
   VgaGfxBase->Unit.bpp = bpp;

   SVGA_WriteReg(VgaGfxBase, SVGA_REG_WIDTH, width);
   SVGA_WriteReg(VgaGfxBase, SVGA_REG_HEIGHT, height);
   SVGA_WriteReg(VgaGfxBase, SVGA_REG_BITS_PER_PIXEL, bpp);
   SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE);
   VgaGfxBase->Unit.pitch = SVGA_ReadReg(VgaGfxBase, SVGA_REG_BYTES_PER_LINE);
}

void SVGA_Disable(VgaGfxBase *VgaGfxBase)
{
   SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, FALSE);
}

void SVGA_Enable(VgaGfxBase *VgaGfxBase)
{
   VgaGfxBase->Unit.fifoMem[SVGA_FIFO_MIN]		= SVGA_FIFO_NUM_REGS * sizeof(UINT32);
   VgaGfxBase->Unit.fifoMem[SVGA_FIFO_MAX]		= VgaGfxBase->Unit.fifoSize;
   VgaGfxBase->Unit.fifoMem[SVGA_FIFO_NEXT_CMD]	= VgaGfxBase->Unit.fifoMem[SVGA_FIFO_MIN];
   VgaGfxBase->Unit.fifoMem[SVGA_FIFO_STOP] 	= VgaGfxBase->Unit.fifoMem[SVGA_FIFO_MIN];

	if (SVGA_HasFIFOCap(VgaGfxBase, SVGA_CAP_EXTENDED_FIFO) && SVGA_IsFIFORegValid(VgaGfxBase, SVGA_FIFO_GUEST_3D_HWVERSION)) {
		VgaGfxBase->Unit.fifoMem[SVGA_FIFO_GUEST_3D_HWVERSION] = SVGA3D_HWVERSION_CURRENT;
	}
	//SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE);
	//SVGA_WriteReg(VgaGfxBase, SVGA_REG_CONFIG_DONE, TRUE);
}

