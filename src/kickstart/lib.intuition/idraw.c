#include "intuitionbase.h"
#include "pixmap.h"
#include "coregfx.h"

#include "font.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"
#define CoreGfxBase IBase->ib_GfxBase

void intu_ILine(IntuitionBase *IBase, struct nWindow *wp, INT32 x1, INT32 y1, INT32 x2, INT32 y2)
{
	//SERVER_LOCK();
	switch (_PrepareDrawing(IBase, wp)) 
	{
	case GR_DRAW_TYPE_WINDOW:
	case GR_DRAW_TYPE_PIXMAP:
		Line(wp->frp, wp->x + x1, wp->y + y1, wp->x + x2, wp->y +y1, TRUE);
		break;
	}
	//	SERVER_UNLOCK();
}

void intu_IRectFill(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 width, INT32 height)
{
	//SERVER_LOCK();

	switch (_PrepareDrawing(IBase, wp)) 
	{
	case GR_DRAW_TYPE_WINDOW:
	case GR_DRAW_TYPE_PIXMAP:
		FillRect(wp->frp, wp->x + x, wp->y + y, width, height);
		break;
	}

	//	SERVER_UNLOCK();
}

void intu_IRect(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 width, INT32 height)
{
	//SERVER_LOCK();

	switch (_PrepareDrawing(IBase, wp)) 
	{
	case GR_DRAW_TYPE_WINDOW:
	case GR_DRAW_TYPE_PIXMAP:
		Rect(wp->frp, wp->x + x, wp->y + y, width, height);
		break;
	}

	//	SERVER_UNLOCK();
}

void intu_IEllipse(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 rx, INT32 ry)
{
	//SERVER_LOCK();

	switch (_PrepareDrawing(IBase, wp)) 
	{
	case GR_DRAW_TYPE_WINDOW:
	case GR_DRAW_TYPE_PIXMAP:
		Ellipse(wp->frp, wp->x + x, wp->y + y, rx, ry, FALSE);
		break;
	}

	//	SERVER_UNLOCK();
}

void intu_IFillEllipse(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 rx, INT32 ry)
{
	//SERVER_LOCK();
	switch (_PrepareDrawing(IBase, wp)) 
	{
	case GR_DRAW_TYPE_WINDOW:
	case GR_DRAW_TYPE_PIXMAP:
		Ellipse(wp->frp, wp->x + x, wp->y + y, rx, ry, TRUE);
		break;
	}
	//	SERVER_UNLOCK();
}

void intu_IText(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, STRPTR str, INT32 count, UINT32 flags)
{
	/* default to baseline alignment if none specified*/
	if((flags&(TF_TOP|TF_BASELINE|TF_BOTTOM)) == 0) flags |= TF_BASELINE;

	//SERVER_LOCK();
	switch (_PrepareDrawing(IBase, wp)) 
	{
		case GR_DRAW_TYPE_WINDOW:
		case GR_DRAW_TYPE_PIXMAP:
		Text(wp->frp, IBase->ib_SystemFont[0], wp->x + x, wp->y + y, str, count, flags);
		break;
	}
	//SERVER_UNLOCK();
}
