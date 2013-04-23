#include "intuitionbase.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

static struct Window *NewWindow(IntuitionBase *IBase, struct Screen *screen, struct Window *pwp, INT32 x, INT32 y, INT32 width, INT32 height,
	INT32 bordersize, UINT32 background, UINT32 bordercolor)
{
	Window	*wp;	/* new window*/
	Window	*rootwp = &screen->RootWindow;

	if (width <= 0 || height <= 0 || bordersize < 0) {
		DPrintF("Width/Height/Bordersize <0? \n");
		return NULL;
	}

	wp = (Window *) AllocVec(sizeof(struct Window), MEMF_CLEAR);
	if (wp == NULL) {
		DPrintF("AllocVec for sizeof Window failed\n");
		return NULL;
	}
	DPrintF("OpenWindow Screen = %x\n", screen);
	wp->screen = screen;
	wp->id = IBase->nextid++;
	wp->rp = rootwp->rp;
	wp->parent = pwp;
	wp->children = NULL;
	wp->siblings = pwp->children;
	wp->x = pwp->x + x;
	wp->y = pwp->y + y;
	wp->width = width;
	wp->height = height;
	wp->bordersize = bordersize;
	wp->background = background;
	wp->bordercolor = bordercolor;
//	wp->owner = curclient;
	wp->realized = FALSE;
//	wp->props = 0;
	wp->title = NULL;
	wp->clipregion = NULL;
	wp->buffer = NULL;
	pwp->children = wp;
	return wp;
}

#define BLACK			RGB(  0,  0,  0)
#define BACKGROUND		RGB(170,170,170)

struct Window *intu_OpenWindow(IntuitionBase *IBase, struct Screen *screen, INT32 x, INT32 y, INT32 width, INT32 height)
{
	struct Window *ret = NewWindow(IBase, screen, &screen->RootWindow, x, y, width, height, 1,  BACKGROUND, BLACK);
	DPrintF("Newwindow %x\n", ret);
	_MapWindow(IBase, ret);
	DPrintF("MapWindow\n");
	return ret;
}

