#include "regionbase.h"

static inline BOOL EQUALRECT(Rect *r1, Rect *r2)
{
	return 	((r1->left == r2->left) && (r1->right == r2->right) &&
			(r1->top == r2->top) && (r1->bottom == r2->bottom));
}

BOOL reg_EqualRegion(RegionBase *RegionBase, ClipRegion *r1, ClipRegion *r2)
{
	INT32	i;

	if (r1->numRects != r2->numRects) return FALSE;
	if (r1->numRects == 0) return TRUE;
	if (!EQUALRECT(&r1->extents, &r2->extents)) return FALSE;
	for (i = 0; i < r1->numRects; i++) 
	{
		if (!EQUALRECT(r1->rects + i, r2->rects + i)) return FALSE;
	}
	return TRUE;
}
