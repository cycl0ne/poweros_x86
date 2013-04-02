#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "regions.h"
#include "region_funcs.h"

#define RegionBase CoreGfxBase->RegionBase

void cgfx_SetClipRegion(CoreGfxBase *CoreGfxBase, CRastPort *rp, ClipRegion *reg)
{
	if (rp->crp_ClipRegion) DestroyRegion(rp->crp_ClipRegion);
	if(!reg) reg = AllocRegion();

	rp->crp_ClipRegion = reg;
	if (rp->crp_ClipRegion->numRects == 0) 
	{
		rp->crp_ClipMinX = MIN_COORD;
		rp->crp_ClipMinY = MIN_COORD;
		rp->crp_ClipMaxX = MAX_COORD;
		rp->crp_ClipMaxY = MAX_COORD;
		rp->crp_ClipResult = FALSE;
		return;
	}

	/* There was at least one valid clip rectangle. Default the clip
	* cache to be the first clip rectangle.
	*/
	rp->crp_ClipMinX = rp->crp_ClipRegion->rects[0].left;
	rp->crp_ClipMinY = rp->crp_ClipRegion->rects[0].top;
	rp->crp_ClipMaxX = rp->crp_ClipRegion->rects[0].right - 1;
	rp->crp_ClipMaxY = rp->crp_ClipRegion->rects[0].bottom - 1;
	rp->crp_ClipResult = TRUE;
}

BOOL cgfx_ClipPoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
{
	INT32 count;
	Rect *rect;
	INT32 temp;

	struct PixMap *psd = rp->crp_PixMap;

	/* First see whether the point lies within the current clip cache
	* rectangle.  If so, then we already know the result.
	*/
	if ((x >= rp->crp_ClipMinX) && (x <= rp->crp_ClipMaxX) &&
	  (y >= rp->crp_ClipMinY) && (y <= rp->crp_ClipMaxY)) 
	{
		if (rp->crp_ClipResult) CheckCursor(rp, x, y, x, y);
		return rp->crp_ClipResult;
	}

	  /* If the point is outside of the screen area, then it is not
	   * plottable, and the clip cache rectangle is the whole half-plane
	   * outside of the screen area.
	   */
	if (x < 0) {
		rp->crp_ClipMinX = MIN_COORD;
		rp->crp_ClipMaxX = -1;
		rp->crp_ClipMinY = MIN_COORD;
		rp->crp_ClipMaxY = MAX_COORD;
		rp->crp_ClipResult = FALSE;
		return FALSE;
	}
	if (y < 0) {
		rp->crp_ClipMinX = MIN_COORD;
		rp->crp_ClipMaxX = MAX_COORD;
		rp->crp_ClipMinY = MIN_COORD;
		rp->crp_ClipMaxY = -1;
		rp->crp_ClipResult = FALSE;
		return FALSE;
	}
	if (x >= psd->xvirtres) {
		rp->crp_ClipMinX = psd->xvirtres;
		rp->crp_ClipMaxX = MAX_COORD;
		rp->crp_ClipMinY = MIN_COORD;
		rp->crp_ClipMaxY = MAX_COORD;
		rp->crp_ClipResult = FALSE;
		return FALSE;
	}
	if (y >= psd->yvirtres) {
		rp->crp_ClipMinX = MIN_COORD;
		rp->crp_ClipMaxX = MAX_COORD;
		rp->crp_ClipMinY = psd->yvirtres;
		rp->crp_ClipMaxY = MAX_COORD;
		rp->crp_ClipResult = FALSE;
		return FALSE;
	}

  /* The point is within the screen area. If there are no clip
   * rectangles, then the point is plottable and the rectangle is the
   * whole screen.
   */
   
	count = rp->crp_ClipRegion->numRects;
	if (count <= 0) 
	{
		rp->crp_ClipMinX = 0;
		rp->crp_ClipMaxX = psd->xvirtres - 1;
		rp->crp_ClipMinY = 0;
		rp->crp_ClipMaxY = psd->yvirtres - 1;
		rp->crp_ClipResult = TRUE;
		CheckCursor(rp, x, y, x, y);
		return TRUE;
	}

	/* We need to scan the list of clip rectangles to calculate a new
	* clip cache rectangle containing this point, and the result. First
	* see if the point lies within any of the clip rectangles. If so,
	* then it is plottable and use that clip rectangle as the cache
	* rectangle.  This is not necessarily the best result, but works ok
	* and is fast.
	*/
	for (rect = rp->crp_ClipRegion->rects; count-- > 0; rect++) {
		if ((x >= rect->left) && (y >= rect->top) && (x < rect->right) && (y < rect->bottom)) {
			rp->crp_ClipMinX = rect->left;
			rp->crp_ClipMinY = rect->top;
			rp->crp_ClipMaxX = rect->right - 1;
			rp->crp_ClipMaxY = rect->bottom - 1;
			rp->crp_ClipResult = TRUE;
			CheckCursor(rp, x, y, x, y);
			return TRUE;
		}
	}

	/* The point is not plottable. Scan the clip rectangles again to
	* determine a rectangle containing more non-plottable points.
	* Simply pick the largest rectangle whose area doesn't contain any
	* of the same coordinates as appropriate sides of the clip
	* rectangles.  This is not necessarily the best result, but works ok
	* and is fast.
	*/
	rp->crp_ClipMinX = MIN_COORD;
	rp->crp_ClipMinY = MIN_COORD;
	rp->crp_ClipMaxX = MAX_COORD;
	rp->crp_ClipMaxY = MAX_COORD;
	count = rp->crp_ClipRegion->numRects;

	for (rect = rp->crp_ClipRegion->rects; count-- > 0; rect++) 
	{
		if ((x < rect->left) && (rect->left <= rp->crp_ClipMaxX)) rp->crp_ClipMaxX = rect->left - 1;
		temp = rect->right - 1;
		if ((x > temp) && (temp >= rp->crp_ClipMinX)) rp->crp_ClipMinX = temp + 1;
		if ((y < rect->top) && (rect->top <= rp->crp_ClipMaxY)) rp->crp_ClipMaxY = rect->top - 1;
		temp = rect->bottom - 1;
		if ((y > temp) && (temp >= rp->crp_ClipMinY)) rp->crp_ClipMinY = temp + 1;
	}
	rp->crp_ClipResult = FALSE;
	return FALSE;
}

INT32 cgfx_ClipArea(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2)
{
	if ((x1 < rp->crp_ClipMinX) || (x1 > rp->crp_ClipMaxX) || (y1 < rp->crp_ClipMinY) || (y1 > rp->crp_ClipMaxY)) ClipPoint(rp, x1, y1);

	if ((x2 >= rp->crp_ClipMinX) && (x2 <= rp->crp_ClipMaxX) && (y2 >= rp->crp_ClipMinY) && (y2 <= rp->crp_ClipMaxY)) 
	{
		if (!rp->crp_ClipResult) return CLIP_INVISIBLE;
		CheckCursor(rp, x1, y1, x2, y2);
		return CLIP_VISIBLE;
	}
	return CLIP_PARTIAL;
}


