#include "regionbase.h"

void reg_UnionRectWithRegion(RegionBase *RegionBase, const Rect *rect, ClipRegion *rgn)
{
    ClipRegion region;

    region.rects = &region.extents;
    region.numRects = 1;
    region.size = 1;
    region.type = REGION_SIMPLE;
    region.extents		= *rect;
    UnionRegion(rgn, rgn, &region);
}
