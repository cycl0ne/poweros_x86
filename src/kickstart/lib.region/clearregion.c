#include "regionbase.h"
#include "_region_int.h"

void region_ClearRegion(RegionBase *RegionBase, Region *rgn)
{
    struct RegionRectangle *r,*r1;
    rgn->bounds.MinX = 0;
    rgn->bounds.MinY = 0;
    rgn->bounds.MaxX = 0;
    rgn->bounds.MaxY = 0;
    for (r = rgn->RegionRectangle; r ; r = r1)  
    {
		r1 = r->Next;
		gfx_int_FreeRR(RegionBase, r);
    }
    rgn->RegionRectangle = 0;
}
