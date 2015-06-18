/**
* File: /gfx_view．c
* User: cycl0ne
* Date: 2014-11-29
* Time: 04:07 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#include "region_interface.h"
#include "framebuffer_interface.h"
#include "gfx_interface.h"

#define SysBase GfxBase->SysBase
#define RegionBase GfxBase->RegionBase

static void InitPB(pGfxBase GfxBase, pPB pb)
{
	SetMode(pb, ROP_COPY);
	SetFillMode(pb, FILL_SOLID);
	SetUseBackground(pb, TRUE);	

	SetDash(pb, 0, 0);
//	SetStippleBitmap(pb, 0,0,0);
	SetClipRegion(pb, AllocRectRegion(0,0, GfxBase->scrdev->xvirtres, GfxBase->scrdev->yvirtres));
	SetForegroundColor(pb, RGB(0, 0, 0));
	SetBackgroundColor(pb, RGB(0, 0, 0));
	RectFill(pb, 0, 0, pb->pb_PixMap->xvirtres, pb->pb_PixMap->yvirtres);
	SetForegroundColor(pb, RGB(255, 255, 255));
}

pPB gfx_AllocPaintBox(pGfxBase GfxBase, pPixMap pm)
{
	if (!pm) return NULL;
	
	pPB ret = AllocVec(sizeof(PaintBox_t), MEMF_FAST|MEMF_CLEAR);
	if (ret)
	{
		ret->pb_PixMap = pm;
		InitPB(GfxBase, ret);
	}
	return ret;
}

pSD gfx_OpenView(pGfxBase GfxBase, int32_t w, int32_t h, int32_t bpp)
{
	pPB		pb = &GfxBase->paintBox;
	APTR FBBase = OpenLibrary("framebuffer.library",0);
	if (FBBase)
	{
		GfxBase->scrdev = OpenFB( w, h, bpp);
		pb->pb_PixMap = GfxBase->scrdev;
		InitPB(GfxBase, pb);
	} else
		KPrintF("Failed fb\n");
	return (pSD) pb; //GfxBase->scrdev;
}
