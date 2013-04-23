#ifndef RASTPORT_H
#define RASTPORT_H

#include "types.h"
#include "regions.h"

#define CLIP_VISIBLE		0
#define CLIP_INVISIBLE		1
#define CLIP_PARTIAL		2

/* GdAllocPolyRegion types*/
#define POLY_EVENODD		1
#define POLY_WINDING		2

typedef struct CGfxPoint {
	INT32 x;
	INT32 y;
} CGfxPoint;

typedef struct CGfxStipple {
	INT32		width;
	INT32		height;
	UINT16 		*bitmap;
} CGfxStipple;

typedef struct CGfxTile {
	INT32	width;
	INT32	height;
	struct PixMap	*pixmap;
	struct CRastPort *rp;
} CGfxTile;

typedef struct CRastPort {
	struct PixMap	*crp_PixMap;
	UINT32		crp_Mode;
	UINT32		crp_DashMask;
	UINT32		crp_DashCount;
	UINT32		crp_FillMode;
	BOOL		crp_useBg;
	UINT32		crp_Foreground;
	UINT32		crp_ForegroundRGB;
	UINT32		crp_Background;
	UINT32		crp_BackgroundRGB;

	CGfxStipple	crp_Stipple;
	CGfxTile    crp_Tile;
	CGfxPoint	crp_ts_Offset;
	INT32		ts_origin_x;
	INT32 		ts_origin_y;
	INT32		crp_ClipMinX;
	INT32		crp_ClipMinY;
	INT32		crp_ClipMaxX;
	INT32		crp_ClipMaxY;
	BOOL		crp_ClipResult;
	struct ClipRegion	*crp_ClipRegion;
} CRastPort;

typedef struct {
	INT32	rows;		/* number of rows on screen */
	INT32	cols;		/* number of columns on screen */
	int 	xdpcm;		/* dots/centimeter in x direction */
	int 	ydpcm;		/* dots/centimeter in y direction */
	int	 	planes;		/* hw # planes*/
	int	 	bpp;		/* hw bpp*/
	int		data_format;/* MWIF_ image data format*/
	INT32	ncolors;	/* hw number of colors supported*/
	int 	fonts;		/* number of built-in fonts */
	int 	buttons;	/* buttons which are implemented */
	//MWKEYMOD modifiers;	/* modifiers which are implemented */
	int	 	pixtype;	/* format of pixel value*/
	int	 	portrait;	/* current portrait mode*/
	BOOL	fbdriver;	/* true if running mwin fb screen driver*/
	UINT32 	rmask;		/* red mask bits in pixel*/
	UINT32	gmask;		/* green mask bits in pixel*/
	UINT32	bmask;		/* blue mask bits in pixel*/
	UINT32	amask;		/* alpha mask bits in pixel*/
	INT32	xpos;		/* current x mouse position*/
	INT32	ypos;		/* current y mouse position*/

/* items below are get/set by the window manager and not used internally*/
	int	vs_width;	/* virtual screen width/height*/
	int	vs_height;
	int	ws_width;	/* workspace width/height*/
	int	ws_height;
} CGfxScreenInfo, *pCGfxScreenInfo;


#endif
