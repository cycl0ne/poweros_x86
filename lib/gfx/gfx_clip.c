/**
* File: /gfx_clip．c
* User: cycl0ne
* Date: 2014-11-26
* Time: 11:49 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"
#include "region_interface.h"

#define SysBase GfxBase->SysBase
#define RegionBase GfxBase->RegionBase

void gfx_SetClipRegion(pGfxBase GfxBase, pPB pb, pClipRegion reg)
{
	if (pb->pb_ClipRegion) DestroyRegion(pb->pb_ClipRegion);
	if (!reg) reg = AllocRegion();

	pb->pb_ClipRegion = reg;
	if (pb->pb_ClipRegion->numRects == 0) 
	{
		pb->pb_ClipMinX = MIN_COORD;
		pb->pb_ClipMinY = MIN_COORD;
		pb->pb_ClipMaxX = MAX_COORD;
		pb->pb_ClipMaxY = MAX_COORD;
		pb->pb_ClipResult = FALSE;
		return;
	}

	/* There was at least one valid clip rectangle. Default the clip
	* cache to be the first clip rectangle.
	*/
	pb->pb_ClipMinX = pb->pb_ClipRegion->rects[0].left;
	pb->pb_ClipMinY = pb->pb_ClipRegion->rects[0].top;
	pb->pb_ClipMaxX = pb->pb_ClipRegion->rects[0].right - 1;
	pb->pb_ClipMaxY = pb->pb_ClipRegion->rects[0].bottom - 1;
	pb->pb_ClipResult = TRUE;
}

BOOL gfx_ClipPoint(pGfxBase GfxBase, pPB pb, INT32 x, INT32 y)
{
	INT32 count;
	pRect rect;
	INT32 temp;

	pSD psd = pb->pb_PixMap;

	/* First see whether the point lies within the current clip cache
	* rectangle.  If so, then we already know the result.
	*/
	if ((x >= pb->pb_ClipMinX) && (x <= pb->pb_ClipMaxX) &&
	  (y >= pb->pb_ClipMinY) && (y <= pb->pb_ClipMaxY)) 
	{
		if (pb->pb_ClipResult) CheckCursor(pb->pb_PixMap, x, y, x, y);
//		DPrintF("CACHI :)\n");
		return pb->pb_ClipResult;
	}
//DPrintF("ClipCache0\n");

	  /* If the point is outside of the screen area, then it is not
	   * plottable, and the clip cache rectangle is the whole half-plane
	   * outside of the screen area.
	   */
	if (x < 0) {
		pb->pb_ClipMinX = MIN_COORD;
		pb->pb_ClipMaxX = -1;
		pb->pb_ClipMinY = MIN_COORD;
		pb->pb_ClipMaxY = MAX_COORD;
		pb->pb_ClipResult = FALSE;
		return FALSE;
	}
	if (y < 0) {
		pb->pb_ClipMinX = MIN_COORD;
		pb->pb_ClipMaxX = MAX_COORD;
		pb->pb_ClipMinY = MIN_COORD;
		pb->pb_ClipMaxY = -1;
		pb->pb_ClipResult = FALSE;
		return FALSE;
	}
	if (x >= psd->xvirtres) {
		pb->pb_ClipMinX = psd->xvirtres;
		pb->pb_ClipMaxX = MAX_COORD;
		pb->pb_ClipMinY = MIN_COORD;
		pb->pb_ClipMaxY = MAX_COORD;
		pb->pb_ClipResult = FALSE;
		return FALSE;
	}
	if (y >= psd->yvirtres) {
		pb->pb_ClipMinX = MIN_COORD;
		pb->pb_ClipMaxX = MAX_COORD;
		pb->pb_ClipMinY = psd->yvirtres;
		pb->pb_ClipMaxY = MAX_COORD;
		pb->pb_ClipResult = FALSE;
		return FALSE;
	}

  /* The point is within the screen area. If there are no clip
   * rectangles, then the point is plottable and the rectangle is the
   * whole screen.
   */
   
	count = pb->pb_ClipRegion->numRects;
	if (count <= 0) 
	{
		pb->pb_ClipMinX = 0;
		pb->pb_ClipMaxX = psd->xvirtres - 1;
		pb->pb_ClipMinY = 0;
		pb->pb_ClipMaxY = psd->yvirtres - 1;
		pb->pb_ClipResult = TRUE;
		CheckCursor(pb->pb_PixMap, x, y, x, y);
		return TRUE;
	}
//DPrintF("ClipCache\n");
	/* We need to scan the list of clip rectangles to calculate a new
	* clip cache rectangle containing this point, and the result. First
	* see if the point lies within any of the clip rectangles. If so,
	* then it is plottable and use that clip rectangle as the cache
	* rectangle.  This is not necessarily the best result, but works ok
	* and is fast.
	*/
	for (rect = pb->pb_ClipRegion->rects; count-- > 0; rect++) {
		if ((x >= rect->left) && (y >= rect->top) && (x < rect->right) && (y < rect->bottom)) {
			pb->pb_ClipMinX = rect->left;
			pb->pb_ClipMinY = rect->top;
			pb->pb_ClipMaxX = rect->right - 1;
			pb->pb_ClipMaxY = rect->bottom - 1;
			pb->pb_ClipResult = TRUE;
			CheckCursor(pb->pb_PixMap, x, y, x, y);
			return TRUE;
		}
	}
//DPrintF("ClipCache2\n");

	/* The point is not plottable. Scan the clip rectangles again to
	* determine a rectangle containing more non-plottable points.
	* Simply pick the largest rectangle whose area doesn't contain any
	* of the same coordinates as appropriate sides of the clip
	* rectangles.  This is not necessarily the best result, but works ok
	* and is fast.
	*/
	pb->pb_ClipMinX = MIN_COORD;
	pb->pb_ClipMinY = MIN_COORD;
	pb->pb_ClipMaxX = MAX_COORD;
	pb->pb_ClipMaxY = MAX_COORD;
	count = pb->pb_ClipRegion->numRects;

	for (rect = pb->pb_ClipRegion->rects; count-- > 0; rect++) 
	{
		if ((x < rect->left) && (rect->left <= pb->pb_ClipMaxX)) pb->pb_ClipMaxX = rect->left - 1;
		temp = rect->right - 1;
		if ((x > temp) && (temp >= pb->pb_ClipMinX)) pb->pb_ClipMinX = temp + 1;
		if ((y < rect->top) && (rect->top <= pb->pb_ClipMaxY)) pb->pb_ClipMaxY = rect->top - 1;
		temp = rect->bottom - 1;
		if ((y > temp) && (temp >= pb->pb_ClipMinY)) pb->pb_ClipMinY = temp + 1;
	}
	//DPrintF("ClipCache3\n");
	pb->pb_ClipResult = FALSE;
	return FALSE;
}

INT32 gfx_ClipArea(pGfxBase GfxBase, pPB pb, INT32 x1, INT32 y1, INT32 x2, INT32 y2)
{
	//	DPrintF("ClipArea %d, %d, %d, %d\n", x1, y1, x2, y2);
	//	DPrintF("RP: %d, %d, %d, %d\n",pb->pb_ClipMinX,pb->pb_ClipMinY, pb->pb_ClipMaxX,pb->pb_ClipMaxY);
	if ((x1 < pb->pb_ClipMinX) || (x1 > pb->pb_ClipMaxX) || (y1 < pb->pb_ClipMinY) || (y1 > pb->pb_ClipMaxY)) 
	{
	//		DPrintF("Clippoint %d, %d\n", x1, y1);
		ClipPoint(pb, x1, y1);
	}
	//	DPrintF("ClipArea %d, %d, %d, %d\n", x1, y1, x2, y2);
	//	DPrintF("RP: %d, %d, %d, %d\n",pb->pb_ClipMinX,pb->pb_ClipMinY, pb->pb_ClipMaxX,pb->pb_ClipMaxY);
	if ((x2 >= pb->pb_ClipMinX) && (x2 <= pb->pb_ClipMaxX) && (y2 >= pb->pb_ClipMinY) && (y2 <= pb->pb_ClipMaxY)) 
	{
	//		DPrintF("ClipResult: %x", pb->pb_ClipResult);
		if (!pb->pb_ClipResult) return CLIP_INVISIBLE;
		CheckCursor(pb->pb_PixMap, x1, y1, x2, y2);
		return CLIP_VISIBLE;
	}
	return CLIP_PARTIAL;
}
