#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "regions.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

#define SysBase 	CoreGfxBase->SysBase

#define SPITCH ((rp->crp_Stipple.width + (IMAGE_BITSPERIMAGE - 1)) / IMAGE_BITSPERIMAGE)
#define BIT_SET(data, w, h) (data[(h * SPITCH) + (w / IMAGE_BITSPERIMAGE)] & (1 << ((IMAGE_BITSPERIMAGE - 1) - (w % IMAGE_BITSPERIMAGE))))

void cgfx_SetStippleBitmap(CoreGfxBase *CoreGfxBase, CRastPort *rp, UINT16 *stipple, INT32 width, INT32 height)
{
	int size;

	if (rp->crp_Stipple.bitmap) FreeVec(rp->crp_Stipple.bitmap);

	rp->crp_Stipple.width = 0;
	rp->crp_Stipple.height = 0;

	if (!stipple) {
		rp->crp_Stipple.bitmap = 0;
		return;
	}

	size = IMAGE_SIZE(width, height) * sizeof(UINT16);
	rp->crp_Stipple.bitmap = AllocVec(size, MEMF_FAST);
	if (!rp->crp_Stipple.bitmap) return;
	rp->crp_Stipple.width = width;
	rp->crp_Stipple.height = height;
	memcpy(rp->crp_Stipple.bitmap, stipple, size);

#if 0
	for (y = 0; y < height; y++) { /* debug output*/
		for (x = 0; x < width; x++) {
			if (BIT_SET(rp->crp_Stipple.bitmap, x, y))
				DPrintF("X");
			else
				DPrintF("_");
		}
		DPrintF("\n");
	}
#endif
}

void cgfx_SetTilePixmap(CoreGfxBase *CoreGfxBase, CRastPort *rp, PixMap *src,INT32 width, INT32 height)
{
	rp->crp_Tile.pixmap = src;
	if (!src) {
		rp->crp_Tile.width = 0;
		rp->crp_Tile.height = 0;
	} else {
		rp->crp_Tile.width = width;
		rp->crp_Tile.height = height;
	}
}

/* This sets the stipple offset to the specified offset */
void cgfx_SetTSOffset(CoreGfxBase *CoreGfxBase, CRastPort *rp, int x, int y)
{
	rp->crp_ts_Offset.x = x;
	rp->crp_ts_Offset.y = y;
}

/* Set the bounding rect for the stipple.  This also constructs a bitmap that gives us an easy
   lookup when the time comes */

/* This only works for tiles */
static void tile_drawrect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 w, INT32 h)
{
	int sx = x, sy = y;
	int px = 0, py = 0;
	int dw = w;
	int dh = h;

	/* This is where the tile starts */
	int tilex = x - rp->ts_origin_x;
	int tiley = y - rp->ts_origin_y;

	/* Sanity check */
	if (!rp->crp_Tile.pixmap || !rp->crp_Tile.width || !rp->crp_Tile.height) return;

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
		int ch = rp->crp_Tile.height - ((tiley + py) % rp->crp_Tile.height);
		if (ch > dh)
			ch = dh;

		dw = w;
		px = 0;

		while (dw) {
			int cw = rp->crp_Tile.width - ((tilex + px) % rp->crp_Tile.width);
			if (cw > dw) cw = dw;
//FIXME!!			Blit(rp, sx + px, sy + py, cw, ch, rp->crp_Tile.pixmap, (tilex + px) % rp->crp_Tile.width, (tiley + py) % rp->crp_Tile.height, ROP_COPY);
			dw -= cw;
			px += cw;
		}
		dh -= ch;
		py += ch;
	}
}

/* This sets the origin of the stipple (we add the offset) */
/* We use this in the following functions                  */
void set_ts_origin(CRastPort *rp, int x, int y)
{
	rp->ts_origin_x = x + rp->crp_ts_Offset.x;
	rp->ts_origin_y = y + rp->crp_ts_Offset.y;
}

/* For these, we need to ensure that the points fall within the stipple box */
void ts_drawpoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
{
	int bx = x - rp->ts_origin_x;
	int by = y - rp->ts_origin_y;

	/* Sanity check - If no stipple / tile is set, then just ignore the request */
	/* FIXME:  X returns an error - Should we too?                              */
	if (rp->crp_FillMode == FILL_STIPPLE || rp->crp_FillMode == FILL_OPAQUE_STIPPLE) 
	{
		if (!rp->crp_Stipple.bitmap || !rp->crp_Stipple.width || !rp->crp_Stipple.height) return;
	} else {
		if (!rp->crp_Tile.pixmap || !rp->crp_Tile.width || !rp->crp_Tile.height) return;
	}


	if (!ClipPoint(rp, x, y))return;

	/* If the bit offset is less than zero    */
	/* Meaning that the pixel in question     */
	/* is to the left or above the current    */
	/* offset - Then just draw the foreground */
	/* FIXME:  Should we just skip the pixel instead? */
	if (bx < 0 || by < 0) 
	{
		rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Foreground);
		return;
	}

	switch (rp->crp_FillMode) {
	case FILL_OPAQUE_STIPPLE:
		if (!BIT_SET(rp->crp_Stipple.bitmap, (bx % rp->crp_Stipple.width), (by % rp->crp_Stipple.height)))
			rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Background);
		else
			rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Foreground);
		break;

	case FILL_STIPPLE:
		if (BIT_SET(rp->crp_Stipple.bitmap, (bx % rp->crp_Stipple.width), (by % rp->crp_Stipple.height)))
			rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Foreground);
		break;

	case FILL_TILE:
		/* Read the bit from the PSD and write it to the current PSD */
		/* FIXME:  This does no checks for depth correctness         */
//FIX!!		rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Tile.pixmap->ReadPixel(rp->crp_Tile.pixmap, (bx % rp->crp_Tile.width), (by % rp->crp_Tile.height)));
		break;
	}
}

/* FIXME:  Optimize the stipple so it uses more bliting and less pixel by pixel stuff */
void ts_drawrow(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 x2, INT32 y)
{
	int x;
	int dstwidth = x2 - x1 + 1;

	switch (rp->crp_FillMode) 
	{
	case FILL_STIPPLE:
	case FILL_OPAQUE_STIPPLE:
		for (x = x1; x <= x2; x++) ts_drawpoint(CoreGfxBase, rp, x, y);
		break;
	case FILL_TILE:
		tile_drawrect(CoreGfxBase, rp, x1, y, dstwidth, 1);
	}
}

void ts_fillrect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 w, INT32 h)
{
	int x1 = x;
	int x2 = x + w - 1;
	int y1 = y;
	int y2 = y + h - 1;

	if (ClipArea(rp, x1, y1, x2, y2) == CLIP_INVISIBLE) return;

	if (rp->crp_FillMode == FILL_TILE)
		tile_drawrect(CoreGfxBase, rp, x, y, w, h);
	else
		for (; y1 <= y2; y1++)
			ts_drawrow(CoreGfxBase, rp, x1, x2, y1);
}
