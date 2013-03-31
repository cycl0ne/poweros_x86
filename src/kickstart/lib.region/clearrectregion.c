#include "regionbase.h"
#include "_region_int.h"

BOOL region_ClearRectRegion(RegionBase *RegionBase, Region *rgn, struct Rectangle *rect)
{
	struct Region *rgntmp;
	if (rgntmp = NewRegion())
	{
		OrRegionRegion(rgn,rgntmp);
		AndRectRegion(rgntmp,rect);
		XorRegionRegion(rgntmp,rgn);
		DisposeRegion(rgntmp);
		return (TRUE);
	}
	return (FALSE);
}
