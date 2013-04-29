#include "windows.h"
#include "screens.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

void _UnrealizeWindow(IntuitionBase *IBase, struct nWindow *wp, BOOL temp_unmap)
{
	struct nWindow	*pwp;
	struct nWindow	*sibwp;
	struct nWindow	*childwp;
	INT32			bs;
	
	if (wp == &wp->screen->root) return;
	if (wp == IBase->clipwp) IBase->clipwp = NULL;
	if (!wp->realized) return;
	wp->realized = FALSE;

	for (childwp = wp->children; childwp; childwp = childwp->siblings) _UnrealizeWindow(IBase, childwp, temp_unmap);

#if 0
	if (!temp_unmap && wp == mousewp) 
	{
		GsCheckMouseWindow();
		GsCheckCursor();
	}

	if (!temp_unmap && wp == focuswp) {
		if (focusfixed) focuswp = rootwp;
		else {
			focusfixed = FALSE;
			GsCheckFocusWindow();
		}
	}

	/* Send unmap update event*/
	GsDeliverUpdateEvent(wp, (temp_unmap? GR_UPDATE_UNMAPTEMP: GR_UPDATE_UNMAP), 0, 0, 0, 0);
#endif
	if (!wp->parent->realized) return;

	bs = wp->bordersize;
	pwp = wp->parent;
	_ClearWindow(IBase, pwp, wp->x - pwp->x - bs, wp->y - pwp->y - bs, wp->width + bs * 2, wp->height + bs * 2, 1);

	sibwp = wp;
	while (sibwp->siblings) 
	{
		sibwp = sibwp->siblings;
		_ExposeArea(IBase, sibwp, wp->x - bs, wp->y - bs, wp->width + bs * 2, wp->height + bs * 2, NULL);
	}
}
