#include "intuitionbase.h"
#include "list.h"
#include "pixmap.h"
#include "view.h"
#include "font.h"
#include "regions.h"
#include "coregfx.h"
#include "windows.h"

#include "exec_funcs.h"
#include "utility_funcs.h"

#define SysBase IBase->ib_SysBase
#define UtilBase IBase->ib_UtilBase

static void OffsetWindow(struct nWindow *wp, INT32 offx, INT32 offy)
{
	struct nWindow *cp;

	wp->x += offx;
	wp->y += offy;
	for(cp=wp->children; cp; cp=cp->siblings) OffsetWindow(cp, offx, offy);
}

static int IsUnobscuredBySiblings(struct nWindow *wp)
{
	struct nWindow *current, *parent, *child;

	for (current=wp; current; current=parent) 
	{
		parent = current->parent;
		if (!parent) break;
		for (child=parent->children; child; child=child->siblings) 
		{
			if (child == current) break;
			else if (_CheckOverlap(child, wp)) return 0;
		}
	}
	return 1;
}

void intu_MoveWindow(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y)
{
	struct nWindow	*parent;
	INT32			offx, offy;
	
	if (wp == NULL) return;
	if (wp == &wp->screen->root) return;
	//SERVER_LOCK();

	parent = wp->parent;
	x += parent->x;
	y += parent->y;
	offx = x - wp->x;
	offy = y - wp->y;

	if (wp->x == x && wp->y == y) 
	{
		//SERVER_UNLOCK();
		return;
	}
	if (wp->mapped && IsUnobscuredBySiblings(wp) && !wp->clipregion) 
	{
#if 0
		int 		oldx = wp->x;
		int 		oldy = wp->y;
		GR_GC_ID	gc = GrNewGC();
		GR_WINDOW * 	stopwp = wp;
		int		X, Y, W, H;

		/* must hide cursor first or GdFixCursor() will show it*/
		HideCursor(rootwp->psd);

		/* turn off clipping of root's children*/
		GrSetGCMode(gc, GR_MODE_COPY|GR_MODE_EXCLUDECHILDREN);

		/* calc new window offsets*/
		OffsetWindow(wp, offx, offy);

		/* force recalc of clip region*/
		clipwp = NULL;

		/* copy window bits to new location*/
		GrCopyArea(parent->id, gc, wp->x - parent->x, wp->y - parent->y,
			wp->width, wp->height, parent->id,
			oldx - parent->x, oldy - parent->y, MWROP_COPY);

		/*
		 * If any portion of the window was offscreen
		 * and is coming onscreen, must send expose events
		 * to this window as well.
		 */
		if ((oldx < 0 && wp->x > oldx) ||
		    (oldy < 0 && wp->y > oldy) ||
		    (oldx+wp->width > rootwp->width && wp->x < oldx) ||
		    (oldy+wp->height > rootwp->height && wp->y < oldy))
			stopwp = NULL;

		/* 
		 * Calculate bounded exposed area and
		 * redraw anything lower than stopwp window.
		 */
		X = MWMIN(oldx, wp->x);
		Y = MWMIN(oldy, wp->y);
		W = MWMAX(oldx, wp->x) + wp->width - X;
		H = MWMAX(oldy, wp->y) + wp->height - Y;
		GsExposeArea(rootwp, X, Y, W, H, stopwp);

		GdShowCursor(rootwp->psd);
		GrDestroyGC(gc);
		DeliverUpdateMoveEventAndChildren(wp);
		SERVER_UNLOCK();
		return;
#endif
	}
	/*
	 * This method will redraw the window entirely,
	 * resulting in considerable flicker.
	 */
	_UnrealizeWindow(IBase, wp, TRUE);
	OffsetWindow(wp, offx, offy);
	_RealizeWindow(IBase, wp, FALSE);
//	DeliverUpdateMoveEventAndChildren(wp);

//	SERVER_UNLOCK();
	return;
}

void intu_WindowToFront(IntuitionBase *IBase, struct nWindow *wp)
{
	struct nWindow 	*prevwp;
	BOOL			overlap;

	if (wp == NULL) return;
	if (wp == &wp->screen->root) return;

	// SERVERLOCK();
	prevwp = wp->parent->children;
	if (prevwp == wp) {
		//SERVER_UNLOCK();
		return;
	}

	overlap = FALSE;
	while (prevwp->siblings != wp) 
	{
		overlap |= _CheckOverlap(prevwp, wp);
		prevwp = prevwp->siblings;
	}
	overlap |= _CheckOverlap(prevwp, wp);

	prevwp->siblings = wp->siblings;
	wp->siblings = wp->parent->children;
	wp->parent->children = wp;

	if (overlap) {
		_DrawBorder(IBase, wp);
		_ExposeArea(IBase, wp, wp->x, wp->y, wp->width, wp->height, NULL);
	}
	//SERVER_UNLOCK();
}

void intu_WindowToBack(IntuitionBase *IBase, struct nWindow *wp)
{
	struct nWindow 	*prevwp;
	struct nWindow	*sibwp;
	struct nWindow	*expwp;

	if (wp == NULL) return;
	if (wp == &wp->screen->root) return;

	// SERVERLOCK();
	if (wp->siblings == NULL) 
	{
		//SERVER_UNLOCK();
		return;
	}
	
	prevwp = wp->parent->children;
	if (prevwp != wp) {
		while (prevwp->siblings != wp)
			prevwp = prevwp->siblings;
	}
	
	expwp = wp->siblings;
	sibwp = wp;
	while (sibwp->siblings) sibwp = sibwp->siblings;

	if (prevwp == wp)	wp->parent->children = wp->siblings;
	else 				prevwp->siblings = wp->siblings;

	sibwp->siblings = wp;
	wp->siblings = NULL;

	while (expwp && (expwp != wp)) 
	{
		if (_CheckOverlap(wp, expwp)) _ExposeArea(IBase, expwp, wp->x - wp->bordersize, wp->y - wp->bordersize, wp->width + wp->bordersize * 2, wp->height + wp->bordersize * 2, NULL);
		expwp = expwp->siblings;
	}

	//SERVER_UNLOCK();
}

void intu_SizeWindow(IntuitionBase *IBase, struct nWindow *wp, INT32 width, INT32 height)
{
	INT32	oldw, oldh;
	//SERVER_LOCK();

	if (wp == NULL) 
	{
		//SERVER_UNLOCK();
		return;
	}
	if (wp == &wp->screen->root) 
	{
		//GsError(GR_ERROR_ILLEGAL_ON_ROOT_WINDOW, wid);
		//SERVER_UNLOCK();
		return;
	}
	if (width <= 0 || height <= 0) 
	{
		//GsError(GR_ERROR_BAD_WINDOW_SIZE, wid);
		//SERVER_UNLOCK();
		return;
	}

	if (wp->width == width && wp->height == height) 
	{
		//SERVER_UNLOCK();
		return;
	}

	/* possibly reallocate buffered window's pixmap to new size*/
//	if (wp->props & GR_WM_PROPS_BUFFERED) GsInitWindowBuffer(wp, width, height); /* allocate buffer and fill background*/

	if (!wp->realized) 
	{
		wp->width = width;
		wp->height = height;
		//SERVER_UNLOCK();
		return;
	}

    oldw = wp->width;
	oldh = wp->height;
	wp->width = width;
	wp->height = height;

	_DrawBorder(IBase, wp);
	_ClearWindow(IBase, wp, 0, 0, wp->width, wp->height, 1);

//	GsDeliverUpdateEvent(wp, GR_UPDATE_SIZE, wp->x, wp->y, width, height);

	if (width < oldw || height < oldh) 
	{
		int bs = wp->bordersize;
		int x = wp->x - bs;
		int y = wp->y - bs;
		int w, h;
		if (oldw < width) oldw = width;
		if (oldh < height) oldh = height;
		w = oldw + bs*2;
		h = oldh + bs*2;
		_ExposeArea(IBase, wp->parent, x + wp->width, y, w - wp->width, h, NULL);
		_ExposeArea(IBase, wp->parent, x, y + wp->height, w - (oldw - wp->width), h - wp->height, NULL);
	}
	//SERVER_UNLOCK();
}

