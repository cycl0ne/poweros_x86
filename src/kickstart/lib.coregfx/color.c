#include "coregfx.h"
#include "rastport.h"

UINT32 GdFindColor(CRastPort *CRastPort, UINT32 c)
{
	return c;
}

UINT32 cgfx_SetMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode)
{
	UINT32 oldmode = CRastPort->crp_Mode;
	CRastPort->crp_Mode = mode;
	return oldmode;
}

UINT32 cgfx_SetFillMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode)
{
	UINT32 oldmode = CRastPort->crp_Fillmode;
	CRastPort->crp_Fillmode = mode;
	return oldmode;
}

BOOL cgfx_SetUseBackground(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, BOOL flag)
{
	BOOL oldusebg = CRastPort->crp_useBg;
	CRastPort->crp_useBg = flag;
	return oldusebg;
}

UINT32 cgfx_SetForegroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg)
{
	UINT32 oldfg = CRastPort->crp_Foreground;
	CRastPort->crp_Foreground = fg;
	return oldfg;
}

UINT32 cgfx_SetBackgroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg)
{
	UINT32 oldbg = CRastPort->crp_Background;
	CRastPort->crp_Background = bg;
	return oldbg;
}

UINT32 cgfx_SetForegroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg)
{
	UINT32 oldfg = CRastPort->crp_Foreground;

	CRastPort->crp_Foreground = GdFindColor(CRastPort, fg);
	CRastPort->crp_ForegroundRGB = fg;
	return oldfg;
}

UINT32 cgfx_SetBackgroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg)
{
	UINT32 oldbg = CRastPort->crp_Background;

	CRastPort->crp_Background = GdFindColor(CRastPort, bg);
	CRastPort->crp_BackgroundRGB = bg;
	return oldbg;
}

void cgfx_SetDash(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 *mask, UINT32 *count)
{
	UINT32 oldm = CRastPort->crp_Dashmask;
	UINT32 oldc = CRastPort->crp_Dashcount;

	if (!mask || !count) return;

	CRastPort->crp_Dashmask = *mask;
	CRastPort->crp_Dashcount = *count;

	*mask = oldm;
	*count = oldc;
}
