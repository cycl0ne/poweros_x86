#include "vgagfx.h"
#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "exec_funcs.h"
#include "blit.h"

extern APTR g_SysBase;
extern APTR g_VgaGfxBase;

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
}

UINT32 SVGA_ReadPixel32(struct CRastPort *rp, INT32 x,INT32 y)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	return *((UINT32*)addr);
}

void SVGA_DrawHorzLine32(struct CRastPort *rp, INT32 x1, INT32 x2, INT32 y, UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x1 << 2);
	int width = x2-x1+1;
	
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
}

void SVGA_DrawFillRect32(struct CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT32 c)
{
	int X1 = x1;
	int Y1 = y1;
		while(y1 <= y2)
			SVGA_DrawHorzLine32(rp, x1, x2, y1++, c);
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
			break;
		case 32:
			if (psd->data_format == IF_RGBA8888)	/* RGBA pixmaps*/
			{
				psd->_DrawPixel		= SVGA_DrawPixel32;
				psd->_ReadPixel		= SVGA_ReadPixel32;
				psd->_DrawHorzLine	= SVGA_DrawHorzLine32;
				psd->_DrawVertLine	= SVGA_DrawVertLine32;
				psd->_FillRect 		= SVGA_DrawFillRect32;
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
				psd->_FillRect 		= SVGA_DrawFillRect32;
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
}

