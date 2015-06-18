/**
* File: /gfx_draw．c
* User: cycl0ne
* Date: 2014-11-27
* Time: 10:06 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"
#include "dos.h"
#include "dos_io.h"
#include "dos_interface.h"

#define SysBase GfxBase->SysBase
#define DOSBase GfxBase->DOSBase
//#define RegionBase GfxBase->RegionBase

static inline void drawpoint(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y)
{
	if (ClipPoint(rp, x, y)) rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
}

void drawrow(pGfxBase GfxBase, pPB rp, INT32 x1, INT32 x2, INT32 y)
{
	INT32 temp;
	PixMap_t *psd = rp->pb_PixMap;
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
	CheckCursor(rp->pb_PixMap, x1, y, x2, y);

	/* If aren't trying to draw a dash, then head for the speed */
	if (!rp->pb_DashCount) 
	{
		while (x1 <= x2) 
		{
			if (ClipPoint(rp, x1, y)) 
			{
				temp = MIN(rp->pb_ClipMaxX, x2);
				psd->DrawHorzLine(rp->pb_PixMap, x1, temp, y, rp->pb_Foreground);
			} else
				temp = MIN(rp->pb_ClipMaxX, x2);
			x1 = temp + 1;
		}
	} else {
		int p, bit = 0;

		/* We want to draw a dashed line instead */
		for (p = x1; p <= x2; p++) 
		{
			if ((rp->pb_DashMask & (1 << bit)) && ClipPoint(rp, p, y))
				psd->DrawPixel(rp->pb_PixMap, p, y, rp->pb_Foreground);

			bit = (bit + 1) % rp->pb_DashCount;
		}
	}
}

void drawcol(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y1, INT32 y2)
{
	INT32 temp;
	PixMap_t *psd = rp->pb_PixMap;
	
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
	CheckCursor(rp->pb_PixMap, x, y1, x, y2);

	if (!rp->pb_DashCount) {
		while (y1 <= y2) 
		{
			if (ClipPoint(rp, x, y1)) 
			{
				temp = MIN(rp->pb_ClipMaxY, y2);
				psd->DrawVertLine(rp->pb_PixMap, x, y1, temp, rp->pb_Foreground);
			} else
				temp = MIN(rp->pb_ClipMaxY, y2);
			y1 = temp + 1;
		}
	} else {
		int p, bit = 0;

		/* We want to draw a dashed line instead */
		for (p = y1; p <= y2; p++) {
			if ((rp->pb_DashMask & (1<<bit)) && ClipPoint(rp, x, p))
				psd->DrawPixel(rp->pb_PixMap, x, p, rp->pb_Foreground);
			bit = (bit + 1) % rp->pb_DashCount;
		}
	}
}

void gfx_Point(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y)
{
	PixMap_t *psd = rp->pb_PixMap;
	if (ClipPoint(rp, x, y)) 
	{
		psd->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
		FixCursor(rp->pb_PixMap);
	}
}

void gfx_Line(pGfxBase GfxBase, pPB rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint) 
{
	int xdelta;		/* width of rectangle around line */
	int ydelta;		/* height of rectangle around line */
	int xinc;		/* increment for moving x coordinate */
	int yinc;		/* increment for moving y coordinate */
	int rem;		/* current remainder */
	unsigned int bit = 0;	/* used for dashed lines */
	INT32 temp;
	PixMap_t *psd = rp->pb_PixMap;

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
		drawrow(GfxBase, rp, x1, x2, y1);
		FixCursor(rp->pb_PixMap);
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
		drawcol(GfxBase, rp, x1, y1, y2);
		FixCursor(rp->pb_PixMap);
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
			 psd->Line(psd, x1, y1, x2, y2, rp->pb_Foreground);
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
	if (ClipPoint(rp, x1, y1)) psd->DrawPixel(rp->pb_PixMap, x1, y1, rp->pb_Foreground);

	if (xdelta >= ydelta) 
	{
		rem = xdelta / 2;
		UINT32	foreground = rp->pb_Foreground;
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

			if (rp->pb_DashCount) 
			{
				if ((rp->pb_DashMask & (1 << bit)) && ClipPoint(rp, x1, y1))
					psd->DrawPixel(rp->pb_PixMap, x1, y1, foreground);
				bit = (bit + 1) % rp->pb_DashCount;
			} else {	/* No dashes */
				//DPrintF("Calling DrawPixel %x %x\n", psd->DrawPixel, foreground);

				if (ClipPoint(rp, x1, y1))
					psd->DrawPixel(rp->pb_PixMap, x1, y1, foreground);
			}

			if (bDrawLastPoint && x1 == x2) break;
		}
	} else {
		rem = ydelta / 2;
		UINT32	foreground = rp->pb_Foreground;
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
			if (rp->pb_DashCount) 
			{
				if ((rp->pb_DashMask & (1 << bit)) && ClipPoint(rp, x1, y1))
					psd->DrawPixel(rp->pb_PixMap, x1, y1, foreground);

				bit = (bit + 1) % rp->pb_DashCount;
			} else {	/* No dashes */
				if (ClipPoint(rp, x1, y1))
					psd->DrawPixel(rp->pb_PixMap, x1, y1, foreground);
			}

			if (bDrawLastPoint && y1 == y2) break;
		}
	}
	FixCursor(rp->pb_PixMap);
}

void gfx_Rect(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y, INT32 width, INT32 height)
{
	INT32 maxx;
	INT32 maxy;

	if (width <= 0 || height <= 0) return;
	maxx = x + width - 1;
	maxy = y + height - 1;
	drawrow(GfxBase, rp, x, maxx, y);
	if (height > 1) drawrow(GfxBase, rp, x, maxx, maxy);
	if (height < 3)	return;
	++y;
	--maxy;
	drawcol(GfxBase, rp, x, y, maxy);
	if (width > 1) drawcol(GfxBase, rp, maxx, y, maxy);
	FixCursor(rp->pb_PixMap);
}

void gfx_RectFill(pGfxBase GfxBase, pPB rp, INT32 x1, INT32 y1, INT32 width, INT32 height)
{
	UINT32	dm = 0;
	UINT32	dc = 0;

	INT32 x2 = x1 + width - 1;
	INT32 y2 = y1 + height - 1;

	if (width <= 0 || height <= 0) return;

	/* Stipples and tiles have their own drawing routines */
	if (rp->pb_FillMode != FILL_SOLID) 
	{
		//set_ts_origin(x1, y1);
		//ts_fillrect(psd, x1, y1, width, height);
		FixCursor(rp->pb_PixMap);
		return;
	}
	pSD psd = rp->pb_PixMap;
	/* See if the rectangle is either totally visible or totally
	 * invisible. If so, then the rectangle drawing is easy.
	 */
	//Printf("Check Clip\n");
	switch (ClipArea(rp, x1, y1, x2, y2)) 
	{
		case CLIP_VISIBLE:
		//Printf("Call psd->FillRect with %d, %d, %d, %d\n",x1, y1, x2, y2);
		//Printf("PSD: %x %x\n", psd, psd->FillRect);
		psd->FillRect(rp->pb_PixMap, x1, y1, x2, y2, rp->pb_Foreground);
		FixCursor(rp->pb_PixMap);
		return;

	case CLIP_INVISIBLE:
		//Printf("Clip Invisible\n");
		return;
	}

	//Printf("partiial seen\n");
	/* Quickly save off the dash settings to avoid problems with drawrow */
	SetDash(rp, &dm, &dc);
	/* The rectangle may be partially obstructed. So do it line by line. */
	while (y1 <= y2)  drawrow(GfxBase, rp, x1, x2, y1++);

	/* Restore the dash settings */
	SetDash(rp, &dm, &dc);
	FixCursor(rp->pb_PixMap);
}

void gfx_BitmapByPoint(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y, INT32 width, INT32 height, const UINT16 *imagebits, int clipresult)
{
	INT32 minx;
	INT32 maxx;
	UINT16 bitvalue = 0;	/* bitmap word value */
	int bitcount;			/* number of bits left in bitmap word */

	//DPrintF("Using slow GdBitmapByPoint\n");
	if (width <= 0 || height <= 0) return;

	/* get valid clipresult if required*/
	if (clipresult < 0) clipresult = ClipArea(rp, x, y, x + width - 1, y + height - 1);
	if (clipresult == CLIP_INVISIBLE) return;

	/* fill background if necessary, use quick method if no clipping*/
	if (rp->pb_useBg) {
		if (clipresult == CLIP_VISIBLE)
			rp->pb_PixMap->FillRect(rp->pb_PixMap, x, y, x + width - 1, y + height - 1, rp->pb_Background);
		else {
			UINT32 savefg = rp->pb_Foreground;			
			rp->pb_Foreground = rp->pb_Background;
			RectFill(rp, x, y, width, height);
			rp->pb_Foreground = savefg;
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
			rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
		bitvalue = IMAGE_SHIFTBIT(bitvalue);
		bitcount--;
		if (x++ == maxx) {
			x = minx;
			++y;
			--height;
			bitcount = 0;
		}
	}
	FixCursor(rp->pb_PixMap);
}

void gfx_GetScreenInfo(pGfxBase GfxBase, pPB rp, pScreenInfo psi)
{
//	rp->pb_PixMap->GetScreenInfo(rp->pb_PixMap, psi);
//	rp->pb_PixMap->_GetScreenInfo(rp, psi);
//	GdGetButtonInfo(&psi->buttons);
//	GdGetModifierInfo(&psi->modifiers, NULL);
	GetCursorPos(&psi->xpos, &psi->ypos);
}
