#include "coregfx.h"
#include "rastport.h"
#include "regions.h"
#include "pixmap.h"
#include "exec_funcs.h"

#define SysBase CoreGfxBase->SysBase

static inline void drawpoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
{
	if (ClipPoint(rp, x, y)) rp->crp_PixMap->_DrawPixel(rp, x, y, rp->crp_Foreground);
}

void drawrow(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 x2, INT32 y)
{
	INT32 temp;
	struct PixMap *psd = rp->crp_PixMap;
	/* reverse endpoints if necessary */
	if (x1 > x2) 
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
	}

	/* clip to physical device */
	if (x1 < 0) x1 = 0;
	if (x2 >= psd->xvirtres) x2 = psd->xvirtres - 1;

	/* check cursor intersect once for whole line */
	CheckCursor(psd, x1, y, x2, y);

	/* If aren't trying to draw a dash, then head for the speed */
	if (!rp->crp_DashCount) 
	{
		while (x1 <= x2) 
		{
			if (ClipPoint(rp, x1, y)) 
			{
				temp = MIN(rp->crp_ClipMaxX, x2);
				psd->_DrawHorzLine(rp, x1, temp, y, rp->crp_Foreground);
			} else
				temp = MIN(rp->crp_ClipMaxX, x2);
			x1 = temp + 1;
		}
	} else {
		unsigned int p, bit = 0;

		/* We want to draw a dashed line instead */
		for (p = x1; p <= x2; p++) 
		{
			if ((rp->crp_DashMask & (1 << bit)) && ClipPoint(rp, p, y))
				psd->_DrawPixel(rp, p, y, rp->crp_Foreground);

			bit = (bit + 1) % rp->crp_DashCount;
		}
	}
}

void drawcol(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y1, INT32 y2)
{
	INT32 temp;
	PixMap *psd = rp->crp_PixMap;
	
	/* reverse endpoints if necessary */
	if (y1 > y2) 
	{
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	/* clip to physical device */
	if (y1 < 0) y1 = 0;
	if (y2 >= psd->yvirtres) y2 = psd->yvirtres - 1;

	/* check cursor intersect once for whole line */
	CheckCursor(psd, x, y1, x, y2);

	if (!rp->crp_DashCount) {
		while (y1 <= y2) 
		{
			if (ClipPoint(rp, x, y1)) 
			{
				temp = MIN(rp->crp_ClipMaxY, y2);
				psd->_DrawVertLine(rp, x, y1, temp, rp->crp_Foreground);
			} else
				temp = MIN(rp->crp_ClipMaxY, y2);
			y1 = temp + 1;
		}
	} else {
		unsigned int p, bit = 0;

		/* We want to draw a dashed line instead */
		for (p = y1; p <= y2; p++) {
			if ((rp->crp_DashMask & (1<<bit)) && ClipPoint(rp, x, p))
				psd->_DrawPixel(rp, x, p, rp->crp_Foreground);
			bit = (bit + 1) % rp->crp_DashCount;
		}
	}
}

void cgfx_Point(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
{
	struct PixMap *psd = rp->crp_PixMap;
	if (ClipPoint(rp, x, y)) 
	{
		psd->_DrawPixel(rp, x, y, rp->crp_Foreground);
		FixCursor(psd);
	}
}

void cgfx_Line(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint) 
{
	int xdelta;		/* width of rectangle around line */
	int ydelta;		/* height of rectangle around line */
	int xinc;		/* increment for moving x coordinate */
	int yinc;		/* increment for moving y coordinate */
	int rem;		/* current remainder */
	unsigned int bit = 0;	/* used for dashed lines */
	INT32 temp;
	PixMap *psd = rp->crp_PixMap;

	/* See if the line is horizontal or vertical. If so, then call
	 * special routines.
	 */
//DPrintF("Line (Internal)\n");
	if (y1 == y2) 
	{
		if (!bDrawLastPoint) 
		{
			if (x1 > x2) 
			{
				temp = x1;
				x1 = x2 + 1;
				x2 = temp;
			} else
				--x2;
		}
		drawrow(CoreGfxBase, rp, x1, x2, y1);
		FixCursor(psd);
		return;
	}
	if (x1 == x2) 
	{
		if (!bDrawLastPoint) 
		{
			if (y1 > y2) 
			{
				temp = y1;
				y1 = y2 + 1;
				y2 = temp;
			} else
				--y2;
		}
		drawcol(CoreGfxBase, rp, x1, y1, y2);
		FixCursor(psd);
		return;
	}
	
	switch (ClipArea(rp, x1, y1, x2, y2)) 
	{
		case CLIP_VISIBLE:
			/*
			 * For size considerations, there's no low-level bresenham
			 * line draw, so we've got to draw all non-vertical
			 * and non-horizontal lines with per-point
			 * clipping for the time being
			 psd->Line(psd, x1, y1, x2, y2, rp->crp_Foreground);
			 GdFixCursor(psd);
			 return;
			 */
			break;
		case CLIP_INVISIBLE:
			return;
	}

	/* The line may be partially obscured. Do the draw line algorithm
	 * checking each point against the clipping regions.
	 */
	xdelta = x2 - x1;
	ydelta = y2 - y1;
	if (xdelta < 0) xdelta = -xdelta;
	if (ydelta < 0)	ydelta = -ydelta;
	xinc = (x2 > x1)? 1 : -1;
	yinc = (y2 > y1)? 1 : -1;

	/* draw first point*/
	if (ClipPoint(rp, x1, y1)) psd->_DrawPixel(rp, x1, y1, rp->crp_Foreground);

	if (xdelta >= ydelta) 
	{
		rem = xdelta / 2;
		UINT32	foreground = rp->crp_Foreground;
		for (;;) 
		{
			if (!bDrawLastPoint && x1 == x2) break;
			x1 += xinc;
			rem += ydelta;
			if (rem >= xdelta) 
			{
				rem -= xdelta;
				y1 += yinc;
			}

			if (rp->crp_DashCount) 
			{
				if ((rp->crp_DashMask & (1 << bit)) && ClipPoint(rp, x1, y1))
					psd->_DrawPixel(rp, x1, y1, foreground);
				bit = (bit + 1) % rp->crp_DashCount;
			} else {	/* No dashes */
				//DPrintF("Calling DrawPixel %x %x\n", psd->_DrawPixel, foreground);

				if (ClipPoint(rp, x1, y1))
					psd->_DrawPixel(rp, x1, y1, foreground);
			}

			if (bDrawLastPoint && x1 == x2) break;
		}
	} else {
		rem = ydelta / 2;
		UINT32	foreground = rp->crp_Foreground;
		for (;;) 
		{
			if (!bDrawLastPoint && y1 == y2) break;
			y1 += yinc;
			rem += xdelta;
			if (rem >= ydelta) 
			{
				rem -= ydelta;
				x1 += xinc;
			}

			/* If we are trying to draw to a dash mask */
			if (rp->crp_DashCount) 
			{
				if ((rp->crp_DashMask & (1 << bit)) && ClipPoint(rp, x1, y1))
					psd->_DrawPixel(rp, x1, y1, foreground);

				bit = (bit + 1) % rp->crp_DashCount;
			} else {	/* No dashes */
				if (ClipPoint(rp, x1, y1))
					psd->_DrawPixel(rp, x1, y1, foreground);
			}

			if (bDrawLastPoint && y1 == y2) break;
		}
	}
	FixCursor(psd);
}

void cgfx_Rect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height)
{
	INT32 maxx;
	INT32 maxy;

	if (width <= 0 || height <= 0) return;
	maxx = x + width - 1;
	maxy = y + height - 1;
	drawrow(CoreGfxBase, rp, x, maxx, y);
	if (height > 1) drawrow(CoreGfxBase, rp, x, maxx, maxy);
	if (height < 3)	return;
	++y;
	--maxy;
	drawcol(CoreGfxBase, rp, x, y, maxy);
	if (width > 1) drawcol(CoreGfxBase, rp, maxx, y, maxy);
	FixCursor(rp->crp_PixMap);
}

void cgfx_FillRect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 width, INT32 height)
{
	UINT32	dm = 0;
	UINT32	dc = 0;

	INT32 x2 = x1 + width - 1;
	INT32 y2 = y1 + height - 1;

	if (width <= 0 || height <= 0) return;

	/* Stipples and tiles have their own drawing routines */
	if (rp->crp_FillMode != FILL_SOLID) 
	{
		//set_ts_origin(x1, y1);
		//ts_fillrect(psd, x1, y1, width, height);
		FixCursor(rp->crp_PixMap);
		return;
	}
	PixMap *psd = rp->crp_PixMap;
	/* See if the rectangle is either totally visible or totally
	 * invisible. If so, then the rectangle drawing is easy.
	 */
//DPrintF("Check Clip\n");
	switch (ClipArea(rp, x1, y1, x2, y2)) 
	{
		case CLIP_VISIBLE:
		DPrintF("Call psd->FillRect with %d, %d, %d, %d\n",x1, y1, x2, y2);
		DPrintF("PSD: %x %x\n", psd, psd->_FillRect);
		psd->_FillRect(rp, x1, y1, x2, y2, rp->crp_Foreground);
		FixCursor(rp->crp_PixMap);
		return;

	case CLIP_INVISIBLE:
//		DPrintF("Clip Invisible\n");
		return;
	}


	/* Quickly save off the dash settings to avoid problems with drawrow */
	SetDash(rp, &dm, &dc);
	/* The rectangle may be partially obstructed. So do it line by line. */
	while (y1 <= y2)  drawrow(CoreGfxBase, rp, x1, x2, y1++);

	/* Restore the dash settings */
	SetDash(rp, &dm, &dc);
	FixCursor(rp->crp_PixMap);
}

void cgfx_BitmapByPoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height, const UINT16 *imagebits, int clipresult)
{
	INT32 minx;
	INT32 maxx;
	UINT16 bitvalue = 0;	/* bitmap word value */
	int bitcount;			/* number of bits left in bitmap word */

DPrintF("Using slow GdBitmapByPoint\n");
	if (width <= 0 || height <= 0) return;

	/* get valid clipresult if required*/
	if (clipresult < 0) clipresult = ClipArea(rp, x, y, x + width - 1, y + height - 1);
	if (clipresult == CLIP_INVISIBLE) return;

	/* fill background if necessary, use quick method if no clipping*/
	if (rp->crp_useBg) {
		if (clipresult == CLIP_VISIBLE)
			rp->crp_PixMap->_FillRect(rp, x, y, x + width - 1, y + height - 1, rp->crp_Background);
		else {
			UINT32 savefg = rp->crp_Foreground;			
			rp->crp_Foreground = rp->crp_Background;
			FillRect(rp, x, y, width, height);
			rp->crp_Foreground = savefg;
		}
	}
	minx = x;
	maxx = x + width - 1;
	bitcount = 0;
	while (height > 0) {
		if (bitcount <= 0) {
			bitcount = IMAGE_BITSPERIMAGE;
			bitvalue = *imagebits++;
		}
		if (IMAGE_TESTBIT(bitvalue) && (clipresult == CLIP_VISIBLE || ClipPoint(rp, x, y)))
			rp->crp_PixMap->_DrawPixel(rp, x, y, rp->crp_Foreground);
		bitvalue = IMAGE_SHIFTBIT(bitvalue);
		bitcount--;
		if (x++ == maxx) {
			x = minx;
			++y;
			--height;
			bitcount = 0;
		}
	}
	FixCursor(rp->crp_PixMap);
}

void cgfx_GetScreenInfo(CoreGfxBase *CoreGfxBase, CRastPort *rp, pCGfxScreenInfo psi)
{
	rp->crp_PixMap->_GetScreenInfo(rp, psi);
//	GdGetButtonInfo(&psi->buttons);
//	GdGetModifierInfo(&psi->modifiers, NULL);
	GetCursorPos(&psi->xpos, &psi->ypos);
}
