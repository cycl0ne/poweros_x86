#include "regionbase.h"

int reg_GetRegionBox(RegionBase *RegionBase, ClipRegion *rgn, pRect prc)
{
	*prc = rgn->extents;
	return rgn->type;
}
