#include "vgagfx.h"

#define SysBase VgaGfxBase->SysBase

UINT32 SVGA_ReadPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + (x << 2);
	return *((UINT32*)addr);
}

UINT32 SVGA_ReadPixel24(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y)
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
	return 0;
}

UINT16 SVGA_ReadPixel16(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + (x << 1);
	return *((UINT16*)addr);
}

UINT8 SVGA_ReadPixel8(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y)
{
	register unsigned char *addr = VgaGfxBase->fb + y * VgaGfxBase->pitch + x;
	return *addr;
}
