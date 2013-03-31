#include "regionbase.h"
#include "_region_int.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

BOOL region_XorRectRegion(RegionBase *RegionBase, struct Region *rgn, struct Rectangle *rect)
{
	if (rgn->RegionRectangle == 0) return OrRectRegion(rgn,rect);
	else
	{
		struct Region outsideregion, insideregion;
		struct RegionRectangle *nrect,*r,*r1,*r2;

		gfx_int_AdjustRegion(&outsideregion,rgn,rect);
		insideregion = outsideregion;
		nrect = gfx_int_NewRect(RegionBase);
		if (!nrect)	Alert(AN_REGIONMEMORY, "Region.library - Out of Memory\n");
		nrect->bounds = *rect;
		gfx_int_OffsetRegionRectangle(nrect, rgn->bounds.MinX, rgn->bounds.MinY);

		for (r = rgn->RegionRectangle; r ; r = r->Next)
		{
			gfx_int_RectSplit(RegionBase, &outsideregion,r->bounds,&nrect->bounds,OPERATION_OR);
		}

		gfx_int_PrependRR(&insideregion,nrect);

		for (r = rgn->RegionRectangle; r ; r = r->Next)
		{
			r1 = insideregion.RegionRectangle;
			insideregion.RegionRectangle = 0;
			for ( ; r1 ; r1 = r2)
			{
				gfx_int_RectSplit(RegionBase, &insideregion,r1->bounds, &r->bounds, OPERATION_OR);
				r2 = r1->Next;
				gfx_int_FreeRR(RegionBase, r1);
			}
		}

		while  ( r = insideregion.RegionRectangle )
		{
			gfx_int_RemoveRR(&insideregion,r);
			gfx_int_PrependRR(&outsideregion,r);
		}
		gfx_int_ReplaceRegion(RegionBase, &outsideregion,rgn);
	}
	return (TRUE);
}

