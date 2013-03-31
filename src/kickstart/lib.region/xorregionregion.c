#include "regionbase.h"
#include "_region_int.h"

BOOL region_XorRegionRegion(RegionBase *RegionBase, Region *rgnsrc, struct Region *rgndst)
{
    RegionRectangle *rr;
    Rectangle rect;
    for (rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next)
    {
        rect = rr->bounds;
        rect.MinX += rgnsrc->bounds.MinX;
        rect.MaxX += rgnsrc->bounds.MinX;
        rect.MinY += rgnsrc->bounds.MinY;
        rect.MaxY += rgnsrc->bounds.MinY;
        XorRectRegion(rgndst,&rect);
    }
    return (TRUE);
}
