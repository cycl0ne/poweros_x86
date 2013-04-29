#include "windows.h"
#include "screens.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

void _RealizeWindow(IntuitionBase *IBase, struct nWindow *wp, BOOL temp)
{
	struct nWindow *rootwp= &wp->screen->root;
//	DPrintF("wp %x, root: %x\n", wp, rootwp);

	if (wp == rootwp) 
	{
		DPrintF("GR_ERROR_ILLEGAL_ON_ROOT_WINDOW\n");
		return;
	}
	/*printf("RealizeWindow %d, map %d realized %d, parent_realized %d\n", wp->id, wp->mapped, wp->realized, wp->parent->realized);*/

	if (wp->realized) return;

	/* 
	 * Send map update event for window manager or others
	 */
	/* send map update event if not temp unmap/map*/
	if (!temp) {
//		GsDeliverUpdateEvent(wp, GR_UPDATE_MAP, wp->x, wp->y, wp->width, wp->height);
	}

	/* 
	 * If window isn't set to be mapped, or parent isn't
	 * realized, then we're done
	 */
	if (!wp->mapped || !wp->parent->realized) return;

	/* set window visible flag*/
	wp->realized = TRUE;

	if (!temp) {
//		GsCheckMouseWindow();
//		GsCheckFocusWindow();
//		GsCheckCursor();
	}
//DPrintF("realize ->Drawborder\n");
	_DrawBorder(IBase, wp);
//DPrintF("realize ->ClearWindow\n");
	_ClearWindow(IBase, wp, 0, 0, wp->width, wp->height, 1);

	for (wp = wp->children; wp; wp = wp->siblings) _RealizeWindow(IBase, wp, temp);
}
