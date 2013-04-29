#include "windows.h"
#include "screens.h"
#define SysBase IBase->ib_SysBase

void _RedrawScreen(IntuitionBase *IBase, struct nScreen *screen)
{
	struct nWindow *rWindow;
	if (screen == NULL)
	{
		rWindow = &IBase->ib_ActiveScreen->root;
	} else
	{
		rWindow = &screen->root;
	}
	DPrintF("ExposeArea %d, %d, %d, %d\n", 0, 0, rWindow->width, rWindow->height);
	_ExposeArea(IBase, rWindow, 0, 0, rWindow->width, rWindow->height, NULL);
}
