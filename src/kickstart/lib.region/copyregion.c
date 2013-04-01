#include "regionbase.h"

void reg_CopyRegion(RegionBase *RegionBase, ClipRegion *dst, ClipRegion *src)
{
	if (dst != src) /*  don't want to copy to itself */
	{  
		if (dst->size < src->numRects)
		{
			if (! (dst->rects = Realloc( dst->rects, dst->numRects * sizeof(Rect), src->numRects * sizeof(Rect)))) return;
			dst->size = src->numRects;
		}
		dst->numRects = src->numRects;
		dst->extents.left = src->extents.left;
		dst->extents.top = src->extents.top;
		dst->extents.right = src->extents.right;
		dst->extents.bottom = src->extents.bottom;
		dst->type = src->type;
		memcpy((char *) dst->rects, (char *) src->rects, (int) (src->numRects * sizeof(Rect)));
	}
}
