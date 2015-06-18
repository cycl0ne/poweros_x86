#include "regionbase.h"

void reg_OffsetRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 x, INT32 y)
{
	int	nbox = rgn->numRects;
	pRect pbox = rgn->rects;

	if(nbox && (x || y)) 
	{
		while(nbox--) 
		{
			pbox->left += x;
			pbox->right += x;
			pbox->top += y;
			pbox->bottom += y;
			pbox++;
		}
		rgn->extents.left += x;
		rgn->extents.right += x;
		rgn->extents.top += y;
		rgn->extents.bottom += y;
	}
}
