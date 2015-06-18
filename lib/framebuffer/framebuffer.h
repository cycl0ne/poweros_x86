/**
* File: /framebuffer．h
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
#include "types.h"
#include "libraries.h"
#include "exec_interface.h"
#include "pci.h"

#include "fonts.h"

typedef unsigned short	IMAGEBITS;

typedef struct FBBase {
	struct Library	Library;
	APTR			fb_SysBase;
	PCIAddress		fb_PCIAddr;
	UINT8*			fb_Buffer;
	INT16			fb_Resolution_X;
	INT16			fb_Resolution_Y;
	INT16			fb_Resolution_B;
} FBBase_t, *pFBBase;

typedef UINT8*	ADDR8;
typedef UINT16*	ADDR16;
typedef UINT32*	ADDR32;

#define NOCOLOR	0x01000000L

#define muldiv255(a,b)		((((a)+1)*(b))>>8)		/* very fast, 92% accurate*/
#define mulscale(a,b,n)		((((a)+1)*(b))>>(n))	/* very fast, always shift for 16bpp*/
#define muldiv255_rgb565(d, sr, sg, sb, as) \
						  (((((((d) & 0xF800) - (sr)) * as) >> 8) + (sr)) & 0xF800)\
						| (((((((d) & 0x07E0) - (sg)) * as) >> 8) + (sg)) & 0x07E0)\
						| (((((((d) & 0x001F) - (sb)) * as) >> 8) + (sb)) & 0x001F)
#define muldiv255_rgb555(d, sr, sg, sb, as) \
						  (((((((d) & 0x7C00) - (sr)) * as) >> 8) + (sr)) & 0x7C00)\
						| (((((((d) & 0x03E0) - (sg)) * as) >> 8) + (sg)) & 0x03E0)\
						| (((((((d) & 0x001F) - (sb)) * as) >> 8) + (sb)) & 0x001F)
#define muldiv255_rgb1555(d, sr, sg, sb, as) \
						  (((((((d) & 0x001F) - (sr)) * as) >> 8) + (sr)) & 0x001F)\
						| (((((((d) & 0x03E0) - (sg)) * as) >> 8) + (sg)) & 0x03E0)\
						| (((((((d) & 0x7C00) - (sb)) * as) >> 8) + (sb)) & 0x7C00)

/* Truecolor color conversion and extraction macros*/
/*
 * Conversion from 8-bit RGB components to MWPIXELVAL
 */
/* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from RGB triplet*/
#define RGB2PIXEL8888(r,g,b)	\
	(0xFF000000UL | ((r) << 16) | ((g) << 8) | (b))

/* create 32 bit 8/8/8/8 format pixel (0xAABBGGRR) from RGB triplet*/
#define RGB2PIXELABGR(r,g,b)	\
	(0xFF000000UL | ((b) << 16) | ((g) << 8) | (r))

/* create 32 bit 8/8/8/8 format pixel (0xAARRGGBB) from ARGB quad*/
#define ARGB2PIXEL8888(a,r,g,b)	\
	(((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

/* create 32 bit 8/8/8/8 format pixel (0xAABBGGRR) from ARGB quad*/
#define ARGB2PIXELABGR(a,r,g,b)	\
	(((a) << 24) | ((b) << 16) | ((g) << 8) | (r))

/* create 24 bit 8/8/8 format pixel (0x00RRGGBB) from RGB triplet*/
#define RGB2PIXEL888(r,g,b)	\
	(((r) << 16) | ((g) << 8) | (b))

/* create 16 bit 5/6/5 format pixel from RGB triplet */
#define RGB2PIXEL565(r,g,b)	\
	((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

/* create 16 bit 5/5/5 format pixel from RGB triplet */
#define RGB2PIXEL555(r,g,b)	\
	((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3))

/* create 16 bit b/g/r 5/5/5 format pixel from RGB triplet */
#define RGB2PIXEL1555(r,g,b)	\
	((((b) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((r) & 0xf8) >> 3) | 0x8000)

/* create 8 bit 3/3/2 format pixel from RGB triplet*/
#define RGB2PIXEL332(r,g,b)	\
	(((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6))

/* create 8 bit 2/3/3 format pixel from RGB triplet*/
#define RGB2PIXEL233(r,g,b)	\
	((((r) & 0xe0) >> 5) | (((g) & 0xe0) >> 2) | (((b) & 0xc0) >> 0))

#define RGB2PIXEL(r,g,b)	RGB2PIXELABGR(r,g,b)
#define COLORVALTOPIXELVAL(c)	COLOR2PIXELABGR(c)
#define PIXELVALTOCOLORVAL(p)	PIXELABGRTOCOLORVAL(p)
#define PIXEL2RED(p)		PIXELABGRRED(p)
#define PIXEL2GREEN(p)		PIXELABGRGREEN(p)
#define PIXEL2BLUE(p)		PIXELABGRBLUE(p)

/* create single component of 5/6/5format pixel from color byte*/
#define RED2PIXEL565(byte)		(((byte) & 0xf8) << 8)
#define GREEN2PIXEL565(byte)	(((byte) & 0xfc) << 3)
#define BLUE2PIXEL565(byte)		(((byte) & 0xf8) >> 3)

/* create single component of 5/5/5format pixel from color byte*/
#define RED2PIXEL555(byte)		(((byte) & 0xf8) << 7)
#define GREEN2PIXEL555(byte)	(((byte) & 0xf8) << 2)
#define BLUE2PIXEL555(byte)		(((byte) & 0xf8) >> 3)

/* create single component of 1/5/5/5format pixel from color byte*/
#define RED2PIXEL1555(byte)		(((byte) & 0xf8) >> 3)
#define GREEN2PIXEL1555(byte)	(((byte) & 0xf8) << 2)
#define BLUE2PIXEL1555(byte)	(((byte) & 0xf8) << 7)

// 15 bit is taken as 16bit!
#define muldiv255_16bpp		muldiv255_rgb565
#define RED2PIXEL(byte)		RED2PIXEL565(byte)
#define GREEN2PIXEL(byte)	GREEN2PIXEL565(byte)
#define BLUE2PIXEL(byte)	BLUE2PIXEL565(byte)
#define REDMASK(pixel)		((pixel) & 0xf800)
#define GREENMASK(pixel)	((pixel) & 0x07e0)
#define BLUEMASK(pixel)		((pixel) & 0x001f)

#define REDVALUE(rgb)	((rgb) & 0xff)
#define GREENVALUE(rgb) (((rgb) >> 8) & 0xff)
#define BLUEVALUE(rgb)	(((rgb) >> 16) & 0xff)
#define ALPHAVALUE(rgb)	(((rgb) >> 24) & 0xff)

void fbportrait_left_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c);
PIXELVAL fbportrait_left_readpixel(pSD psd, INT32 x, INT32 y);
void fbportrait_left_drawhorzline(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c);
void fbportrait_left_drawvertline(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c);
void fbportrait_left_fillrect(pSD psd, INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c);
void fbportrait_left_blit(pSD dstpsd, INT32 destx, INT32 desty, INT32 w, INT32 h,
	pSD srcpsd, INT32 srcx, INT32 srcy, int op);
void fbportrait_left_convblit_blend_mask_alpha_byte(pSD dstpsd, pBlitParms gc);
void fbportrait_left_convblit_copy_mask_mono_byte_msb(pSD psd, pBlitParms gc);
void fbportrait_left_convblit_copy_mask_mono_byte_lsb(pSD psd, pBlitParms gc);

void fbportrait_right_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c);
PIXELVAL fbportrait_right_readpixel(pSD psd, INT32 x, INT32 y);
void fbportrait_right_drawhorzline(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c);
void fbportrait_right_drawvertline(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c);
void fbportrait_right_fillrect(pSD psd, INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c);
void fbportrait_right_blit(pSD dstpsd, INT32 destx, INT32 desty, INT32 w, INT32 h,
	pSD srcpsd, INT32 srcx, INT32 srcy, int op);
void fbportrait_right_convblit_blend_mask_alpha_byte(pSD dstpsd, pBlitParms gc);
void fbportrait_right_convblit_copy_mask_mono_byte_msb(pSD psd, pBlitParms gc);
void fbportrait_right_convblit_copy_mask_mono_byte_lsb(pSD psd, pBlitParms gc);

void fbportrait_down_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c);
PIXELVAL fbportrait_down_readpixel(pSD psd, INT32 x, INT32 y);
void fbportrait_down_drawhorzline(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c);
void fbportrait_down_drawvertline(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c);
void fbportrait_down_fillrect(pSD psd, INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c);
void fbportrait_down_blit(pSD dstpsd, INT32 destx, INT32 desty, INT32 w, INT32 h,
	pSD srcpsd, INT32 srcx, INT32 srcy, int op);
void fbportrait_down_convblit_blend_mask_alpha_byte(pSD dstpsd, pBlitParms gc);
void fbportrait_down_convblit_copy_mask_mono_byte_msb(pSD psd, pBlitParms gc);
void fbportrait_down_convblit_copy_mask_mono_byte_lsb(pSD psd, pBlitParms gc);

/* ----- 32bpp output -----*/
void convblit_srcover_rgba8888_rgba8888(pSD psd, pBlitParms gc);	// png/tiff 32bpp RGBA srcover
void convblit_copy_rgba8888_rgba8888(pSD psd, pBlitParms gc);		// 32bpp RGBA to 32bpp RGBA copy
void convblit_copy_rgb888_rgba8888(pSD psd, pBlitParms gc);		// png/jpg 24bpp RGB copy

void convblit_srcover_rgba8888_bgra8888(pSD psd, pBlitParms gc);	// png/tiff 32bpp RGBA srcover
void convblit_copy_rgba8888_bgra8888(pSD psd, pBlitParms gc);		// 32bpp RGBA to 32bpp BGRA copy
void convblit_copy_rgb888_bgra8888(pSD psd, pBlitParms gc);		// png/jpg 24bpp RGB copy

void convblit_copy_8888_8888(pSD psd, pBlitParms gc);				// 32bpp to 32bpp copy

/* ----- 24bpp output -----*/
void convblit_srcover_rgba8888_bgr888(pSD psd, pBlitParms gc);
void convblit_copy_rgba8888_bgr888(pSD psd, pBlitParms gc);		// 32bpp RGBA to 24bpp BGR copy
void convblit_copy_rgb888_bgr888(pSD psd, pBlitParms gc);

void convblit_copy_888_888(pSD psd, pBlitParms gc);				// 24bpp to 24bpp copy

void convblit_copy_bgra8888_bgr888(pSD psd, pBlitParms gc);		// 32bpp BGRA to 24bpp BGR copy

/* ----- 16bpp output -----*/
void convblit_srcover_rgba8888_16bpp(pSD psd, pBlitParms gc);
void convblit_copy_rgba8888_16bpp(pSD psd, pBlitParms gc);		// 32bpp RGBA to 16bpp copy
void convblit_copy_rgb888_16bpp(pSD psd, pBlitParms gc);

void convblit_copy_16bpp_16bpp(pSD psd, pBlitParms gc);			// 16bpp to 16bpp copy

/* convblit_mask.c*/
/* 1bpp and 8bpp (alphablend) mask conversion blits - for font display*/

/* ----- 32bpp output -----*/
void convblit_copy_mask_mono_byte_msb_rgba(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_rgba(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_word_msb_rgba(pSD psd, pBlitParms gc);
void convblit_blend_mask_alpha_byte_rgba(pSD psd, pBlitParms gc);

void convblit_copy_mask_mono_byte_msb_bgra(pSD psd, pBlitParms gc);		/* ft2 non-alias*/
void convblit_copy_mask_mono_byte_lsb_bgra(pSD psd, pBlitParms gc);		/* t1lib non-alias*/
void convblit_copy_mask_mono_word_msb_bgra(pSD psd, pBlitParms gc);		/* pcf non-alias*/
void convblit_blend_mask_alpha_byte_bgra(pSD psd, pBlitParms gc);			/* ft2/t1lib alias*/

/* ----- 24bpp output -----*/
void convblit_copy_mask_mono_byte_msb_bgr(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_bgr(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_word_msb_bgr(pSD psd, pBlitParms gc);
void convblit_blend_mask_alpha_byte_bgr(pSD psd, pBlitParms gc);

/* ----- 16bpp output -----*/
void convblit_copy_mask_mono_byte_msb_16bpp(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_16bpp(pSD psd, pBlitParms gc);
void convblit_copy_mask_mono_word_msb_16bpp(pSD psd, pBlitParms gc);
void convblit_blend_mask_alpha_byte_16bpp(pSD psd, pBlitParms gc);

/* convblit_frameb.c*/
/* framebuffer pixel format blits - must handle backwards copy, different rotation code*/
void frameblit_xxxa8888(pSD psd, pBlitParms gc);		/* 32bpp*/
void frameblit_24bpp(pSD psd, pBlitParms gc);			/* 24bpp*/
void frameblit_16bpp(pSD psd, pBlitParms gc);			/* 16bpp*/
void frameblit_8bpp(pSD psd, pBlitParms gc);			/* 8bpp*/

/* framebuffer pixel format stretch blits - different rotation code, no backwards copy*/
void frameblit_stretch_xxxa8888(pSD dstpsd, pBlitParms gc);	/* 32bpp, alpha in byte 4*/
void frameblit_stretch_24bpp(pSD psd, pBlitParms gc);			/* 24 bpp*/
void frameblit_stretch_16bpp(pSD psd, pBlitParms gc);			/* 16 bpp*/
void frameblit_stretch_8bpp(pSD psd, pBlitParms gc);			/* 8 bpp*/

/* these work for src_over and copy only*/
void frameblit_stretch_rgba8888_bgra8888(pSD psd, pBlitParms gc);	/* RGBA -> BGRA*/
void frameblit_stretch_rgba8888_bgr888(pSD psd, pBlitParms gc);	/* RGBA -> BGR*/
void frameblit_stretch_rgba8888_16bpp(pSD psd, pBlitParms gc);	/* RGBA -> 16bpp*/

/* devimage.c*/
void convblit_pal8_rgba8888(pBlitParms gc);
void convblit_pal4_msb_rgba8888(pBlitParms gc);
void convblit_pal1_byte_msb_rgba8888(pBlitParms gc);

/* image_bmp.c*/

/* Conversion blit 24bpp BGR to 24bpp RGB*/
void convblit_bgr888_rgb888(unsigned char *data, int width, int height, int pitch);
/* Conversion blit 32bpp BGRX to 32bpp RGBA 255 alpha*/
void convblit_bgrx8888_rgba8888(unsigned char *data, int width, int height, int pitch);

/* image_tiff.c*/

/* Conversion blit flip y direction 32bpp (upside-down)*/
void convblit_flipy_8888(pBlitParms gc);
#endif
