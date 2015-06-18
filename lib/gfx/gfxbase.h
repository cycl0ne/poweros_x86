/**
* File: /framebuffer．h
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef GFXBASE_H
#define GFXBASE_H
#include "types.h"
#include "libraries.h"
#include "regions.h"
#include "gfx.h"
#include "fonts.h"
#include "exec_interface.h"
#include "gfx_interface.h"

#define memcpy(a,b,c) CopyMem(b,a,c)

typedef struct GfxCursor {
	INT32	xpos;		/* current x position of mouse */
	INT32	ypos;		/* current y position of mouse */
	INT32	minx;		/* minimum allowed x position */
	INT32	maxx;		/* maximum allowed x position */
	INT32	miny;		/* minimum allowed y position */
	INT32	maxy;		/* maximum allowed y position */
	INT32	scale;		/* acceleration scale factor */
	INT32	thresh;		/* acceleration threshhold */
	INT32	buttons;	/* current state of buttons */
	INT32	changed;	/* mouse state has changed */
	INT32 	curminx;	/* minimum x value of cursor */
	INT32 	curminy;	/* minimum y value of cursor */
	INT32 	curmaxx;	/* maximum x value of cursor */
	INT32 	curmaxy;	/* maximum y value of cursor */
	INT32	curvisible;	/* >0 if cursor is visible*/
	INT32 	curneedsrestore;/* cursor needs restoration after drawing*/
	INT32 	cursavx;	/* saved cursor location*/
	INT32 	cursavy;
	INT32	cursavx2;
	INT32	cursavy2;
	UINT32 	curfg;		/* foreground color of cursor */
	UINT32	curbg;		/* background color of cursor */
	UINT32	cursavbits[MAX_CURSOR_SIZE * MAX_CURSOR_SIZE];
	UINT16	cursormask[MAX_CURSOR_BUFLEN];
	UINT16	cursorcolor[MAX_CURSOR_BUFLEN];	
} GfxCursor_t, *pGfxCursor;

typedef struct GfxBase {
	struct Library	Library;
	APTR			SysBase;
	APTR			RegionBase;
	APTR			DOSBase;
	GfxCursor_t		Cursor;
	pSD				scrdev;
	PaintBox_t		paintBox;
	CoreFont_t		*builtin_fonts;
	CoreFont_t		*user_builtin_fonts;
} GfxBase_t, *pGfxBase;

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
} ScreenInfo, *pScreenInfo;

void gen_unloadfont(pGfxBase GfxBase,pFont pfont);
void gen_drawtext(pGfxBase GfxBase,pFont pfont, pPB rp, INT32 x, INT32 y, const void *text, int cc, UINT32 flags);
void gen_gettextbits(pGfxBase GfxBase,pFont pfont, int ch, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
void gen_gettextsize(pGfxBase GfxBase,pFont pfont, const void *text, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
BOOL gen_getfontinfo(pGfxBase GfxBase,pFont pfont, pFontInfo pfontinfo);
void gen_setfontproc(pCoreFont pf);

void dbcs_gettextbits(pGfxBase GfxBase, pFont pfont, int ch, UINT32 flags, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
void dbcs_gettextsize(pGfxBase GfxBase, pFont pfont, const unsigned short *str, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase);

#endif
