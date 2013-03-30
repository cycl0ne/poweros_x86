#include "coregfx.h"
#include "rastport.h"

UINT32 GdFindColor(CRastPort *CRastPort, UINT32 c)
{
	return c;
}

UINT32 cgfx_SetMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode)
{
	UINT32 oldmode = CRastPort->crp_mode;
	CRastPort->crp_mode = mode;
	return oldmode;
}

UINT32 cgfx_SetFillMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode)
{
	UINT32 oldmode = CRastPort->crp_fillmode;
	CRastPort->crp_mode = mode;
	return oldmode;
}

BOOL cgfx_SetUseBackground(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, BOOL flag)
{
	BOOL oldusebg = CRastPort->crp_usebg;
	CRastPort->crp_usebg = flag;
	return oldusebg;
}

UINT32 cgfx_SetForegroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg)
{
	UINT32 oldfg = CRastPort->crp_foreground;
	CRastPort->crp_foreground = fg;
	return oldfg;
}

UINT32 cgfx_SetBackgroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg)
{
	UINT32 oldbg = CRastPort->crp_background;
	CRastPort->crp_background = bg;
	return oldbg;
}

UINT32 cgfx_SetForegroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg)
{
	UINT32 oldfg = CRastPort->crp_foreground;

	CRastPort->crp_foreground = GdFindColor(CRastPort, fg);
	CRastPort->crp_foreground_rgb = fg;
	return oldfg;
}

UINT32 cgfx_SetBackgroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg)
{
	UINT32 oldbg = CRastPort->crp_background;

	CRastPort->crp_background = GdFindColor(CRastPort, bg);
	CRastPort->crp_background_rgb = bg;
	return oldbg;
}

void cgfx_SetDash(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 *mask, UINT32 *count)
{
	UINT32 oldm = CRastPort->crp_dashmask;
	UINT32 oldc = CRastPort->crp_dashcount;

	if (!mask || !count) return;

	CRastPort->crp_dashmask = *mask;
	CRastPort->crp_dashcount = *count;

	*mask = oldm;
	*count = oldc;
}
