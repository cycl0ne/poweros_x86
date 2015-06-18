#include "regionbase.h"

INT32 reg_RectInRegion(RegionBase *RegionBase, ClipRegion *rgn, const Rect *rect)
{
    Rect *	pCurRect;
    Rect *	pRectEnd;
    INT32	rx, ry;
    BOOL	partIn, partOut;

    /* this is (just) a useful optimization */
    if (!rgn->numRects || !EXTENTCHECK(&rgn->extents, rect)) return RECT_OUT;

    partOut = FALSE;
    partIn = FALSE;
    rx = rect->left;
    ry = rect->top;

    /* 
     * can stop when both partOut and partIn are TRUE,
     * or we reach rect->bottom
     */
    for (pCurRect = rgn->rects, pRectEnd = pCurRect + rgn->numRects;
		 pCurRect < pRectEnd; pCurRect++) 
	{
		if (pCurRect->bottom <= ry) continue;		/* not far enough down yet*/
		if (pCurRect->top > ry) 
		{
		   partOut = TRUE;	/* missed part of rectangle above */
		   if (partIn || (pCurRect->top >= rect->bottom))
			  break;
		   ry = pCurRect->top;	/* x guaranteed to be == rect->left */
		}

		if (pCurRect->right <= rx) continue;		/* not far enough over yet */
		if (pCurRect->left > rx) 
		{
		   partOut = TRUE;	/* missed part of rectangle to left */
		   if (partIn) break;
		}

		if (pCurRect->left < rect->right) {
			partIn = TRUE;	/* definitely overlap */
			if (partOut) break;
		}

		if (pCurRect->right >= rect->right) {
		   ry = pCurRect->bottom;	/* finished with this band */
		   if (ry >= rect->bottom) break;
		   rx = rect->left;	/* reset x out to left again */
		} else {
			break;
		}
    }

    return(partIn ? ((ry < rect->bottom) ? RECT_PARTIN : RECT_ALLIN) : RECT_OUT);
}
