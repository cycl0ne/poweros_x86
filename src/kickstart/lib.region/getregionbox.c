#include "regionbase.h"

int reg_GetRegionBox(RegionBase *RegionBase, ClipRegion *rgn, Rect *prc)
{
	*prc = rgn->extents;
	return rgn->type;
}
