#include "vgagfx.h"

void SVGA_DrawFillRect(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, UINT32 c, UINT32 rop)
{
	switch(VgaGfxBase->bpp)
	{
		case 32:
			SVGA_DrawFillRect32(VgaGfxBase, x1, y1, x2, y2, c, rop);
			break;
		case 24:
			//SVGA_DrawFillRect24(VgaGfxBase, x1, y1, x2, y2, c, rop);
			break;
		case 16:
			SVGA_DrawFillRect16(VgaGfxBase, x1, y1, x2, y2, c, rop);
			break;
		case 8:
		default:
		break;
	}
}

void SVGA_DrawFillRect32(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, UINT32 c, UINT32 rop)
{
	int X1 = x1;
	int Y1 = y1;
		while(y1 <= y2)
			SVGA_DrawHorzLine32(VgaGfxBase, x1, x2, y1++, c, rop);
}

void SVGA_DrawFillRect16(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, UINT16 c, UINT32 rop)
{
	int X1 = x1;
	int Y1 = y1;
		while(y1 <= y2)
			SVGA_DrawHorzLine16(VgaGfxBase, x1, x2, y1++, c, rop);
}
