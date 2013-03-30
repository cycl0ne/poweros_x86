#include "vgagfx.h"


void SVGA_DrawPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + (x << 2);
	*((UINT32*)addr) = c;
}

void SVGA_DrawPixel24(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + x * 3;
#if 0
	UINT8 r = PIXEL888RED(c);
	UINT8 g = PIXEL888GREEN(c);
	UINT8 b = PIXEL888BLUE(c);
	addr[0] = b;
	addr[1] = g;
	addr[2] = r;
#endif
}

void SVGA_DrawPixel16(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT16 c, UINT32 rop)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + (x << 1);
	*((UINT16*)addr) = c;
}

void SVGA_DrawPixel8(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT8 c, UINT32 rop)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + x;
	*addr = c;
}


