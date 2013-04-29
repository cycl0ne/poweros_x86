#include "intuitionbase.h"
#include "regions.h"
#include "region_funcs.h"

#define CoreGfxBase IBase->ib_GfxBase
#define RegionBase 	IBase->ib_RgnBase
#define SysBase 	IBase->ib_SysBase

UINT32 _PrepareDrawing(IntuitionBase *IBase, struct nWindow *wp) //, struct IntuDrawable **retdp)
{
//	CRastPort 	*rp = wp->rp;
	ClipRegion	*regionp = wp->userclipregion;
	INT32		xoff = wp->xoff;
	INT32		yoff = wp->yoff;
//	ClipRegion	*reg;

	if (!wp->realized) return GR_DRAW_TYPE_NONE;
	if (wp != IBase->clipwp) 
	{
		/* Special handling if user region is not at offset 0,0*/
		if (regionp && (xoff || yoff)) 
		{
			ClipRegion *local = AllocRegion();
			CopyRegion(local, regionp);
			OffsetRegion(local, xoff, yoff);
			_SetClipWindow(IBase, wp, local, 0);//gcp->mode & ~GR_MODE_DRAWMASK);
			DestroyRegion(local);
		} else
		_SetClipWindow(IBase, wp, regionp? regionp: NULL, 0); //gcp->mode & ~GR_MODE_DRAWMASK);
	}
	return GR_DRAW_TYPE_WINDOW;
}
