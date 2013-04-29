#include "windows.h"
#include "screens.h"
#define SysBase IBase->ib_SysBase

void _ExposeArea(IntuitionBase *IBase, struct nWindow *wp, INT32 rootx, INT32 rooty, INT32 width, INT32 height, struct nWindow *stopwp)
{
	if (!wp->realized || wp == stopwp) return;

	if ((rootx >= wp->x + wp->width + wp->bordersize) ||
		(rooty >= wp->y + wp->height + wp->bordersize) ||
		(rootx + width <= wp->x - wp->bordersize) ||
		(rooty + height <= wp->y - wp->bordersize))
			return;

//	DPrintF("DrawBorder\n");
	if (rootx < wp->x || rooty < wp->y ||
		(rootx + width > wp->x + wp->width) ||
		(rooty + height > wp->y + wp->height)) _DrawBorder(IBase, wp);

//	DPrintF("ClearWindow\n");
	_ClearWindow(IBase, wp, rootx - wp->x, rooty - wp->y, width, height, 1);

	for (wp = wp->children; wp; wp = wp->siblings) _ExposeArea(IBase, wp, rootx, rooty, width, height, stopwp);
}
