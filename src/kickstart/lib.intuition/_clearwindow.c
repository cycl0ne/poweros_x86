#include "intuitionbase.h"
#include "pixmap.h"
#include "coregfx.h"

#include "font.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

#define SysBase IBase->ib_SysBase
#define CoreGfxBase IBase->ib_GfxBase

void _ClearWindow(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 width, INT32 height, INT32 exposeflag)
{
	if (!wp->realized) return;

	if (x < 0) 
	{
		width += x;
		x = 0;
	}
	if (y < 0) 
	{
		height += y;
		y = 0;
	}
	if (x + width > wp->width) width = wp->width - x;
	if (y + height > wp->height) height = wp->height - y;
	if (x >= wp->width || y >= wp->height || width <= 0 || height <= 0) return;

#if 0
	if (exposeflag == 2) wp->props |= GR_WM_PROPS_DRAWING_DONE;

	if (wp->props & GR_WM_PROPS_BUFFERED) 
	{
		/* nothing to do until drawing finalized*/
		if (!(wp->props & GR_WM_PROPS_DRAWING_DONE)) return;
		/* prepare clipping to window boundaries*/
		SetClipWindow(wp, NULL, 0);
		clipwp = NULL;		/* reset clip cache since no user regions used*/

#if DEBUG_EXPOSE
curgcp = NULL;
GdSetFillMode(GR_FILL_SOLID);
GdSetMode(GR_MODE_COPY);
GdSetForegroundColor(wp->psd, MWRGB(255,255,0)); /* yellow*/
GdFillRect(wp->psd, wp->x+x, wp->y+y, width, height);
usleep(500000);
#endif
		/* copy window pixmap buffer to window*/
		Blit(wp->psd, wp->x + x, wp->y + y, width, height, wp->buffer->psd, x, y, MWROP_COPY);
		return;				/* don't deliver exposure events*/
	}

	/*
	 * Unbuffered window: erase background unless nobackground flag set
	 */
	if (!(wp->props & GR_WM_PROPS_NOBACKGROUND)) 
	{
		/* perhaps find a better way of determining whether pixmap needs src_over*/
		int hasalpha = wp->bgpixmap && (wp->bgpixmap->psd->data_format & MWIF_HASALPHA);

		/*
	 	 * Draw the background of the window.
	 	 * Invalidate the current graphics context since
	 	 * we are changing the foreground color and mode.
	 	 */
		SetClipWindow(wp, NULL, 0);
		clipwp = NULL;		/* reset clip cache since no user regions used*/

#if DEBUG_EXPOSE
GdSetFillMode(GR_FILL_SOLID);
GdSetMode(GR_MODE_COPY);
GdSetForegroundColor(wp->psd, MWRGB(255,255,0)); /* yellow*/
GdFillRect(wp->psd, wp->x+x, wp->y+y, width, height);
usleep(500000);
#endif

		curgcp = NULL;
		SetFillMode(GR_FILL_SOLID);
		SetMode(GR_MODE_COPY);
		SetForegroundColor(wp->psd, wp->background);

		/* if background pixmap w/alpha channel and stretchblit, fill entire (clipped) window*/
		if (hasalpha && wp->bgpixmapflags == GR_BACKGROUND_STRETCH) FillRect(wp->psd, wp->x, wp->y, wp->width, wp->height);
		else /* if no pixmap background clear exposed area*/
			if (!wp->bgpixmap || hasalpha)	/* FIXME will flash with pixmap, should check src_over*/
				if (!(wp->bgpixmapflags & GR_BACKGROUND_TRANS))
					FillRect(wp->psd, wp->x + x, wp->y + y, width, height);

		if (wp->bgpixmap) GsDrawBackgroundPixmap(wp, wp->bgpixmap, x, y, width, height);
	}
	if (exposeflag) GsDeliverExposureEvent(wp, x, y, width, height);
#endif
	_SetClipWindow(IBase, wp, NULL, 0);
	IBase->clipwp = NULL;
	SetFillMode(wp->frp, FILL_SOLID);
	SetMode(wp->frp, ROP_COPY);
	
//	DPrintF("Foregroundcolor: %x\n", wp->background );
	SetForegroundColor(wp->frp, wp->background);
	FillRect(wp->frp, wp->x + x, wp->y + y, width, height);
}
