/**
* File: /gfx_color．c
* User: cycl0ne
* Date: 2014-11-27
* Time: 10:42 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#define SysBase GfxBase->SysBase

UINT32 gfx_SetMode(pGfxBase GfxBase, pPB rp, UINT32 mode)
{
	UINT32 oldmode = rp->pb_Mode;
	rp->pb_Mode = mode;
	rp->pb_PixMap->mode = mode;
	return oldmode;
}

UINT32 gfx_SetFillMode(pGfxBase GfxBase, pPB rp, UINT32 mode)
{
	UINT32 oldmode = rp->pb_FillMode;
	rp->pb_FillMode = mode;
	return oldmode;
}

BOOL gfx_SetUseBackground(pGfxBase GfxBase, pPB rp, BOOL flag)
{
	BOOL oldusebg = rp->pb_useBg;
	rp->pb_useBg = flag;
	return oldusebg;
}

UINT32 gfx_SetForegroundPixelVal(pGfxBase GfxBase, pPB rp, PIXELVAL fg)
{
	UINT32 oldfg = rp->pb_Foreground;
	rp->pb_Foreground = fg;
	return oldfg;
}

UINT32 gfx_SetBackgroundPixelVal(pGfxBase GfxBase, pPB rp, PIXELVAL bg)
{
	UINT32 oldbg = rp->pb_Background;
	rp->pb_Background = bg;
	rp->pb_PixMap->background = bg;
	return oldbg;
}

UINT32 gfx_SetForegroundColor(pGfxBase GfxBase, pPB rp, COLORVAL fg)
{
	UINT32 oldfg = rp->pb_Foreground;

	rp->pb_Foreground = FindColor(rp, fg);
	rp->pb_ForegroundRGB = fg;
//	DPrintF("Findcolor: Old: %x, Found: %x\n", fg, rp->pb_Foreground);
	return oldfg;
}

UINT32 gfx_SetBackgroundColor(pGfxBase GfxBase, pPB rp, COLORVAL bg)
{
	UINT32 oldbg = rp->pb_Background;

	rp->pb_Background = FindColor(rp, bg);
	rp->pb_BackgroundRGB = bg;
	rp->pb_PixMap->background = bg;
	return oldbg;
}

void gfx_SetDash(pGfxBase GfxBase, pPB rp, UINT32 *mask, UINT32 *count)
{
	UINT32 oldm = rp->pb_DashMask;
	UINT32 oldc = rp->pb_DashCount;

	if (!mask || !count) return;

	rp->pb_DashMask = *mask;
	rp->pb_DashCount = *count;

	*mask = oldm;
	*count = oldc;
}

#define COLOR2PIXEL8888(c)		((((c) & 0xff) << 16) | ((c) & 0xff00ff00ul) | (((c) & 0xff0000) >> 16))
#define PIXELABGR2PIXEL8888(c)	COLOR2PIXEL8888(c)
#define COLOR2PIXELABGR(c)		(c)
#define COLOR2PIXEL888(c)		((((c) & 0xff) << 16) | ((c) & 0xff00) | (((c) & 0xff0000) >> 16))
#define COLOR2PIXEL565(c)		((((c) & 0xf8) << 8) | (((c) & 0xfc00) >> 5) | (((c) & 0xf80000) >> 19))
#define COLOR2PIXEL555(c)		((((c) & 0xf8) << 7) | (((c) & 0xf800) >> 6) | (((c) & 0xf80000) >> 19))
#define COLOR2PIXEL1555(c)		((((c) & 0xf8) >> 3) | (((c) & 0xf800) >> 6) | (((c) & 0xf80000) >> 9) | 0x8000)
#define COLOR2PIXEL332(c)		(((c) & 0xe0) | (((c) & 0xe000) >> 11) | (((c) & 0xc00000) >> 22))
#define COLOR2PIXEL233(c)		((((c) & 0xC00000) >> 16) | (((c) & 0x00E000) >> 10) | (((c) & 0xE0) >> 5))

UINT32 gfx_FindColor(pGfxBase GfxBase, pPB rp, UINT32 c)
{
	PixMap_t *psd = rp->pb_PixMap;
//Printf("psd->PixType: %x\n", psd->pixtype);
	switch(psd->pixtype)
	{
		case PF_TRUECOLOR8888:
			return COLOR2PIXEL8888(c);
		case PF_TRUECOLORABGR:
			return COLOR2PIXELABGR(c);
		case PF_TRUECOLOR888:
			return COLOR2PIXEL888(c);
		case PF_TRUECOLOR565:
			return COLOR2PIXEL565(c);
		case PF_TRUECOLOR555:
			return COLOR2PIXEL555(c);
		case PF_TRUECOLOR1555:
			return COLOR2PIXEL1555(c);
		case PF_TRUECOLOR332:
			return COLOR2PIXEL332(c);
		case PF_TRUECOLOR233:
			return COLOR2PIXEL233(c);
	}
	//if (psd->ncolors == 2 && scrdev.pixtype != PF_PALETTE) return c & 1;
	return FindNearestColor(rp, (int)psd->ncolors, c);
}

UINT32 gfx_FindNearestColor(pPB rp, int size, UINT32 cr)
{
	int		best = 0;
//	APTR SysBase = g_SysBase;
//	DPrintF("FindNcolor: PixType: %x \n", PF_TRUECOLOR8888);
#if 0
 	MWPALENTRY *pal
	MWPALENTRY *	rgb;
	int		r, g, b;
	int		R, G, B;
	int32_t		diff = 0x7fffffffL;
	int32_t		sq;
	r = REDVALUE(cr);
	g = GREENVALUE(cr);
	b = BLUEVALUE(cr);
	for(rgb=pal; diff && rgb < &pal[size]; ++rgb) 
	{
		R = rgb->r - r;
		G = rgb->g - g;
		B = rgb->b - b;
		/* speedy linear distance method*/
		sq = abs(R) + abs(G) + abs(B);
		if(sq < diff) 
		{
			best = rgb - pal;
			if((diff = sq) == 0) return best;
		}
	}
#endif
	return best;
}

