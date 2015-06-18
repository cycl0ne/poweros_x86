/**
* File: /gfx_drawstipple．c
* User: cycl0ne
* Date: 2014-11-27
* Time: 11:39 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#define SysBase GfxBase->SysBase

#define SPITCH ((rp->pb_Stipple.width + (IMAGE_BITSPERIMAGE - 1)) / IMAGE_BITSPERIMAGE)
#define BIT_SET(data, w, h) (data[(h * SPITCH) + (w / IMAGE_BITSPERIMAGE)] & (1 << ((IMAGE_BITSPERIMAGE - 1) - (w % IMAGE_BITSPERIMAGE))))

void gfx_SetStippleBitmap(pGfxBase GfxBase, pPB rp, UINT16 *stipple, INT32 width, INT32 height)
{
	int size;

	if (rp->pb_Stipple.bitmap) FreeVec(rp->pb_Stipple.bitmap);

	rp->pb_Stipple.width = 0;
	rp->pb_Stipple.height = 0;

	if (!stipple) {
		rp->pb_Stipple.bitmap = 0;
		return;
	}

	size = IMAGE_SIZE(width, height) * sizeof(UINT16);
	rp->pb_Stipple.bitmap = AllocVec(size, MEMF_FAST);
	if (!rp->pb_Stipple.bitmap) return;
	rp->pb_Stipple.width = width;
	rp->pb_Stipple.height = height;
	memcpy(rp->pb_Stipple.bitmap, stipple, size);

#if 0
	for (y = 0; y < height; y++) { /* debug output*/
		for (x = 0; x < width; x++) {
			if (BIT_SET(rp->pb_Stipple.bitmap, x, y))
				DPrintF("X");
			else
				DPrintF("_");
		}
		DPrintF("\n");
	}
#endif
}

void gfx_SetTilePixmap(pGfxBase GfxBase, pPB rp, PixMap_t *src, INT32 width, INT32 height)
{
	rp->pb_Tile.pixmap = src;
	rp->pb_Tile.rp		= rp;
	if (!src) {
		rp->pb_Tile.width = 0;
		rp->pb_Tile.height = 0;
	} else {
		rp->pb_Tile.width = width;
		rp->pb_Tile.height = height;
	}
}

/* This sets the stipple offset to the specified offset */
void gfx_SetTSOffset(pGfxBase GfxBase, pPB rp, int x, int y)
{
	rp->pb_ts_Offset.x = x;
	rp->pb_ts_Offset.y = y;
}

/* Set the bounding rect for the stipple.  This also constructs a bitmap that gives us an easy
   lookup when the time comes */

/* This only works for tiles */
static void tile_drawrect(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y, INT32 w, INT32 h)
{
	int sx = x, sy = y;
	int px = 0, py = 0;
	int dw = w;
	int dh = h;

	/* This is where the tile starts */
	int tilex = x - rp->ts_origin_x;
	int tiley = y - rp->ts_origin_y;

	/* Sanity check */
	if (!rp->pb_Tile.pixmap || !rp->pb_Tile.width || !rp->pb_Tile.height) return;

	/* Adjust the starting point to correspond with the tile offset */
	if (tilex < 0) 
	{
		sx += -tilex;
		dw -= -tilex;

		if (sx > (x + w - 1)) return;
		tilex = sx - rp->ts_origin_x;
	}

	if (tiley < 0) {
		sy += -tiley;
		dh -= -tiley;

		if (sy > (y + h - 1)) return;
		tiley = sy - rp->ts_origin_y;
	}

	while (dh) {
		int ch = rp->pb_Tile.height - ((tiley + py) % rp->pb_Tile.height);
		if (ch > dh)
			ch = dh;

		dw = w;
		px = 0;

		while (dw) {
			int cw = rp->pb_Tile.width - ((tilex + px) % rp->pb_Tile.width);
			if (cw > dw) cw = dw;
			Blit(rp, sx + px, sy + py, cw, ch, rp->pb_Tile.rp, (tilex + px) % rp->pb_Tile.width, (tiley + py) % rp->pb_Tile.height, ROP_COPY);
			dw -= cw;
			px += cw;
		}
		dh -= ch;
		py += ch;
	}
}

/* This sets the origin of the stipple (we add the offset) */
/* We use this in the following functions                  */
void set_ts_origin(pPB rp, int x, int y)
{
	rp->ts_origin_x = x + rp->pb_ts_Offset.x;
	rp->ts_origin_y = y + rp->pb_ts_Offset.y;
}

/* For these, we need to ensure that the points fall within the stipple box */
void ts_drawpoint(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y)
{
	int bx = x - rp->ts_origin_x;
	int by = y - rp->ts_origin_y;

	/* Sanity check - If no stipple / tile is set, then just ignore the request */
	/* FIXME:  X returns an error - Should we too?                              */
	if (rp->pb_FillMode == FILL_STIPPLE || rp->pb_FillMode == FILL_OPAQUE_STIPPLE) 
	{
		if (!rp->pb_Stipple.bitmap || !rp->pb_Stipple.width || !rp->pb_Stipple.height) return;
	} else {
		if (!rp->pb_Tile.pixmap || !rp->pb_Tile.width || !rp->pb_Tile.height) return;
	}


	if (!ClipPoint(rp, x, y))return;

	/* If the bit offset is less than zero    */
	/* Meaning that the pixel in question     */
	/* is to the left or above the current    */
	/* offset - Then just draw the foreground */
	/* FIXME:  Should we just skip the pixel instead? */
	if (bx < 0 || by < 0) 
	{
		rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
		return;
	}

	switch (rp->pb_FillMode) {
	case FILL_OPAQUE_STIPPLE:
		if (!BIT_SET(rp->pb_Stipple.bitmap, (bx % rp->pb_Stipple.width), (by % rp->pb_Stipple.height)))
			rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Background);
		else
			rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
		break;

	case FILL_STIPPLE:
		if (BIT_SET(rp->pb_Stipple.bitmap, (bx % rp->pb_Stipple.width), (by % rp->pb_Stipple.height)))
			rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Foreground);
		break;

	case FILL_TILE:
		/* Read the bit from the PSD and write it to the current PSD */
		/* FIXME:  This does no checks for depth correctness         */
		rp->pb_PixMap->DrawPixel(rp->pb_PixMap, x, y, rp->pb_Tile.pixmap->ReadPixel(rp->pb_Tile.rp->pb_PixMap, (bx % rp->pb_Tile.width), (by % rp->pb_Tile.height)));
		break;
	}
}

/* FIXME:  Optimize the stipple so it uses more bliting and less pixel by pixel stuff */
void ts_drawrow(pGfxBase GfxBase, pPB rp, INT32 x1, INT32 x2, INT32 y)
{
	int x;
	int dstwidth = x2 - x1 + 1;

	switch (rp->pb_FillMode) 
	{
	case FILL_STIPPLE:
	case FILL_OPAQUE_STIPPLE:
		for (x = x1; x <= x2; x++) ts_drawpoint(GfxBase, rp, x, y);
		break;
	case FILL_TILE:
		tile_drawrect(GfxBase, rp, x1, y, dstwidth, 1);
	}
}

void ts_fillrect(pGfxBase GfxBase, pPB rp, INT32 x, INT32 y, INT32 w, INT32 h)
{
	int x1 = x;
	int x2 = x + w - 1;
	int y1 = y;
	int y2 = y + h - 1;

	if (ClipArea(rp, x1, y1, x2, y2) == CLIP_INVISIBLE) return;

	if (rp->pb_FillMode == FILL_TILE)
		tile_drawrect(GfxBase, rp, x, y, w, h);
	else
		for (; y1 <= y2; y1++)
			ts_drawrow(GfxBase, rp, x1, x2, y1);
}


