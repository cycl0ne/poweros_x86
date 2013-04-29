#include "vgagfx.h"
#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "exec_funcs.h"
#include "blit.h"
#include "vmware.h"

#define PIXEL888RED(pixelval)		(((pixelval) >> 16) & 0xff)
#define PIXEL888GREEN(pixelval)		(((pixelval) >> 8) & 0xff)
#define PIXEL888BLUE(pixelval)		((pixelval) & 0xff)

extern APTR g_SysBase;
extern APTR g_VgaGfxBase;

void SVGA_DrawPixel24(struct CRastPort *rp, INT32 x,INT32 y,UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + x * 3;
	UINT8 r = PIXEL888RED(c);
	UINT8 g = PIXEL888GREEN(c);
	UINT8 b = PIXEL888BLUE(c);

	if(rp->crp_Mode == ROP_COPY)
	{
		addr[0] = b;
		addr[1] = g;
		addr[2] = r;
	}
	else
	{
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode , 1, (UINT8), b, *(UINT8*), addr, 0, 1);
		APPLYOP(rp->crp_Mode , 1, (UINT8), g, *(UINT8*), addr, 0, 1);
		APPLYOP(rp->crp_Mode , 1, (UINT8), r, *(UINT8*), addr, 0, 1);
	}
//	if (psd->_Update) psd->_Update(psd, x, y, 1, 1);
}

UINT32 SVGA_ReadPixel24(struct CRastPort *rp, INT32 x,INT32 y)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + x * 3;
	return RGB2PIXEL888(addr[2], addr[1], addr[0]);
}

void SVGA_DrawHorzLine24(struct CRastPort *rp, INT32 x1, INT32 x2, INT32 y, UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + x1 * 3;
	UINT8 r = PIXEL888RED(c);
	UINT8 g = PIXEL888GREEN(c);
	UINT8 b = PIXEL888BLUE(c);
	int w = x2-x1+1;

	if(rp->crp_Mode == ROP_COPY)
	{
		while(--w >= 0)
		{
			*addr++ = b;
			*addr++ = g;
			*addr++ = r;
		}
	}
	else
	{
		UINT32 gr_background = rp->crp_Background;
		while(--w >= 0)
		{
			APPLYOP(rp->crp_Mode, 1, (UINT8), b, *(UINT8*), addr, 0, 1);
			APPLYOP(rp->crp_Mode, 1, (UINT8), g, *(UINT8*), addr, 0, 1);
			APPLYOP(rp->crp_Mode, 1, (UINT8), r, *(UINT8*), addr, 0, 1);
		}
	}

	//if (psd->_Update) psd->_Update(psd, x1, y, x2-x1+1, 1);
}

void SVGA_DrawVertLine24(struct CRastPort *rp, INT32 x, INT32 y1, INT32 y2, UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	int	pitch = psd->pitch;
	register unsigned char *addr = psd->addr + y1 * pitch + x * 3;
	UINT8 r = PIXEL888RED(c);
	UINT8 g = PIXEL888GREEN(c);
	UINT8 b = PIXEL888BLUE(c);
	int height = y2-y1+1;

	if(rp->crp_Mode == ROP_COPY)
	{
		while (--height >= 0)
		{
			addr[0] = b;
			addr[1] = g;
			addr[2] = r;
			addr += pitch;
		}
	}
	else
	{
		UINT32 gr_background = rp->crp_Background;
		while (--height >= 0)
		{
			APPLYOP(rp->crp_Mode, 1, (UINT8), b, *(UINT8*), addr, 0, 1);
			APPLYOP(rp->crp_Mode, 1, (UINT8), g, *(UINT8*), addr, 0, 1);
			APPLYOP(rp->crp_Mode, 1, (UINT8), r, *(UINT8*), addr, 0, 1);
			addr += pitch - 3;
		}
	}

	//if (psd->_Update) psd->_Update(psd, x, y1, 1, y2-y1+1);
}

void SVGA_DrawPixel32(struct CRastPort *rp, INT32 x,INT32 y,UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	if (rp->crp_Mode == ROP_COPY)
		*((UINT32*)addr) = c;
	else
	{
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, 1, (UINT32), c, *(UINT32*), addr, 0, 0);
	}
	if (psd->_Update) 
	{
		//APTR SysBase = g_SysBase;
		//DPrintF("DP32: Update\n");
		psd->_Update(psd, x, y, 1, 1);
	}
}

UINT32 SVGA_ReadPixel32(struct CRastPort *rp, INT32 x,INT32 y)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	return *((UINT32*)addr);
}

#define SysBase g_SysBase
void SVGA_DrawHorzLine32(struct CRastPort *rp, INT32 x1, INT32 x2, INT32 y, UINT32 c)
{
//	DPrintF("DrawHorzLine\n");
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x1 << 2);
	int width = x2-x1+1;
//	DPrintF("DrawHorzLine %x, %x, %x\n", addr, rp, psd);
	
	if(rp->crp_Mode == ROP_COPY)
	{
		int w = width;
		while (--w >= 0)
		{
			*((UINT32*)addr) = c;
			addr += 4;		
		}
	} else {
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, width, (UINT32), c, *(UINT32*), addr, 0, 4);
	}
	if (psd->_Update) psd->_Update(psd, x1, y, x2-x1+1, 1);
}

void SVGA_DrawVertLine32(struct CRastPort *rp, INT32 x, INT32 y1, INT32 y2, UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
//	if (psd == NULL) return;
	int	pitch = psd->pitch;
	register unsigned char *addr = psd->addr + y1 * pitch + (x << 2);
	int height = y2-y1+1;

	if(rp->crp_Mode == ROP_COPY)
	{
		int h = height;
		while (--h >= 0)
		{
			*((UINT32*)addr) = c;
			addr += pitch;
		}
	} else {
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, height, (UINT32), c, *(UINT32*), addr, 0, pitch)
	}
	if (psd->_Update) psd->_Update(psd, x, y1, 1, y2-y1+1);
}

void SVGA_DrawFillRect(struct CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT32 c)
{
//	DPrintF("DrawFillRect\n");
	int X1 = x1;
	int Y1 = y1;
		while(y1 <= y2)
		{
//			DPrintF("y1:%d ", y1);
			rp->crp_PixMap->_DrawHorzLine(rp, x1, x2, y1++, c);
		}
	if (rp->crp_PixMap->_Update) {
		rp->crp_PixMap->_Update(rp->crp_PixMap, X1, Y1, x2-X1+1, y2-Y1+1);
	}
}

void SVGA_Update(struct PixMap *pix, INT32 x, INT32 y, INT32 width, INT32 height)
{
	if (pix->flags & PSF_SCREEN)
	{
		APTR VgaGfxBase = pix->basedata;
		SVGA_FifoBeginWrite(VgaGfxBase);
		SVGA_FifoWrite(VgaGfxBase, SVGA_CMD_UPDATE);
		SVGA_FifoWrite(VgaGfxBase, x);
		SVGA_FifoWrite(VgaGfxBase, y);
		SVGA_FifoWrite(VgaGfxBase, width); //VgaGfxBase->dm.virtual_width);
		SVGA_FifoWrite(VgaGfxBase, height); //VgaGfxBase->dm.virtual_height);
		SVGA_FifoEndWrite(VgaGfxBase);
		SVGA_FifoSync(VgaGfxBase);
	}
}

BOOL SVGA_SelectSubdriver(PixMap *psd)
{
//	APTR SysBase = g_SysBase;
//	DPrintF("planes %d\n", psd->planes);
	if(psd->planes == 1) 
	{
		switch(psd->bpp) 
		{
		case 1:
			break;
		case 2:
			break;
		case 4:
			break;
		case 8:
			break;
		case 16:
			break;
		case 24:
			psd->_DrawPixel		= SVGA_DrawPixel24;
			psd->_ReadPixel		= SVGA_ReadPixel24;
			psd->_DrawHorzLine	= SVGA_DrawHorzLine24;
			psd->_DrawVertLine	= SVGA_DrawVertLine24;
			psd->_FillRect 		= SVGA_DrawFillRect;
			psd->_Update		= SVGA_Update;
			psd->BlitFallback	= NULL;
			psd->FrameBlit			= frameblit_24bpp;
			psd->FrameStretchBlit	= frameblit_stretch_24bpp;
			psd->BlitCopyMaskMonoByteMSB = convblit_copy_mask_mono_byte_msb_bgr;
			psd->BlitCopyMaskMonoByteLSB = convblit_copy_mask_mono_byte_lsb_bgr;
			psd->BlitCopyMaskMonoWordMSB = convblit_copy_mask_mono_word_msb_bgr;
			psd->BlitBlendMaskAlphaByte  = convblit_blend_mask_alpha_byte_bgr;
			psd->BlitCopyRGBA8888 = convblit_copy_rgba8888_bgr888;
			psd->BlitSrcOverRGBA8888 = convblit_srcover_rgba8888_bgr888;
			psd->BlitCopyRGB888 = convblit_copy_rgb888_bgr888;
			psd->BlitStretchRGBA8888 = frameblit_stretch_rgba8888_bgr888;
			break;
		case 32:
			if (psd->data_format == IF_RGBA8888)	/* RGBA pixmaps*/
			{
				psd->_DrawPixel		= SVGA_DrawPixel32;
				psd->_ReadPixel		= SVGA_ReadPixel32;
				psd->_DrawHorzLine	= SVGA_DrawHorzLine32;
				psd->_DrawVertLine	= SVGA_DrawVertLine32;
				psd->_FillRect 		= SVGA_DrawFillRect;
				psd->_Update		= SVGA_Update;
				psd->BlitFallback	= NULL;
				psd->FrameBlit			= frameblit_xxxa8888;
				psd->FrameStretchBlit	= frameblit_stretch_xxxa8888;
				psd->BlitCopyMaskMonoByteMSB = convblit_copy_mask_mono_byte_msb_bgra;
				psd->BlitCopyMaskMonoByteLSB = convblit_copy_mask_mono_byte_lsb_bgra;
				psd->BlitCopyMaskMonoWordMSB = convblit_copy_mask_mono_word_msb_bgra;
				psd->BlitBlendMaskAlphaByte  = convblit_blend_mask_alpha_byte_bgra;
				psd->BlitCopyRGBA8888 = convblit_copy_rgba8888_bgra8888;
				psd->BlitSrcOverRGBA8888 = convblit_srcover_rgba8888_bgra8888;
				psd->BlitCopyRGB888 = convblit_copy_rgb888_bgra8888;
				psd->BlitStretchRGBA8888 = frameblit_stretch_rgba8888_bgra8888;
			}
			else
			{
				psd->_DrawPixel		= SVGA_DrawPixel32;
				psd->_ReadPixel		= SVGA_ReadPixel32;
				psd->_DrawHorzLine	= SVGA_DrawHorzLine32;
				psd->_DrawVertLine	= SVGA_DrawVertLine32;
				psd->_FillRect 		= SVGA_DrawFillRect;
				psd->_Update		= SVGA_Update;
				psd->BlitFallback	= NULL;
				psd->FrameBlit			= frameblit_xxxa8888;
				psd->FrameStretchBlit	= frameblit_stretch_xxxa8888;
				psd->BlitCopyMaskMonoByteMSB = convblit_copy_mask_mono_byte_msb_bgra;
				psd->BlitCopyMaskMonoByteLSB = convblit_copy_mask_mono_byte_lsb_bgra;
				psd->BlitCopyMaskMonoWordMSB = convblit_copy_mask_mono_word_msb_bgra;
				psd->BlitBlendMaskAlphaByte  = convblit_blend_mask_alpha_byte_bgra;
				psd->BlitCopyRGBA8888 = convblit_copy_rgba8888_bgra8888;
				psd->BlitSrcOverRGBA8888 = convblit_srcover_rgba8888_bgra8888;
				psd->BlitCopyRGB888 = convblit_copy_rgb888_bgra8888;
				psd->BlitStretchRGBA8888 = frameblit_stretch_rgba8888_bgra8888;
			}
			break;
		}
	}
	return TRUE;
}

