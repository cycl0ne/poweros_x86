#include "coregfx.h"
#include "pixmap.h"

#define SysBase CoreGfxBase->SysBase

PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 bpp, APTR pixels, UINT32 palsize)
{
	if (width <= 0 || height <= 0) return NULL;

	PixMap *ret = AllocVec(sizeof(PixMap), MEMF_CLEAR);
	if (ret)
	{
		ret->flags = FPM_AllocVec|FPM_Memory;
		ret->xres = ret->xvirtres = width;
		ret->yres = ret->yvirtres = height;
		ret->bpp  = bpp;
		ret->pitch = width * (bpp>>3);
		ret->memsize = width * height * (bpp>>3);
		if (pixels)	
		{
			ret->addr = pixels;
		} else 
		{
			ret->flags |= FPM_AllocAddr;
			ret->addr = AllocVec(ret->memsize, MEMF_CLEAR);
		}
		if (!ret->addr) 
		{
			FreeVec(ret);
			ret = NULL;
		}
	}
	return ret;
}

void cgfx_FreePixMap(CoreGfxBase *CoreGfxBase, PixMap *pix)
{
	if (pix == NULL) return;
	if (pix->flags & FPM_AllocAddr)	FreeVec(pix->addr);
	if (pix->flags & FPM_AllocVec)	FreeVec(pix);
}

BOOL BltPixMap(CoreGfxBase *CoreGfxBase, PixMap *dstpix, UINT32 dstx, UINT32 dsty, UINT32 width, UINT32 height, PixMap *srcpix, UINT32 srcx, UINT32 srcy)
{
	if ((srcpix == NULL) || (dstpix == NULL)) return FALSE;
	if (srcpix->bpp != dstpix->bpp) return FALSE;
	
	if (srcx < 0) 
	{
		width += srcx;
		dstx -= srcx;
		srcx = 0;
	}
	if (srcy < 0) 
	{
		height += srcy;
		dsty -= srcy;
		srcy = 0;
	}
	if (srcx + width > srcpix->xvirtres) width = srcpix->xvirtres - srcx;
	if (srcy + height > srcpix->yvirtres) height = srcpix->yvirtres - srcy;
	
	
}
