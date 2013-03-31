#include "regionbase.h"
#include "_region_int.h"

BOOL region_OrRectRegion(struct RegionBase *RegionBase, struct Region *rgn, struct Rectangle *rect)
{
	struct RegionRectangle *r,*r1,*r2;
	struct Region newrgn;
    
	gfx_int_AdjustRegion(&newrgn,rgn,rect);

    if (r = gfx_int_NewRect(RegionBase))
    {
		gfx_int_PrependRR(&newrgn,r);
		r->bounds = *rect;
    	gfx_int_OffsetRegionRectangle(r,rgn->bounds.MinX,rgn->bounds.MinY);
    	for (r = rgn->RegionRectangle; r ; r = r->Next)
    	{
			for (r1 = newrgn.RegionRectangle ; r1 ; r1 = r2)
			{
				r2 = r1->Next;
				if (gfx_int_RectXRect(&r->bounds,&r1->bounds))
				{
					if (!gfx_int_Obscured(&r->bounds,&r1->bounds) )
					{
						gfx_int_RectSplit(RegionBase, &newrgn,r1->bounds,&r->bounds,OPERATION_OR);
						if (r2 == 0)	r2 = r1->Next;
					}
					gfx_int_RemoveRR(&newrgn,r1);
					gfx_int_FreeRR(RegionBase, r1);
				}
			}
		}
    	for (r = newrgn.RegionRectangle ; r ; r = r1 )
    	{
			r1 = r->Next;
			gfx_int_PrependRR(rgn,r);
    	}
		return (TRUE);
    }
    return (FALSE);
}
