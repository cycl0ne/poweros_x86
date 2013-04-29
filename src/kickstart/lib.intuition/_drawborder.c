#include "intuitionbase.h"
#include "pixmap.h"
#include "coregfx.h"
#include "rastport.h"

#include "font.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"
#define SysBase		IBase->ib_SysBase
#define CoreGfxBase IBase->ib_GfxBase

void _DrawBorder(IntuitionBase *IBase, struct nWindow *wp)
{
	INT32	lminx;		/* left edge minimum x */
	INT32	rminx;		/* right edge minimum x */
	INT32	tminy;		/* top edge minimum y */
	INT32	bminy;		/* bottom edge minimum y */
	INT32	topy;		/* top y value of window */
	INT32	boty;		/* bottom y value of window */
	INT32	width;		/* original width of window */
	INT32	height;		/* original height of window */
	INT32	bs;			/* border size */
	struct CRastPort	*rp;

	rp = wp->frp;
	
	bs = wp->bordersize;
//	DPrintF("Bordersize = %d\n", bs);
	if (bs <= 0) return;

	width = wp->width;
	height = wp->height;
	lminx = wp->x - bs;
	rminx = wp->x + width;
	tminy = wp->y - bs;
	bminy = wp->y + height;
	topy = wp->y;
	boty = bminy - 1;
 
	wp->x -= bs;
	wp->y -= bs;
	wp->width += (bs * 2);
	wp->height += (bs * 2);
	wp->bordersize = 0;

	IBase->clipwp = NULL;
//	DPrintF("DrawBorder->SetClipWindow\n");
	_SetClipWindow(IBase, wp, NULL, 0);

	SetMode(rp, ROP_COPY);
	SetForegroundColor(rp, wp->bordercolor);
	SetDash(rp, 0, 0);
	SetFillMode(rp, FILL_SOLID);
//	DPrintF("DrawBorder->Line\n");

	if (bs == 1) {
		Line(rp, lminx, tminy, rminx, tminy, TRUE);
		Line(rp, lminx, bminy, rminx, bminy, TRUE);
		Line(rp, lminx, topy, lminx, boty, TRUE);
		Line(rp, rminx, topy, rminx, boty, TRUE);
	} else {
		FillRect(rp, lminx, tminy, width + bs * 2, bs);
		FillRect(rp, lminx, bminy, width + bs * 2, bs);
		FillRect(rp, lminx, topy, bs, height);
		FillRect(rp, rminx, topy, bs, height);
	}

	wp->x += bs;
	wp->y += bs;
	wp->width -= (bs * 2);
	wp->height -= (bs * 2);
	wp->bordersize = bs;
	IBase->clipwp = NULL;
}
