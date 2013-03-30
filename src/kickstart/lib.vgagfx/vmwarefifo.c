#include "vgagfx.h"
#include "vmware.h"

#define SysBase VgaGfxBase->SysBase

static void SVGA_FifoPrintCapabilities(VgaGfxBase *VgaGfxBase, int c)
{
	DPrintF("fifo capabilities:\n");
	if (c & SVGA_FIFO_CAP_FENCE)		DPrintF("FENCE\n");
	if (c & SVGA_FIFO_CAP_ACCELFRONT)	DPrintF("ACCELFRONT\n");
	if (c & SVGA_FIFO_CAP_PITCHLOCK)	DPrintF("PITCHLOCK\n");
}

void SVGA_FifoInit(VgaGfxBase *VgaGfxBase)
{
	UINT32 *fifo = VgaGfxBase->fifo;

	VgaGfxBase->fifoCapabilities = 0;
	VgaGfxBase->fifoFlags = 0;
	if (VgaGfxBase->capabilities & SVGA_CAP_EXTENDED_FIFO) {
		VgaGfxBase->fifoCapabilities = fifo[SVGA_FIFO_CAPABILITIES];
		VgaGfxBase->fifoFlags = fifo[SVGA_FIFO_FLAGS];
		SVGA_FifoPrintCapabilities(VgaGfxBase, VgaGfxBase->fifoCapabilities);
	}

	fifo[SVGA_FIFO_MIN]		 = VgaGfxBase->fifoMin * 4;
	fifo[SVGA_FIFO_MAX]		 = VgaGfxBase->fifoSize;
	fifo[SVGA_FIFO_NEXT_CMD] = fifo[SVGA_FIFO_MIN];
	fifo[SVGA_FIFO_STOP]	 = fifo[SVGA_FIFO_MIN];

	VgaGfxBase->fifoNext = fifo[SVGA_FIFO_NEXT_CMD];

	DPrintF("init fifo: %ld -> %ld\n", fifo[SVGA_FIFO_MIN], fifo[SVGA_FIFO_MAX]);
}

void SVGA_FifoStart(VgaGfxBase *VgaGfxBase)
{
	WriteReg(VgaGfxBase, SVGA_REG_ENABLE, 1);
	WriteReg(VgaGfxBase, SVGA_REG_CONFIG_DONE, 1);
}

void SVGA_FifoStop(VgaGfxBase *VgaGfxBase)
{
	WriteReg(VgaGfxBase, SVGA_REG_CONFIG_DONE, 0);
	WriteReg(VgaGfxBase, SVGA_REG_ENABLE, 0);
}
			
void SVGA_FifoSync(VgaGfxBase *VgaGfxBase)
{
	asm volatile("wbinvd");
	WriteReg(VgaGfxBase, SVGA_REG_SYNC, 1);
	while (ReadReg(VgaGfxBase, SVGA_REG_BUSY));
	
}

void SVGA_FifoBeginWrite(VgaGfxBase *VgaGfxBase)
{
	// Get Fifo Semaphore
	//ACQUIRE_BEN(gSi->fifoLock);
}

void SVGA_FifoWrite(VgaGfxBase *VgaGfxBase, UINT32 value)
{
	register UINT32 *fifo = VgaGfxBase->fifo;
	register UINT32 fifoCapacity = fifo[SVGA_FIFO_MAX] - fifo[SVGA_FIFO_MIN];

	/* If the fifo is full, sync it */
	if (fifo[SVGA_FIFO_STOP] == fifo[SVGA_FIFO_NEXT_CMD] + 4 ||
		fifo[SVGA_FIFO_STOP] + fifoCapacity == fifo[SVGA_FIFO_NEXT_CMD] + 4)
		SVGA_FifoSync(VgaGfxBase);

	fifo[VgaGfxBase->fifoNext / 4] = value;
	VgaGfxBase->fifoNext = fifo[SVGA_FIFO_MIN] +
		(VgaGfxBase->fifoNext + 4 - fifo[SVGA_FIFO_MIN]) % fifoCapacity;
}

void SVGA_FifoEndWrite(VgaGfxBase *VgaGfxBase)
{
	register UINT32 *fifo = VgaGfxBase->fifo;
	fifo[SVGA_FIFO_NEXT_CMD] = VgaGfxBase->fifoNext;
	//RELEASE_BEN(gSi->fifoLock);
}

void SVGA_FifoUpdateFullscreen(VgaGfxBase *VgaGfxBase)
{
	SVGA_FifoBeginWrite(VgaGfxBase);
	SVGA_FifoWrite(VgaGfxBase, SVGA_CMD_UPDATE);
	SVGA_FifoWrite(VgaGfxBase, 0);
	SVGA_FifoWrite(VgaGfxBase, 0);
	SVGA_FifoWrite(VgaGfxBase, VgaGfxBase->width); //VgaGfxBase->dm.virtual_width);
	SVGA_FifoWrite(VgaGfxBase, VgaGfxBase->height); //VgaGfxBase->dm.virtual_height);
	SVGA_FifoEndWrite(VgaGfxBase);
	SVGA_FifoSync(VgaGfxBase);
}
