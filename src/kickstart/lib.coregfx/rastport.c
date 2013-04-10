#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "region_funcs.h"

#define SysBase CoreGfxBase->SysBase
#define RegionBase CoreGfxBase->RegionBase

struct CRastPort *cgfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm)
{
	struct CRastPort *rp = AllocVec(sizeof(struct CRastPort), MEMF_CLEAR|MEMF_FAST);
	if (rp)
	{
		rp->crp_PixMap = bm;
		SetMode(rp, ROP_COPY);
		SetFillMode(rp, FILL_SOLID);
		SetForegroundPixelVal(rp, 0x0);
		SetBackgroundPixelVal(rp, 0x0);
		SetUseBackground(rp, TRUE);
		SetDash(rp, 0,0);
		//SetStippleBitmap(0,0,0);
		//DPrintF("SetClipRegion %x\n", RegionBase);
		SetClipRegion(rp, AllocRectRegion(0, 0, bm->xvirtres, bm->yvirtres));
		//DPrintF("SetClipRegion2\n");
	}
	return rp;
}

void cgfx_FreeRastPort(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, BOOL flag)
{
	if (flag); //FreeBitmap;
	FreeVec(rp);
}

