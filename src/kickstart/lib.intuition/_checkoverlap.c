#include "intuitionbase.h"
#include "windows.h"

BOOL _CheckOverlap(struct nWindow *topwp, struct nWindow *botwp)
{
	INT32	minx1;
	INT32	miny1;
	INT32	maxx1;
	INT32	maxy1;
	INT32	minx2;
	INT32	miny2;
	INT32	maxx2;
	INT32	maxy2;
	INT32	bs;

	if (!topwp->realized || !botwp->realized) return FALSE;

	bs = topwp->bordersize;
	minx1 = topwp->x - bs;
	miny1 = topwp->y - bs;
	maxx1 = topwp->x + topwp->width + bs - 1;
	maxy1 = topwp->y + topwp->height + bs - 1;

	bs = botwp->bordersize;
	minx2 = botwp->x - bs;
	miny2 = botwp->y - bs;
	maxx2 = botwp->x + botwp->width + bs - 1;
	maxy2 = botwp->y + botwp->height + bs - 1;

	if (minx1 > maxx2 || minx2 > maxx1 || miny1 > maxy2 || miny2 > maxy1) return FALSE;
	return TRUE;
}
