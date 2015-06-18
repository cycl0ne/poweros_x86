#include "regionbase.h"

BOOL reg_PtInRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 x, INT32 y)
{
    INT32 i;

    if (rgn->numRects > 0 && INRECT(rgn->extents, x, y))
	for (i = 0; i < rgn->numRects; i++)
	    if (INRECT (rgn->rects[i], x, y)) return TRUE;
    return FALSE;
}
