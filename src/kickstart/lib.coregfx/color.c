#include "coregfx.h"
#include "rastport.h"
#include "pixmap.h"

#define SysBase CoreGfxBase->SysBase

UINT32 cgfx_SetMode(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 mode)
{
	UINT32 oldmode = rp->crp_Mode;
	rp->crp_Mode = mode;
	return oldmode;
}

UINT32 cgfx_SetFillMode(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 mode)
{
	UINT32 oldmode = rp->crp_FillMode;
	rp->crp_FillMode = mode;
	return oldmode;
}

BOOL cgfx_SetUseBackground(CoreGfxBase *CoreGfxBase, CRastPort *rp, BOOL flag)
{
	BOOL oldusebg = rp->crp_useBg;
	rp->crp_useBg = flag;
	return oldusebg;
}

UINT32 cgfx_SetForegroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 fg)
{
	UINT32 oldfg = rp->crp_Foreground;
	rp->crp_Foreground = fg;
	return oldfg;
}

UINT32 cgfx_SetBackgroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 bg)
{
	UINT32 oldbg = rp->crp_Background;
	rp->crp_Background = bg;
	return oldbg;
}

UINT32 cgfx_SetForegroundColor(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 fg)
{
	UINT32 oldfg = rp->crp_Foreground;

	rp->crp_Foreground = FindColor(rp, fg);
	rp->crp_ForegroundRGB = fg;
//	DPrintF("Findcolor: Old: %x, Found: %x\n", fg, rp->crp_Foreground);
	return oldfg;
}

UINT32 cgfx_SetBackgroundColor(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 bg)
{
	UINT32 oldbg = rp->crp_Background;

	rp->crp_Background = FindColor(rp, bg);
	rp->crp_BackgroundRGB = bg;
	return oldbg;
}

void cgfx_SetDash(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT32 *mask, UINT32 *count)
{
	UINT32 oldm = rp->crp_DashMask;
	UINT32 oldc = rp->crp_DashCount;

	if (!mask || !count) return;

	rp->crp_DashMask = *mask;
	rp->crp_DashCount = *count;

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

UINT32 cgfx_FindColor(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, UINT32 c)
{
	PixMap *psd = rp->crp_PixMap;
//DPrintF("psd->PixType: %x\n", psd->pixtype);
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

extern APTR g_SysBase;
#undef SysBase
UINT32 cgfx_FindNearestColor(CRastPort *rp, int size, UINT32 cr)
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






