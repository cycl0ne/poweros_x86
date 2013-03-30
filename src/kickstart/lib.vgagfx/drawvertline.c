#include "vgagfx.h"

void SVGA_DrawVertLine32(VgaGfxBase *VgaGfxBase, UINT32 y1, UINT32 y2, UINT32 x, UINT32 c, UINT32 rop)
{
	register UINT8 *addr = VgaGfxBase->fb + y1 * VgaGfxBase->pitch + (x << 2);
	int height = y2-y1+1;
	
	if(rop == MWROP_COPY)
	{
		int h = height;
		while (--h >= 0)
		{
			*((UINT32*)addr) = c;
			addr += VgaGfxBase->pitch;
		}
	} else {
		//APPLYOP(gr_mode, width, (UINT32), c, *(UINT32), addr, 0, 4);
	}
}

void SVGA_DrawVertLine16(VgaGfxBase *VgaGfxBase, UINT32 y1, UINT32 y2, UINT32 x, UINT16 c, UINT32 rop)
{
	register UINT8 *addr = VgaGfxBase->fb + y1 * VgaGfxBase->pitch + (x << 1);
	int height = y2-y1+1;
	
	if(rop == MWROP_COPY)
	{
		int h = height;
		while (--h >= 0)
		{
			*((UINT16*)addr) = c;
			addr += VgaGfxBase->pitch;
		}
	} else {
		//APPLYOP(gr_mode, width, (UINT32), c, *(UINT16), addr, 0, 2);
	}
}
