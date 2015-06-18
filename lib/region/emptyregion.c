#include "regionbase.h"

BOOL reg_EmptyRegion(RegionBase *RegionBase, ClipRegion *rgn)
{
	return rgn->numRects == 0;
}
