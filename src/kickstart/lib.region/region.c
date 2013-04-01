#include "regionbase.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

typedef void (*REGION_OverlapBandFunctionPtr) (RegionBase *RegionBase, ClipRegion * pReg, Rect * r1, Rect * r1End, Rect * r2, Rect * r2End, INT32 top, INT32 bottom);
typedef void (*REGION_NonOverlapBandFunctionPtr) (RegionBase *RegionBase, ClipRegion * pReg, Rect * r, Rect * end, INT32 top, INT32 bottom);


static void REGION_SetExtents (RegionBase *RegionBase, ClipRegion *pReg)
{
    Rect *pRect, *pRectEnd, *pExtents;

    if (pReg->numRects == 0)
    {
		pReg->extents.left = 0;
		pReg->extents.top = 0;
		pReg->extents.right = 0;
		pReg->extents.bottom = 0;
		return;
    }

    pExtents = &pReg->extents;
    pRect = pReg->rects;
    pRectEnd = &pRect[pReg->numRects - 1];

    /*
     * Since pRect is the first rectangle in the region, it must have the
     * smallest top and since pRectEnd is the last rectangle in the region,
     * it must have the largest bottom, because of banding. Initialize left and
     * right from pRect and pRectEnd, resp., as good things to initialize them
     * to...
     */
    pExtents->left = pRect->left;
    pExtents->top = pRect->top;
    pExtents->right = pRectEnd->right;
    pExtents->bottom = pRectEnd->bottom;

    while (pRect <= pRectEnd)
    {
		if (pRect->left < pExtents->left) pExtents->left = pRect->left;
		if (pRect->right > pExtents->right) pExtents->right = pRect->right;
		pRect++;
    }
}


/* *********************************************************************
 *           REGION_Coalesce
 *
 *      Attempt to merge the rects in the current band with those in the
 *      previous one. Used only by REGION_RegionOp.
 *
 * Results:
 *      The new index for the previous band.
 *
 * Side Effects:
 *      If coalescing takes place:
 *          - rectangles in the previous band will have their bottom fields
 *            altered.
 *          - pReg->numRects will be decreased.
 *
 */
static INT32 REGION_Coalesce (RegionBase *RegionBase, 
	     ClipRegion *pReg, /* Region to coalesce */
	     INT32 prevStart,  /* Index of start of previous band */
	     INT32 curStart    /* Index of start of current band */
) {
    Rect *pPrevRect;          /* Current rect in previous band */
    Rect *pCurRect;           /* Current rect in current band */
    Rect *pRegEnd;            /* End of region */
    INT32 curNumRects;          /* Number of rectangles in current band */
    INT32 prevNumRects;         /* Number of rectangles in previous band */
    INT32 bandtop;               /* top coordinate for current band */

    pRegEnd = &pReg->rects[pReg->numRects];

    pPrevRect = &pReg->rects[prevStart];
    prevNumRects = curStart - prevStart;

    /*
     * Figure out how many rectangles are in the current band. Have to do
     * this because multiple bands could have been added in REGION_RegionOp
     * at the end when one region has been exhausted.
     */
    pCurRect = &pReg->rects[curStart];
    bandtop = pCurRect->top;
    for (curNumRects = 0; (pCurRect != pRegEnd) && (pCurRect->top == bandtop); curNumRects++)
    {
		pCurRect++;
    }
    
    if (pCurRect != pRegEnd)
    {
		/*
		 * If more than one band was added, we have to find the start
		 * of the last band added so the next coalescing job can start
		 * at the right place... (given when multiple bands are added,
		 * this may be pointless -- see above).
		 */
		pRegEnd--;
		while (pRegEnd[-1].top == pRegEnd->top)
		{
			pRegEnd--;
		}
		curStart = pRegEnd - pReg->rects;
		pRegEnd = pReg->rects + pReg->numRects;
    }
	
    if ((curNumRects == prevNumRects) && (curNumRects != 0)) 
    {
		pCurRect -= curNumRects;
		/*
		 * The bands may only be coalesced if the bottom of the previous
		 * matches the top scanline of the current.
		 */
		if (pPrevRect->bottom == pCurRect->top)
		{
	    /*
	     * Make sure the bands have rects in the same places. This
	     * assumes that rects have been added in such a way that they
	     * cover the most area possible. I.e. two rects in a band must
	     * have some horizontal space between them.
	     */
			do
			{
				if ((pPrevRect->left != pCurRect->left) ||
					(pPrevRect->right != pCurRect->right))
				{
					/*
					 * The bands don't line up so they can't be coalesced.
					 */
					return (curStart);
				}
				pPrevRect++;
				pCurRect++;
				prevNumRects -= 1;
			} while (prevNumRects != 0);

			pReg->numRects -= curNumRects;
			pCurRect -= curNumRects;
			pPrevRect -= curNumRects;

			/*
			 * The bands may be merged, so set the bottom of each rect
			 * in the previous band to that of the corresponding rect in
			 * the current band.
			 */
			do
			{
				pPrevRect->bottom = pCurRect->bottom;
				pPrevRect++;
				pCurRect++;
				curNumRects -= 1;
			} while (curNumRects != 0);

			/*
			 * If only one band was added to the region, we have to backup
			 * curStart to the start of the previous band.
			 *
			 * If more than one band was added to the region, copy the
			 * other bands down. The assumption here is that the other bands
			 * came from the same region as the current one and no further
			 * coalescing can be done on them since it's all been done
			 * already... curStart is already in the right place.
			 */
			if (pCurRect == pRegEnd)
			{
				curStart = prevStart;
			} else {
				do
				{
					*pPrevRect++ = *pCurRect++;
				} while (pCurRect != pRegEnd);
			}
		}
    }
    return (curStart);
}

/* *********************************************************************
 *           REGION_RegionOp
 *
 *      Apply an operation to two regions. Called by GdUnion,
 *      GdXor, GdSubtract, GdIntersect...
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      The new region is overwritten.
 *
 * Notes:
 *      The idea behind this function is to view the two regions as sets.
 *      Together they cover a rectangle of area that this function divides
 *      into horizontal bands where points are covered only by one region
 *      or by both. For the first case, the nonOverlapFunc is called with
 *      each the band and the band's upper and lower extents. For the
 *      second, the overlapFunc is called to process the entire band. It
 *      is responsible for clipping the rectangles in the band, though
 *      this function provides the boundaries.
 *      At the end of each band, the new region is coalesced, if possible,
 *      to reduce the number of rectangles in the region.
 *
 */
static void REGION_RegionOp(
			RegionBase *RegionBase, 
			ClipRegion *newReg, /* Place to store result */
			ClipRegion *reg1,   /* First region in operation */
			ClipRegion *reg2,   /* 2nd region in operation */
			REGION_OverlapBandFunctionPtr overlapFunc,     /* Function to call for over-lapping bands */
			REGION_NonOverlapBandFunctionPtr nonOverlap1Func, /* Function to call for non-overlapping bands in region 1 */
			REGION_NonOverlapBandFunctionPtr nonOverlap2Func  /* Function to call for non-overlapping bands in region 2 */
) {
    Rect *r1;                         /* Pointer into first region */
    Rect *r2;                         /* Pointer into 2d region */
    Rect *r1End;                      /* End of 1st region */
    Rect *r2End;                      /* End of 2d region */
    INT32 ybot;                         /* Bottom of intersection */
    INT32 ytop;                         /* Top of intersection */
    Rect *oldRects;                   /* Old rects for newReg */
    INT32 prevBand;                     /* Index of start of
						 * previous band in newReg */
    INT32 curBand;                      /* Index of start of current
						 * band in newReg */
    Rect *r1BandEnd;                  /* End of current band in r1 */
    Rect *r2BandEnd;                  /* End of current band in r2 */
    INT32 top;                          /* Top of non-overlapping band */
    INT32 bot;                          /* Bottom of non-overlapping band */
    
    /*
     * Initialization:
     *  set r1, r2, r1End and r2End appropriately, preserve the important
     * parts of the destination region until the end in case it's one of
     * the two source regions, then mark the "new" region empty, allocating
     * another array of rectangles for it to use.
     */
    r1 = reg1->rects;
    r2 = reg2->rects;
    r1End = r1 + reg1->numRects;
    r2End = r2 + reg2->numRects;
    

    /*
     * newReg may be one of the src regions so we can't empty it. We keep a 
     * note of its rects pointer (so that we can free them later), preserve its
     * extents and simply set numRects to zero. 
     */

    oldRects = newReg->rects;
    newReg->numRects = 0;

    /*
     * Allocate a reasonable number of rectangles for the new region. The idea
     * is to allocate enough so the individual functions don't need to
     * reallocate and copy the array, which is time consuming, yet we don't
     * have to worry about using too much memory. I hope to be able to
     * nuke the GdRealloc() at the end of this function eventually.
     */
    newReg->size = MAX(reg1->numRects,reg2->numRects) * 2;

    if (! (newReg->rects = AllocVec(( sizeof(Rect) * newReg->size ), MEMF_CLEAR|MEMF_FAST)))
    {
	newReg->size = 0;
	return;
    }
    
    /*
     * Initialize ybot and ytop.
     * In the upcoming loop, ybot and ytop serve different functions depending
     * on whether the band being handled is an overlapping or non-overlapping
     * band.
     *  In the case of a non-overlapping band (only one of the regions
     * has points in the band), ybot is the bottom of the most recent
     * intersection and thus clips the top of the rectangles in that band.
     * ytop is the top of the next intersection between the two regions and
     * serves to clip the bottom of the rectangles in the current band.
     *  For an overlapping band (where the two regions intersect), ytop clips
     * the top of the rectangles of both regions and ybot clips the bottoms.
     */
    if (reg1->extents.top < reg2->extents.top) ybot = reg1->extents.top;
    else ybot = reg2->extents.top;
    
    /*
     * prevBand serves to mark the start of the previous band so rectangles
     * can be coalesced into larger rectangles. qv. miCoalesce, above.
     * In the beginning, there is no previous band, so prevBand == curBand
     * (curBand is set later on, of course, but the first band will always
     * start at index 0). prevBand and curBand must be indices because of
     * the possible expansion, and resultant moving, of the new region's
     * array of rectangles.
     */
    prevBand = 0;
    
    do
    {
		curBand = newReg->numRects;

		/*
		 * This algorithm proceeds one source-band (as opposed to a
		 * destination band, which is determined by where the two regions
		 * intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
		 * rectangle after the last one in the current band for their
		 * respective regions.
		 */
		r1BandEnd = r1;
		while ((r1BandEnd != r1End) && (r1BandEnd->top == r1->top))
		{
			r1BandEnd++;
		}
	
		r2BandEnd = r2;
		while ((r2BandEnd != r2End) && (r2BandEnd->top == r2->top))
		{
			r2BandEnd++;
		}
	
		/*
		 * First handle the band that doesn't intersect, if any.
		 *
		 * Note that attention is restricted to one band in the
		 * non-intersecting region at once, so if a region has n
		 * bands between the current position and the next place it overlaps
		 * the other, this entire loop will be passed through n times.
		 */
		if (r1->top < r2->top)
		{
			top = MAX(r1->top,ybot);
			bot = MIN(r1->bottom,r2->top);

			if ((top != bot) && (nonOverlap1Func != (void (*)())NULL))
			{
			(* nonOverlap1Func) (RegionBase, newReg, r1, r1BandEnd, top, bot);
			}

			ytop = r2->top;
		}
		else if (r2->top < r1->top)
		{
			top = MAX(r2->top,ybot);
			bot = MIN(r2->bottom,r1->top);

			if ((top != bot) && (nonOverlap2Func != (void (*)())NULL))
			{
			(* nonOverlap2Func) (RegionBase, newReg, r2, r2BandEnd, top, bot);
			}

			ytop = r1->top;
		}
		else
		{
			ytop = r1->top;
		}

		/*
		 * If any rectangles got added to the region, try and coalesce them
		 * with rectangles from the previous band. Note we could just do
		 * this test in miCoalesce, but some machines incur a not
		 * inconsiderable cost for function calls, so...
		 */
		if (newReg->numRects != curBand)
		{
			prevBand = REGION_Coalesce (RegionBase, newReg, prevBand, curBand);
		}

		/*
		 * Now see if we've hit an intersecting band. The two bands only
		 * intersect if ybot > ytop
		 */
		ybot = MIN(r1->bottom, r2->bottom);
		curBand = newReg->numRects;
		if (ybot > ytop)
		{
			(* overlapFunc) (RegionBase, newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);

		}
		
		if (newReg->numRects != curBand)
		{
			prevBand = REGION_Coalesce (RegionBase, newReg, prevBand, curBand);
		}

		/*
		 * If we've finished with a band (bottom == ybot) we skip forward
		 * in the region to the next band.
		 */
		if (r1->bottom == ybot)
		{
			r1 = r1BandEnd;
		}
		if (r2->bottom == ybot)
		{
			r2 = r2BandEnd;
		}
	} while ((r1 != r1End) && (r2 != r2End));

	/*
	 * Deal with whichever region still has rectangles left.
	 */
	curBand = newReg->numRects;
	if (r1 != r1End)
    {
		if (nonOverlap1Func != (void (*)())NULL)
		{
			do
			{
				r1BandEnd = r1;
				while ((r1BandEnd < r1End) && (r1BandEnd->top == r1->top))
				{
					r1BandEnd++;
				}
				(* nonOverlap1Func) (RegionBase, newReg, r1, r1BandEnd, MAX(r1->top,ybot), r1->bottom);
				r1 = r1BandEnd;
			} while (r1 != r1End);
		}
    } else if ((r2 != r2End) && (nonOverlap2Func != (void (*)())NULL))
    {
		do
		{
			r2BandEnd = r2;
			while ((r2BandEnd < r2End) && (r2BandEnd->top == r2->top))
			{
			 r2BandEnd++;
			}
			(* nonOverlap2Func) (RegionBase, newReg, r2, r2BandEnd, MAX(r2->top,ybot), r2->bottom);
			r2 = r2BandEnd;
		} while (r2 != r2End);
    }

    if (newReg->numRects != curBand)
    {
		(void) REGION_Coalesce (RegionBase, newReg, prevBand, curBand);
    }

    /*
     * A bit of cleanup. To keep regions from growing without bound,
     * we shrink the array of rectangles to match the new number of
     * rectangles in the region. This never goes to 0, however...
     *
     * Only do this stuff if the number of rectangles allocated is more than
     * twice the number of rectangles in the region (a simple optimization...).
     */
    if (newReg->numRects < (newReg->size >> 1))
    {
	if (REGION_NOT_EMPTY(newReg))
	{
	    Rect *prev_rects = newReg->rects;
	    newReg->rects = Realloc( newReg->rects, sizeof(Rect) * newReg->size, sizeof(Rect) * newReg->numRects );
	    newReg->size = newReg->numRects;
	    if (! newReg->rects)
		newReg->rects = prev_rects;
	}
	else
	{
	    /*
	     * No point in doing the extra work involved in an Xrealloc if
	     * the region is empty
	     */
	    newReg->size = 1;
	    FreeVec( newReg->rects );
	    newReg->rects = AllocVec( sizeof(Rect), MEMF_FAST|MEMF_CLEAR );
	}
    }
    FreeVec( oldRects );
}

/* *********************************************************************
 *          Region Intersection
 ***********************************************************************/


/* *********************************************************************
 *	     REGION_IntersectO
 *
 * Handle an overlapping band for REGION_Intersect.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      Rectangles may be added to the region.
 *
 */
static void REGION_IntersectO(RegionBase *RegionBase, ClipRegion *pReg,  Rect *r1, Rect *r1End,
			Rect *r2, Rect *r2End, INT32 top, INT32 bottom)

{
    INT32       left, right;
    Rect      *pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    while ((r1 != r1End) && (r2 != r2End))
    {
	left = MAX(r1->left, r2->left);
	right =	MIN(r1->right, r2->right);

	/*
	 * If there's any overlap between the two rectangles, add that
	 * overlap to the new region.
	 * There's no need to check for subsumption because the only way
	 * such a need could arise is if some region has two rectangles
	 * right next to each other. Since that should never happen...
	 */
	if (left < right)
	{
	    MEMCHECK(pReg, pNextRect, pReg->rects);
	    pNextRect->left = left;
	    pNextRect->top = top;
	    pNextRect->right = right;
	    pNextRect->bottom = bottom;
	    pReg->numRects += 1;
	    pNextRect++;
	}

	/*
	 * Need to advance the pointers. Shift the one that extends
	 * to the right the least, since the other still has a chance to
	 * overlap with that region's next rectangle, if you see what I mean.
	 */
	if (r1->right < r2->right)
	{
	    r1++;
	}
	else if (r2->right < r1->right)
	{
	    r2++;
	}
	else
	{
	    r1++;
	    r2++;
	}
    }
}

/**
 * Finds the intersection of two regions - i.e. the places where
 * they overlap.
 *
 * @param newReg Output region.  May be one of the source regions.
 * @param reg1 Source region.
 * @param reg2 Source region.
 */
void reg_IntersectRegion(RegionBase *RegionBase, ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2)
{
   /* check for trivial reject */
    if ( (!(reg1->numRects)) || (!(reg2->numRects))  || (!EXTENTCHECK(&reg1->extents, &reg2->extents)))
		newReg->numRects = 0;
    else
		REGION_RegionOp (RegionBase, newReg, reg1, reg2,  REGION_IntersectO, NULL, NULL);
    
    /*
     * Can't alter newReg's extents before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the extents of those regions being the same. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    REGION_SetExtents(RegionBase, newReg);
    newReg->type = (newReg->numRects) ? REGION_COMPLEX : REGION_NULL ;
}

/* *********************************************************************
 *	     Region Union
 ***********************************************************************/

/* *********************************************************************
 *	     REGION_UnionNonO
 *
 *      Handle a non-overlapping band for the union operation. Just
 *      Adds the rectangles into the region. Doesn't have to check for
 *      subsumption or anything.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      pReg->numRects is incremented and the final rectangles overwritten
 *      with the rectangles we're passed.
 *
 */
static void REGION_UnionNonO(RegionBase *RegionBase, ClipRegion *pReg,Rect *r,Rect *rEnd,INT32 top, INT32 bottom)
{
    Rect *pNextRect;

    pNextRect = &pReg->rects[pReg->numRects];

    while (r != rEnd)
    {
	MEMCHECK(pReg, pNextRect, pReg->rects);
	pNextRect->left = r->left;
	pNextRect->top = top;
	pNextRect->right = r->right;
	pNextRect->bottom = bottom;
	pReg->numRects += 1;
	pNextRect++;
	r++;
    }
}

/* *********************************************************************
 *	     REGION_UnionO
 *
 *      Handle an overlapping band for the union operation. Picks the
 *      left-most rectangle each time and merges it into the region.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      Rectangles are overwritten in pReg->rects and pReg->numRects will
 *      be changed.
 *
 */
 #define MERGERECT(r) \
    if ((pReg->numRects != 0) &&  \
	(pNextRect[-1].top == top) &&  \
	(pNextRect[-1].bottom == bottom) &&  \
	(pNextRect[-1].right >= r->left))  \
    {  \
	if (pNextRect[-1].right < r->right)  \
	{  \
	    pNextRect[-1].right = r->right;  \
	}  \
    }  \
    else  \
    {  \
	MEMCHECK(pReg, pNextRect, pReg->rects);  \
	pNextRect->top = top;  \
	pNextRect->bottom = bottom;  \
	pNextRect->left = r->left;  \
	pNextRect->right = r->right;  \
	pReg->numRects += 1;  \
	pNextRect += 1;  \
    }  \
    r++;

static void REGION_UnionO(RegionBase *RegionBase, ClipRegion *pReg, Rect *r1, Rect *r1End, Rect *r2, Rect *r2End, INT32 top, INT32 bottom)
{
    Rect *pNextRect;
    
    pNextRect = &pReg->rects[pReg->numRects];
    
    while ((r1 != r1End) && (r2 != r2End))
    {
	if (r1->left < r2->left)
	{
	    MERGERECT(r1);
	}
	else
	{
	    MERGERECT(r2);
	}
    }
    
    if (r1 != r1End)
    {
	do
	{
	    MERGERECT(r1);
	} while (r1 != r1End);
    }
    else while (r2 != r2End)
    {
	MERGERECT(r2);
    }
}

/**
 * Finds the union of two regions - i.e. the places which are
 * in either reg1 or reg2 or both.
 *
 * @param newReg Output region.  May be one of the source regions.
 * @param reg1 Source region.
 * @param reg2 Source region.
 */
void reg_UnionRegion(RegionBase *RegionBase, ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2)
{
    /*  checks all the simple cases */

    /*
     * Region 1 and 2 are the same or region 1 is empty
     */
    if ( (reg1 == reg2) || (!(reg1->numRects)) )
    {
		if (newReg != reg2) CopyRegion(newReg, reg2);
		return;
    }

    /*
     * if nothing to union (region 2 empty)
     */
    if (!(reg2->numRects))
    {
		if (newReg != reg1) CopyRegion(newReg, reg1);
		return;
    }

    /*
     * Region 1 completely subsumes region 2
     */
    if ((reg1->numRects == 1) && 
	(reg1->extents.left <= reg2->extents.left) &&
	(reg1->extents.top <= reg2->extents.top) &&
	(reg1->extents.right >= reg2->extents.right) &&
	(reg1->extents.bottom >= reg2->extents.bottom))
    {
		if (newReg != reg1) CopyRegion(newReg, reg1);
		return;
    }

    /*
     * Region 2 completely subsumes region 1
     */
    if ((reg2->numRects == 1) && 
	(reg2->extents.left <= reg1->extents.left) &&
	(reg2->extents.top <= reg1->extents.top) &&
	(reg2->extents.right >= reg1->extents.right) &&
	(reg2->extents.bottom >= reg1->extents.bottom))
    {
		if (newReg != reg2)	CopyRegion(newReg, reg2);
		return;
    }

	REGION_RegionOp (RegionBase, newReg, reg1, reg2, REGION_UnionO, REGION_UnionNonO, REGION_UnionNonO);

    newReg->extents.left = MIN(reg1->extents.left, reg2->extents.left);
    newReg->extents.top = MIN(reg1->extents.top, reg2->extents.top);
    newReg->extents.right = MAX(reg1->extents.right, reg2->extents.right);
    newReg->extents.bottom = MAX(reg1->extents.bottom, reg2->extents.bottom);
    newReg->type = (newReg->numRects) ? REGION_COMPLEX : REGION_NULL ;
}

/* *********************************************************************
 *	     Region Subtraction
 ***********************************************************************/

/* *********************************************************************
 *	     REGION_SubtractNonO1
 *
 *      Deal with non-overlapping band for subtraction. Any parts from
 *      region 2 we discard. Anything from region 1 we add to the region.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      pReg may be affected.
 *
 */
static void REGION_SubtractNonO1(RegionBase *RegionBase, ClipRegion *pReg, Rect *r, Rect *rEnd, INT32 top, INT32 bottom)
{
    Rect *pNextRect;
	
    pNextRect = &pReg->rects[pReg->numRects];
	
    while (r != rEnd)
    {
	MEMCHECK(pReg, pNextRect, pReg->rects);
	pNextRect->left = r->left;
	pNextRect->top = top;
	pNextRect->right = r->right;
	pNextRect->bottom = bottom;
	pReg->numRects += 1;
	pNextRect++;
	r++;
    }
}


/* *********************************************************************
 *	     REGION_SubtractO
 *
 *      Overlapping band subtraction. x1 is the left-most point not yet
 *      checked.
 *
 * Results:
 *      None.
 *
 * Side Effects:
 *      pReg may have rectangles added to it.
 *
 */
static void REGION_SubtractO(RegionBase *RegionBase, ClipRegion *pReg, Rect *r1, Rect *r1End, Rect *r2, Rect *r2End, INT32 top, INT32 bottom)
{
    Rect *pNextRect;
    INT32 left;
    
    left = r1->left;
    pNextRect = &pReg->rects[pReg->numRects];

    while ((r1 != r1End) && (r2 != r2End))
    {
	if (r2->right <= left)
	{
	    /*
	     * Subtrahend missed the boat: go to next subtrahend.
	     */
	    r2++;
	}
	else if (r2->left <= left)
	{
	    /*
	     * Subtrahend preceeds minuend: nuke left edge of minuend.
	     */
	    left = r2->right;
	    if (left >= r1->right)
	    {
		/*
		 * Minuend completely covered: advance to next minuend and
		 * reset left fence to edge of new minuend.
		 */
		r1++;
		if (r1 != r1End)
		    left = r1->left;
	    }
	    else
	    {
		/*
		 * Subtrahend now used up since it doesn't extend beyond
		 * minuend
		 */
		r2++;
	    }
	}
	else if (r2->left < r1->right)
	{
	    /*
	     * Left part of subtrahend covers part of minuend: add uncovered
	     * part of minuend to region and skip to next subtrahend.
	     */
	    MEMCHECK(pReg, pNextRect, pReg->rects);
	    pNextRect->left = left;
	    pNextRect->top = top;
	    pNextRect->right = r2->left;
	    pNextRect->bottom = bottom;
	    pReg->numRects += 1;
	    pNextRect++;
	    left = r2->right;
	    if (left >= r1->right)
	    {
		/*
		 * Minuend used up: advance to new...
		 */
		r1++;
		if (r1 != r1End)
		    left = r1->left;
	    }
	    else
	    {
		/*
		 * Subtrahend used up
		 */
		r2++;
	    }
	}
	else
	{
	    /*
	     * Minuend used up: add any remaining piece before advancing.
	     */
	    if (r1->right > left)
	    {
		MEMCHECK(pReg, pNextRect, pReg->rects);
		pNextRect->left = left;
		pNextRect->top = top;
		pNextRect->right = r1->right;
		pNextRect->bottom = bottom;
		pReg->numRects += 1;
		pNextRect++;
	    }
	    r1++;
	    left = r1->left;
	}
    }

    /*
     * Add remaining minuend rectangles to region.
     */
    while (r1 != r1End)
    {
	MEMCHECK(pReg, pNextRect, pReg->rects);
	pNextRect->left = left;
	pNextRect->top = top;
	pNextRect->right = r1->right;
	pNextRect->bottom = bottom;
	pReg->numRects += 1;
	pNextRect++;
	r1++;
	if (r1 != r1End)
	{
	    left = r1->left;
	}
    }
}
	
/**
 * Finds the difference of two regions (regM - regS) - i.e. the
 * places which are in regM but not regS.
 *
 * @param regD Output (Difference) region.  May be one of the source regions.
 * @param regM Source (Minuend) region - the region to subtract from.
 * @param regS Source (Subtrahend) region - the region we subtract.
 */
void reg_SubtractRegion(RegionBase *RegionBase, ClipRegion *regD, ClipRegion *regM, ClipRegion *regS )
{
   /* check for trivial reject */
    if 	( (!(regM->numRects)) || (!(regS->numRects))  ||
		(!EXTENTCHECK(&regM->extents, &regS->extents)) )
    {
		CopyRegion(regD, regM);
		return;
    }
 
    REGION_RegionOp (RegionBase, regD, regM, regS, REGION_SubtractO, REGION_SubtractNonO1, NULL);

    /*
     * Can't alter newReg's extents before we call miRegionOp because
     * it might be one of the source regions and miRegionOp depends
     * on the extents of those regions being the unaltered. Besides, this
     * way there's no checking against rectangles that will be nuked
     * due to coalescing, so we have to examine fewer rectangles.
     */
    REGION_SetExtents (RegionBase, regD);
    regD->type = (regD->numRects) ? REGION_COMPLEX : REGION_NULL ;
}
