#include "windows.h"
#include "screens.h"

void _MapWindow(IntuitionBase *IBase, struct nWindow *wp)
{
//	SERVER_LOCK();

	if (!wp || wp->mapped) {
//		SERVER_UNLOCK();
		return;
	}

	wp->mapped = TRUE;

	_RealizeWindow(IBase, wp, FALSE);

//	SERVER_UNLOCK();
}
