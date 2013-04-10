#include "vgagfx.h"
#include "vmware.h"

#define SysBase VgaGfxBase->SysBase
#define ExpansionBase	VgaGfxBase->ExpansionBase

static void PrintCapabilities(VgaGfxBase *VgaGfxBase, UINT32 c)
{
	DPrintF("capabilities:\n");
	if (c & SVGA_CAP_RECT_FILL)			DPrintF("RECT_FILL\n");
	if (c & SVGA_CAP_RECT_COPY)			DPrintF("RECT_COPY\n");
	if (c & SVGA_CAP_RECT_PAT_FILL)		DPrintF("RECT_PAT_FILL\n");
	if (c & SVGA_CAP_LEGACY_OFFSCREEN)	DPrintF("LEGACY_OFFSCREEN\n");
	if (c & SVGA_CAP_RASTER_OP)			DPrintF("RASTER_OP\n");
	if (c & SVGA_CAP_CURSOR)			DPrintF("CURSOR\n");
	if (c & SVGA_CAP_CURSOR_BYPASS)		DPrintF("CURSOR_BYPASS\n");
	if (c & SVGA_CAP_CURSOR_BYPASS_2)	DPrintF("CURSOR_BYPASS_2\n");
	if (c & SVGA_CAP_8BIT_EMULATION)	DPrintF("8BIT_EMULATION\n");
	if (c & SVGA_CAP_ALPHA_CURSOR)		DPrintF("ALPHA_CURSOR\n");
	if (c & SVGA_CAP_GLYPH)				DPrintF("GLYPH\n");
	if (c & SVGA_CAP_GLYPH_CLIPPING)	DPrintF("GLYPH_CLIPPING\n");
	if (c & SVGA_CAP_OFFSCREEN_1)		DPrintF("OFFSCREEN_1\n");
	if (c & SVGA_CAP_ALPHA_BLEND)		DPrintF("ALPHA_BLEND\n");
	if (c & SVGA_CAP_3D)				DPrintF("3D\n");
	if (c & SVGA_CAP_EXTENDED_FIFO)		DPrintF("EXTENDED_FIFO\n");
}

//	VgaGfxBase->ioBase 	= 			;
//	VgaGfxBase->fbMem		= (void*)	PCIGetBARAddr(&VgaGfxBase->pciAddr, 1);
//	VgaGfxBase->fifoMem	= (void*)	PCIGetBARAddr(&VgaGfxBase->pciAddr, 2);

UINT32 SVGA_CheckCapabilities(VgaGfxBase *VgaGfxBase)
{
	UINT32 id;

	/* Needed to read/write registers */
	PCISetMemEnable(&VgaGfxBase->pciAddr, TRUE);
	VgaGfxBase->indexPort = PCIGetBARAddr(&VgaGfxBase->pciAddr, 0) + SVGA_INDEX_PORT;
	VgaGfxBase->valuePort = PCIGetBARAddr(&VgaGfxBase->pciAddr, 0) + SVGA_VALUE_PORT;
//	DPrintF("index port: %x, value port: %x\n", VgaGfxBase->indexPort, VgaGfxBase->valuePort);

	/* This should be SVGA II according to the PCI device_id,
	 * but just in case... */
	WriteReg(VgaGfxBase, SVGA_REG_ID, SVGA_ID_2);
	if ((id = ReadReg(VgaGfxBase, SVGA_REG_ID)) != SVGA_ID_2) {
		DPrintF("SVGA_REG_ID is %ld, not %d\n", id, SVGA_REG_ID);
		return FALSE;
	}
//	DPrintF("SVGA_REG_ID OK\n");

	/* Grab some info */
	VgaGfxBase->maxWidth = ReadReg(VgaGfxBase, SVGA_REG_MAX_WIDTH);
	VgaGfxBase->maxHeight = ReadReg(VgaGfxBase, SVGA_REG_MAX_HEIGHT);
//	DPrintF("max resolution: %d x %d\n", VgaGfxBase->maxWidth, VgaGfxBase->maxHeight);
	VgaGfxBase->fbDma = (void *)ReadReg(VgaGfxBase, SVGA_REG_FB_START);
	VgaGfxBase->fb = (void *)ReadReg(VgaGfxBase, SVGA_REG_FB_START);
	VgaGfxBase->fbSize = ReadReg(VgaGfxBase, SVGA_REG_VRAM_SIZE);
//	DPrintF("frame buffer: %p, size %x\n", VgaGfxBase->fbDma, VgaGfxBase->fbSize);
	VgaGfxBase->fifoDma = (void *)ReadReg(VgaGfxBase, SVGA_REG_MEM_START);
	VgaGfxBase->fifo = (void *)ReadReg(VgaGfxBase, SVGA_REG_MEM_START);
	VgaGfxBase->fifoSize = ReadReg(VgaGfxBase, SVGA_REG_MEM_SIZE) & ~3;
//	DPrintF("fifo: %x, size %x\n", VgaGfxBase->fifoDma, VgaGfxBase->fifoSize);
	VgaGfxBase->capabilities = ReadReg(VgaGfxBase, SVGA_REG_CAPABILITIES);
//	PrintCapabilities(VgaGfxBase, VgaGfxBase->capabilities);
	VgaGfxBase->fifoMin = (VgaGfxBase->capabilities & SVGA_CAP_EXTENDED_FIFO) ? ReadReg(VgaGfxBase, SVGA_REG_MEM_REGS) : 4;
	return TRUE;
}

APTR SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp)
{
	WriteReg(VgaGfxBase, SVGA_REG_WIDTH, nWidth);
	WriteReg(VgaGfxBase, SVGA_REG_HEIGHT, nHeight);
	WriteReg(VgaGfxBase, SVGA_REG_BITS_PER_PIXEL, nBpp);
	VgaGfxBase->fbOffset = ReadReg(VgaGfxBase, SVGA_REG_FB_OFFSET);
	VgaGfxBase->bytesPerRow = ReadReg(VgaGfxBase, SVGA_REG_BYTES_PER_LINE);
	VgaGfxBase->width = nWidth;
	VgaGfxBase->height = nHeight;
	VgaGfxBase->bpp = ReadReg(VgaGfxBase, SVGA_REG_DEPTH);
	ReadReg(VgaGfxBase, SVGA_REG_PSEUDOCOLOR);
	ReadReg(VgaGfxBase, SVGA_REG_RED_MASK);
	ReadReg(VgaGfxBase, SVGA_REG_GREEN_MASK);
	ReadReg(VgaGfxBase, SVGA_REG_BLUE_MASK);
	return VgaGfxBase->fbDma;
}

APTR SVGA_SetDisplayMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp)
{
	SVGA_FifoStop(VgaGfxBase);
	SVGA_SetMode(VgaGfxBase, nWidth, nHeight, nBpp);
	SVGA_FifoInit(VgaGfxBase);

	memset(VgaGfxBase->fbDma, 0, nHeight * VgaGfxBase->bytesPerRow);
//	FifoUpdateFullscreen();
	SVGA_FifoStart(VgaGfxBase);
	return VgaGfxBase->fbDma;
}

void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) 
{
	SVGA_FifoBeginWrite(VgaGfxBase);
	SVGA_FifoWrite(VgaGfxBase, SVGA_CMD_RECT_FILL );
	SVGA_FifoWrite(VgaGfxBase, color );
	SVGA_FifoWrite(VgaGfxBase, x );
	SVGA_FifoWrite(VgaGfxBase, y );
	SVGA_FifoWrite(VgaGfxBase, width );
	SVGA_FifoWrite(VgaGfxBase, height );
	SVGA_FifoEndWrite(VgaGfxBase);
	SVGA_FifoSync(VgaGfxBase);
}
		
void SVGA_CopyRect(VgaGfxBase *VgaGfxBase, UINT32 srcX, UINT32 srcY, UINT32 dstX, UINT32 dstY, UINT32 width, UINT32 height ) 
{
	SVGA_FifoBeginWrite(VgaGfxBase);
	SVGA_FifoWrite(VgaGfxBase, SVGA_CMD_RECT_COPY );
	SVGA_FifoWrite(VgaGfxBase, srcX );
	SVGA_FifoWrite(VgaGfxBase, srcY );
	SVGA_FifoWrite(VgaGfxBase, dstX );
	SVGA_FifoWrite(VgaGfxBase, dstY );
	SVGA_FifoWrite(VgaGfxBase, width );
	SVGA_FifoWrite(VgaGfxBase, height );
	SVGA_FifoEndWrite(VgaGfxBase);
	SVGA_FifoSync(VgaGfxBase);
}
