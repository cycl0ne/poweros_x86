#include "regionbase.h"
#include "_region_int.h"

BOOL region_AndRegionRegion(RegionBase *RegionBase, struct Region *rgnsrc, struct Region *rgndst)
{
	struct Region *newrgn,*rgn;
	struct RegionRectangle *rr;
	struct Rectangle rect;
	int out_of_memory = TRUE;

	newrgn = NewRegion();
	if (newrgn == 0)	return(FALSE);

	rgn = NewRegion();
	if (rgn == 0)	
	{
		if (newrgn)	DisposeRegion(newrgn);
		return(FALSE);
	}

	for( rr = rgnsrc->RegionRectangle; rr ; rr = rr->Next )
	{
		if (!OrRegionRegion(rgndst,rgn))
		{
			if (rgn)	DisposeRegion(rgn);
			if (newrgn)	DisposeRegion(newrgn);
			return(FALSE);
		}
		rect = rr->bounds;
		rect.MinX += rgnsrc->bounds.MinX;
		rect.MaxX += rgnsrc->bounds.MinX;
		rect.MinY += rgnsrc->bounds.MinY;
		rect.MaxY += rgnsrc->bounds.MinY;
		AndRectRegion(rgn,&rect);
		if (!OrRegionRegion(rgn,newrgn)) 
		{	
			if (rgn)	DisposeRegion(rgn);
			if (newrgn)	DisposeRegion(newrgn);
			return(FALSE);
		}
		ClearRegion(rgn);
	}
	DisposeRegion(rgn);
	gfx_int_ReplaceRegion(RegionBase, newrgn,rgndst);
	DisposeRegion(newrgn);
	return(TRUE);
}

