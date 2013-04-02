#include "coregfx.h"
#include "rastport.h"
#include "pixmap.h"
#include "view.h"

#define SysBase CoreGfxBase->SysBase

void SVGA_FifoUpdateFullscreen(APTR VgaGfxBase);

void cgfx_MoveCursor(CoreGfxBase *CoreGfxBase, INT32 newx, INT32 newy)
{
	CGfxCursor *cur = &CoreGfxBase->Cursor;

	INT32 shiftx;
	INT32 shifty;

	shiftx = newx - cur->curminx;
	shifty = newy - cur->curminy;
	if(shiftx == 0 && shifty == 0) return;
	cur->curminx += shiftx;
	cur->curmaxx += shiftx;
	cur->curminy += shifty;
	cur->curmaxy += shifty;

	/* Restore the screen under the mouse pointer*/
	HideCursor();
	/* Draw the new pointer*/
	ShowCursor();
}

BOOL cgfx_GetCursorPos(CoreGfxBase *CoreGfxBase, INT32 *px, INT32 *py)
{
	CGfxCursor *cur = &CoreGfxBase->Cursor;
	*px = cur->xpos;
	*py = cur->ypos;
	return cur->curvisible > 0;	/* return TRUE if visible*/
}

void cgfx_SetCursor(CoreGfxBase *CoreGfxBase, struct Cursor *pcursor)
{
	int	bytes;
	CGfxCursor *cur = &CoreGfxBase->Cursor;
	if (!CoreGfxBase->ActiveView) return;

	CRastPort *rp = CoreGfxBase->ActiveView->vp->RastPort;

	HideCursor();
	cur->curmaxx = cur->curminx + pcursor->width - 1;
	cur->curmaxy = cur->curminy + pcursor->height - 1;

	cur->curfg = FindColor(rp, pcursor->fgcolor);
	cur->curbg = FindColor(rp, pcursor->bgcolor);

	bytes = IMAGE_SIZE(pcursor->width, pcursor->height) * sizeof(UINT32);

	memcpy(cur->cursorcolor, pcursor->image, bytes);
	memcpy(cur->cursormask, pcursor->mask, bytes);
	ShowCursor();
}

INT32 cgfx_ShowCursor(CoreGfxBase *CoreGfxBase)
{
	INT32 		x;
	INT32 		y;
	CGfxCursor *cur = &CoreGfxBase->Cursor;

	UINT32 *saveptr;
	UINT16 *cursorptr;
	UINT16 *maskptr;
	UINT16 	curbit, cbits = 0, mbits = 0;
	UINT32 	oldcolor;
	UINT32 	newcolor;
	int 	oldmode;
	int		prevcursor = cur->curvisible;

	if (CoreGfxBase->ActiveView == NULL) return prevcursor;	
	CRastPort	*rp	= CoreGfxBase->ActiveView->vp->RastPort;
	PixMap 		*psd= rp->crp_PixMap;

//	DPrintF("curvis: %d\n", cur->curvisible );
	if(++cur->curvisible != 1) 
	{
//		DPrintF("curvis: %d\n", cur->curvisible );
//		DPrintF("Cursor still hidden\n");
		return prevcursor;
	}
//	DPrintF("curvis: %d\n", cur->curvisible );
//	DPrintF("ShowCursor show Main %x\n", psd->addr);

	oldmode = SetMode(rp, ROP_COPY);

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
//		DPrintF("ShowCursor show Main %x, %d, %d\n", psd, x, y);
			if(x >= 0 && x < psd->xvirtres &&
			   y >= 0 && y < psd->yvirtres) 
			   {
				oldcolor = psd->ReadPixel(rp, x, y);
				//DPrintF("Readpixel %x ", oldcolor);
				if (curbit & mbits) 
				{
					newcolor = (curbit&cbits)? cur->curbg: cur->curfg;
					if (oldcolor != newcolor) psd->DrawPixel(rp, x, y, newcolor);
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
	SetMode(rp, oldmode);
	SVGA_FifoUpdateFullscreen(CoreGfxBase->VgaGfxBase);
	return prevcursor;
}

INT32 cgfx_HideCursor(CoreGfxBase *CoreGfxBase)
{
	UINT32 		*saveptr;
	INT32 		x, y;
	int 		oldmode;
	CGfxCursor 	*cur = &CoreGfxBase->Cursor;
	int			prevcursor = cur->curvisible;

	if (!CoreGfxBase->ActiveView) return prevcursor;
	CRastPort	*rp	= CoreGfxBase->ActiveView->vp->RastPort;
	PixMap 		*psd= rp->crp_PixMap;

	if (cur->curvisible-- <= 0) return prevcursor;

	oldmode = SetMode(rp, ROP_COPY);

	saveptr = cur->cursavbits;
	for (y = cur->cursavy; y <= cur->cursavy2; y++) 
	{
		for (x = cur->cursavx; x <= cur->cursavx2; x++) 
		{
			if(x >= 0 && x < psd->xvirtres &&
			   y >= 0 && y < psd->yvirtres) 
			{
				psd->DrawPixel(rp, x, y, *saveptr++);
			}
		}
	}
 	SetMode(rp, oldmode);
	SVGA_FifoUpdateFullscreen(CoreGfxBase->VgaGfxBase);
	return prevcursor;
}

void cgfx_CheckCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp,INT32 x1,INT32 y1,INT32 x2,INT32 y2)
{
	INT32 temp;
	PixMap *psd = rp->crp_PixMap;
	CGfxCursor	*cur = &CoreGfxBase->Cursor;

	if (cur->curvisible <= 0 || (psd->flags & PSF_SCREEN) == 0) return;

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

	HideCursor();
	cur->curneedsrestore = TRUE;
}

void cgfx_FixCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp)
{
	CGfxCursor	*cur = &CoreGfxBase->Cursor;

	if (cur->curneedsrestore && (rp->crp_PixMap->flags & PSF_SCREEN)) 
	{
		ShowCursor();
		cur->curneedsrestore = FALSE;
	}
	SVGA_FifoUpdateFullscreen(CoreGfxBase->VgaGfxBase);
}

