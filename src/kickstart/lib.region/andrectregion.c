#include "regionbase.h"
#include "_region_int.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

void region_AndRectRegion(RegionBase *RegionBase, struct Region *rgn, struct Rectangle *rect)
{
    struct RegionRectangle *r, *r1;
    struct Region newrgn;
	struct RegionRectangle rr_tmp;
    
	newrgn.RegionRectangle = 0;

    if (rgn->RegionRectangle == 0)	return;
    else
    {
		gfx_int_Intersect(&rgn->bounds, rect, &newrgn.bounds);
		    
		if ( (newrgn.bounds.MinX != rgn->bounds.MinX) ||
		     (newrgn.bounds.MinY != rgn->bounds.MinY) )
		{
		    gfx_int_AdjustRegionRectangles(rgn,newrgn.bounds.MinX-rgn->bounds.MinX, newrgn.bounds.MinY-rgn->bounds.MinY);
		}
		rgn->bounds = newrgn.bounds;
    }
	r = &rr_tmp;
	r->bounds = *rect;
    gfx_int_OffsetRegionRectangle(r,rgn->bounds.MinX,rgn->bounds.MinY);

    for (r = rgn->RegionRectangle; r ; r = r1)
    {
		r1 = r->Next;
		if (gfx_int_RectXRect(&rr_tmp.bounds,&r->bounds))
		{
			if (!gfx_int_Obscured(&r->bounds,&rr_tmp.bounds) )
			{
				if (gfx_int_Obscured(&rr_tmp.bounds,&r->bounds))
				{
					gfx_int_RemoveRR(rgn,r);
					gfx_int_PrependRR(&newrgn,r);
				} else 
				{
					gfx_int_RectSplit(RegionBase, &newrgn, rr_tmp.bounds, &r->bounds, OPERATION_AND);
				}
			} else
			{
				r = newrgn.RegionRectangle = gfx_int_NewRect(RegionBase);
				if (!r)	Alert(AN_REGIONMEMORY, "region.library - No Memory\n");
				r->bounds = rr_tmp.bounds;
				break;
			}
		}
    }
    gfx_int_ReplaceRegion(RegionBase, &newrgn,rgn);
}
