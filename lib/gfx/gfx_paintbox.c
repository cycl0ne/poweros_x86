/**
* File: /gfx_paintbox．c
* User: cycl0ne
* Date: 2014-11-26
* Time: 11:40 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#if 0
struct PaintBox {
	pSD			pb_PixMap;
	INT32		pb_ClipMinX;
	INT32		pb_ClipMinY;
	INT32		pb_ClipMaxX;
	INT32		pb_ClipMaxY;
	BOOL		pb_ClipResult;
	pClipRegion	pb_ClipRegion;
	
	PIXELVAL	pb_foreground;
	PIXELVAL	pb_background;
	BOOL		pb_usebg;
	INT32		pb_mode;
//	PalEntry	pb_palette[256];
//	INT32		pb_

	COLORVAL	pb_foreground_rgb;
	COLORVAL	pb_background_rgb;
	INT32		pb_dashmask;
	INT32		pb_dashcount;
	INT32		pb_fillmode;
	Stipple_t	pb_stipple;
	Tile_t		pb_tile;
	Point_t		pb_ts_offset;
	
	
}PaintBox_t, *pPB;
#endif

