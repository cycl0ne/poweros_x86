#include "regionbase.h"
#include "_region_int.h"

BOOL region_OrRegionRegion(RegionBase *RegionBase, Region *rgnsrc, struct Region *rgndst)
{
    struct RegionRectangle *rr;
    struct Rectangle rect;
    for (rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next)
    {
        rect = rr->bounds;
        rect.MinX += rgnsrc->bounds.MinX;
        rect.MaxX += rgnsrc->bounds.MinX;
        rect.MinY += rgnsrc->bounds.MinY;
        rect.MaxY += rgnsrc->bounds.MinY;
        OrRectRegion(rgndst,&rect);
    }
    return (TRUE);
}
