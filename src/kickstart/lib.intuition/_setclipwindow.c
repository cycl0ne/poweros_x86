#include "intuitionbase.h"
#include "regions.h"
#include "region_funcs.h"

#define CoreGfxBase IBase->ib_GfxBase
#define RegionBase 	IBase->ib_RgnBase
#define SysBase 	IBase->ib_SysBase

#define GR_MODE_EXCLUDECHILDREN	0x0100		/* exclude children on clip*/

void _SetClipWindow(pIntuitionBase IBase, struct nWindow *wp, ClipRegion *userregion, INT32 flags)
{
	struct nWindow	*rootwp = &wp->screen->root;
	struct nWindow	*orgwp;		/* original window pointer */
	struct nWindow	*pwp;		/* parent window */
	struct nWindow	*sibwp;		/* sibling windows */
	INT32	minx;		/* minimum clip x coordinate */
	INT32	miny;		/* minimum clip y coordinate */
	INT32	maxx;		/* maximum clip x coordinate */
	INT32	maxy;		/* maximum clip y coordinate */
	INT32	diff;		/* difference in coordinates */
	INT32	bs;			/* border size */
	INT32	x, y, width, height;
	ClipRegion	*vis, *r;

	if (!wp->realized) return;

	IBase->clipwp = wp;

	/*
	 * Start with the rectangle for the complete window.
	 * We will then cut pieces out of it as needed.
	 */
	x = wp->x;
	y = wp->y;
	width = wp->width;
	height = wp->height;

	/*
	 * First walk upwards through all parent windows,
	 * and restrict the visible part of this window to the part
	 * that shows through all of those parent windows.
	 */
	pwp = wp;
	while (pwp != rootwp) 
	{
//DPrintF("pwp: %x, rootwp %x\n", pwp, rootwp);
		pwp = pwp->parent;
//DPrintF("pwp: %x, rootwp %x\n", pwp, rootwp);
//for(;;);
		diff = pwp->x - x;
		if (diff > 0) 
		{
			width -= diff;
			x = pwp->x;
		}
		diff = (pwp->x + pwp->width) - (x + width);
		if (diff < 0) width += diff;
		diff = pwp->y - y;
		if (diff > 0) 
		{
			height -= diff;
			y = pwp->y;
		}
		diff = (pwp->y + pwp->height) - (y + height);
		if (diff < 0) height += diff;
	}

	/*
	 * If the window is completely clipped out of view, then
	 * set the clipping region to indicate that.
	 */
	if (width <= 0 || height <= 0) 
	{
		SetClipRegion(IBase->clipwp->frp, NULL);
		return;
	}

	/*
	 * Allocate region to clipped size of window,
	 * intersect with user window clip region.
	 */
	vis = AllocRectRegion(x, y, x+width, y+height);
	if (wp->clipregion) 
	{
		OffsetRegion(wp->clipregion, wp->x, wp->y);
		IntersectRegion(vis, vis, wp->clipregion);
		OffsetRegion(wp->clipregion, -wp->x, -wp->y);
	}

	/* 
	 * Allocate temp region
	 */
	r = AllocRegion();

	/*
	 * Now examine all windows that obscure this window, and
	 * for each obscuration, break up the clip rectangles into
	 * the smaller pieces that are still visible.  The windows
	 * that can obscure us are the earlier siblings of all of
	 * our parents.
 	 */
	orgwp = wp;
	pwp = wp;
	while (pwp != NULL) 
	{
		wp = pwp;
		pwp = wp->parent;

		if(!pwp) 
		{
			/* We're clipping the root window*/
			if (!(flags & GR_MODE_EXCLUDECHILDREN))
				/* start with root's children*/
				sibwp = rootwp->children;
			else sibwp = NULL;	 /* no search*/
			wp = NULL;		 /* search all root's children*/
		} else {
			sibwp = pwp->children;	 /* clip siblings*/
		}

		for (; sibwp != wp; sibwp = sibwp->siblings) 
		{
			if (!sibwp->realized) continue;

			bs = sibwp->bordersize;
			minx = sibwp->x - bs;
			miny = sibwp->y - bs;
			maxx = sibwp->x + sibwp->width + bs;
			maxy = sibwp->y + sibwp->height + bs;

			if (sibwp->clipregion) 
			{
				ClipRegion *shapeR = AllocRegion();
				SetRectRegion(shapeR, minx, miny, maxx, maxy);
				
				/* FIXME: can user set invalid clipregion here? */
				OffsetRegion(sibwp->clipregion, sibwp->x, sibwp->y);
				IntersectRegion(shapeR, shapeR, sibwp->clipregion);
				OffsetRegion(sibwp->clipregion, -sibwp->x, -sibwp->y);
				
				SubtractRegion(vis, vis, shapeR);
				DestroyRegion(shapeR);
			} else {
				SetRectRegion(r, minx, miny, maxx, maxy);
				SubtractRegion(vis, vis, r);
			}			
		}
		/* if not clipping the root window, stop when you reach it*/
		if (pwp == rootwp) break;
	}

	wp = orgwp;
	/*
	 * If not the root window, clip all children.
	 * (Root window's children are are clipped above)
	 */
	if(wp != rootwp && !(flags & GR_MODE_EXCLUDECHILDREN)) 
	{
		for (sibwp=wp->children; sibwp; sibwp = sibwp->siblings) 
		{
			if (!sibwp->realized) continue;

			bs = sibwp->bordersize;
			minx = sibwp->x - bs;
			miny = sibwp->y - bs;
			maxx = sibwp->x + sibwp->width + bs;
			maxy = sibwp->y + sibwp->height + bs;

			SetRectRegion(r, minx, miny, maxx, maxy);
			SubtractRegion(vis, vis, r);
			/* FIXME: shaped windows with borders won't work */
			if (wp->clipregion) 
			{
				/* FIXME: can user set invalid clipregion here? */
				OffsetRegion(sibwp->clipregion, sibwp->x, sibwp->y);
				SubtractRegion(vis, vis, sibwp->clipregion);
				OffsetRegion(sibwp->clipregion, -sibwp->x, -sibwp->y);
			}
		}
	}

	/*
	 * Intersect with user region, if set.
	 */
	if (userregion) 
	{
		/* temporarily offset region by window coordinates*/
		OffsetRegion(userregion, wp->x, wp->y);
		IntersectRegion(vis, vis, userregion);
		OffsetRegion(userregion, -wp->x, -wp->y);
	}

	/*
	 * Set the clip region (later destroy handled by GdSetClipRegion)
	 */
	SetClipRegion(IBase->clipwp->frp, vis);

	/*
	 * Destroy temp region
	 */
	DestroyRegion(r);
}
