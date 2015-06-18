/**
* File: /gfx_cursor．c
* User: cycl0ne
* Date: 2014-11-27
* Time: 12:20 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#define SysBase GfxBase->SysBase

void gfx_MoveCursor(pGfxBase GfxBase, pPB pb, INT32 newx, INT32 newy)
{
	pGfxCursor cur = &GfxBase->Cursor;

	INT32 shiftx;
	INT32 shifty;

	shiftx = newx - cur->curminx;
	shifty = newy - cur->curminy;
	if (shiftx == 0 && shifty == 0) return;
	cur->curminx += shiftx;
	cur->curmaxx += shiftx;
	cur->curminy += shifty;
	cur->curmaxy += shifty;

	/* Restore the screen under the mouse pointer*/
	HideCursor(pb->pb_PixMap);
	/* Draw the new pointer*/
	ShowCursor(pb->pb_PixMap);
}

BOOL gfx_GetCursorPos(pGfxBase GfxBase, INT32 *px, INT32 *py)
{
	pGfxCursor cur = &GfxBase->Cursor;
	*px = cur->xpos;
	*py = cur->ypos;
	return cur->curvisible > 0;	/* return TRUE if visible*/
}

void gfx_SetCursor(pGfxBase GfxBase, pPB pb, struct Cursor *pcursor)
{
	int	bytes;
	pGfxCursor	cur = &GfxBase->Cursor;

	HideCursor(pb->pb_PixMap);
	cur->curmaxx = cur->curminx + pcursor->width - 1;
	cur->curmaxy = cur->curminy + pcursor->height - 1;

	cur->curfg = FindColor(pb, pcursor->fgcolor);
	cur->curbg = FindColor(pb, pcursor->bgcolor);

	bytes = IMAGE_SIZE(pcursor->width, pcursor->height) * sizeof(IMAGEBITS);

	CopyMem(pcursor->image, cur->cursorcolor, bytes);
	CopyMem(pcursor->mask, cur->cursormask, bytes);
	ShowCursor(pb->pb_PixMap);
}

INT32 gfx_ShowCursor(pGfxBase GfxBase, pSD psd)
{
	INT32 		x;
	INT32 		y;
	pGfxCursor cur = &GfxBase->Cursor;
	UINT32 *saveptr;
	UINT16 *cursorptr;
	UINT16 *maskptr;
	UINT16 	curbit, cbits = 0, mbits = 0;
	UINT32 	oldcolor;
	UINT32 	newcolor;
	int 	oldmode;
	int		prevcursor = cur->curvisible;

	if (++cur->curvisible != 1) 
	{
		return prevcursor;
	}

	oldmode = psd->mode;//SetMode(rp, ROP_COPY);
	psd->mode = ROP_COPY;

	saveptr			= cur->cursavbits;
	cur->cursavx	= cur->curminx;
	cur->cursavy	= cur->curminy;
	cur->cursavx2	= cur->curmaxx;
	cur->cursavy2	= cur->curmaxy;
	cursorptr		= cur->cursorcolor;
	maskptr			= cur->cursormask;

	/*
	 * Loop through bits, resetting to firstbit at end of each row
	 */
	curbit = 0;
	for (y = cur->curminy; y <= cur->curmaxy; y++) 
	{
		if (curbit != IMAGE_FIRSTBIT) 
		{
			cbits = *cursorptr++;
			mbits = *maskptr++;
			curbit = IMAGE_FIRSTBIT;
		}
		for (x = cur->curminx; x <= cur->curmaxx; x++) 
		{
			if(x >= 0 && x < psd->xvirtres &&
			   y >= 0 && y < psd->yvirtres) 
			   {
				oldcolor = psd->ReadPixel(psd, x, y);
				if (curbit & mbits) 
				{
					newcolor = (curbit&cbits)? cur->curbg: cur->curfg;
					if (oldcolor != newcolor) psd->DrawPixel(psd, x, y, newcolor);
				}
				*saveptr++ = oldcolor;
			}
			curbit = IMAGE_NEXTBIT(curbit);
			if (!curbit) 
			{	/* check > one UINT16 wide*/
				cbits = *cursorptr++;
				mbits = *maskptr++;
				curbit = IMAGE_FIRSTBIT;
			}
		}
	}
 	psd->mode = oldmode;
	return prevcursor;
}

INT32 gfx_HideCursor(pGfxBase GfxBase, pSD psd)
{
	UINT32 		*saveptr;
	INT32 		x, y;
	int 		oldmode;
	pGfxCursor 	cur = &GfxBase->Cursor;
	int			prevcursor = cur->curvisible;

	if (cur->curvisible-- <= 0) return prevcursor;

	oldmode = psd->mode;//SetMode(rp, ROP_COPY);
	psd->mode = ROP_COPY;
	
	saveptr = cur->cursavbits;
	for (y = cur->cursavy; y <= cur->cursavy2; y++) 
	{
		for (x = cur->cursavx; x <= cur->cursavx2; x++) 
		{
			if(x >= 0 && x < psd->xvirtres &&
			   y >= 0 && y < psd->yvirtres) 
			{
				psd->DrawPixel(psd, x, y, *saveptr++);
			}
		}
	}
 	//SetMode(rp, oldmode);
 	psd->mode = oldmode;
	return prevcursor;
}

void gfx_CheckCursor(pGfxBase GfxBase, pSD psd, INT32 x1,INT32 y1,INT32 x2,INT32 y2)
{
	INT32 temp;
	pGfxCursor	cur = &GfxBase->Cursor;

	if (cur->curvisible <= 0 || (psd->flags & PMF_SCREEN) == 0) return;

	if (x1 > x2) 
	{
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) 
	{
		temp = y1;
		y1 = y2;
		y2 = temp;
	}
	if (x1 > cur->curmaxx || x2 < cur->curminx || y1 > cur->curmaxy || y2 < cur->curminy) return;

	HideCursor(psd);
	cur->curneedsrestore = TRUE;
}

void gfx_FixCursor(pGfxBase GfxBase, pSD psd)
{
	pGfxCursor	cur = &GfxBase->Cursor;

	if (cur->curneedsrestore && (psd->flags & PMF_SCREEN)) 
	{
		ShowCursor(psd);
		cur->curneedsrestore = FALSE;
	}
}

