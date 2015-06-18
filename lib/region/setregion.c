#include "regionbase.h"

void reg_SetRectRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 left, INT32 top, INT32 right, INT32 bottom)
{
	if (left != right && top != bottom) {
		rgn->rects->left = rgn->extents.left = left;
		rgn->rects->top = rgn->extents.top = top;
		rgn->rects->right = rgn->extents.right = right;
		rgn->rects->bottom = rgn->extents.bottom = bottom;
		rgn->numRects = 1;
		rgn->type = REGION_SIMPLE;
	} else
		EMPTY_REGION(rgn);
}

void reg_SetRectRegionIndirect(RegionBase *RegionBase, ClipRegion *rgn, Rect *prc)
{
	SetRectRegion(rgn, prc->left, prc->top, prc->right, prc->bottom);
}

