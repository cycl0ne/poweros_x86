#include "windows.h"
#include "screens.h"
#define SysBase IBase->ib_SysBase

void _RedrawScreen(IntuitionBase *IBase, struct Screen *screen)
{
	struct Window *rWindow;
	if (screen == NULL)
	{
		rWindow = &IBase->ib_ActiveScreen->RootWindow;
	} else
	{
		rWindow = &screen->RootWindow;
	}
	_ExposeArea(IBase, rWindow, 0, 0, rWindow->width, rWindow->height, NULL);
}
