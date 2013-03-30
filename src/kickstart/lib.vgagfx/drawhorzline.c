#include "vgagfx.h"

#define SysBase VgaGfxBase->SysBase
void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) ;

void SVGA_DrawHorzLine32(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 x2, UINT32 y, UINT32 c, UINT32 rop)
{
	register UINT8 *addr = VgaGfxBase->fb + y * VgaGfxBase->bytesPerRow + (x1 << 2);
//DPrintF("FbMem: %x, Pitch %d\n", VgaGfxBase->fbMem, VgaGfxBase->bytesPerRow);
	int width = x2-x1+1;
	
	if(rop == MWROP_COPY)
	{
		int w = width;
		while (--w >= 0)
		{
			*((UINT32*)addr) = c;
			addr += 4;		
		}
	} else {
		DPrintF("Not implemented\n");
		//APPLYOP(gr_mode, width, (UINT32), c, *(UINT32), addr, 0, 4);
	}
}

void SVGA_DrawHorzLine16(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 x2, UINT32 y, UINT16 c, UINT32 rop)
{
	register UINT8 *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + (x1 << 1);
	int width = x2-x1+1;
	
	if(rop == MWROP_COPY)
	{
		int w = width;
		while (--w >= 0)
		{
		*((UINT16*)addr) = c;
			addr += 2;
		}
	} else {
		//APPLYOP(gr_mode, width, (UINT32), c, *(UINT16), addr, 0, 2);
	}
}
