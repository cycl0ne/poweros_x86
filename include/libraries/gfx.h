/**
* File: /gfx．h
* User: cycl0ne
* Date: 2014-11-26
* Time: 04:03 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef GFX_H
#define GFX_H
#include "types.h"
#include "regions.h"

typedef struct CoreFont *pCoreFont;
typedef struct PixMap *pSD;
typedef UINT32 PIXELVAL;
typedef UINT32 COLORVAL;
	
#define CLIP_VISIBLE		0
#define CLIP_INVISIBLE		1
#define CLIP_PARTIAL		2

typedef struct Point {
	INT32 x;
	INT32 y;
} Point_t, *pPoint;

typedef struct Stipple {
	INT32		width;
	INT32		height;
	UINT16 		*bitmap;
} Stipple_t, *pStipple;

typedef struct Tile {
	INT32			width;
	INT32			height;
	pSD				pixmap;
	struct PaintBox *rp;
} Tile_t, *pTile;

#define PORTRAIT_NONE		0x00	/* hw framebuffer, no rotation*/
#define PORTRAIT_LEFT		0x01	/* rotate left*/
#define	PORTRAIT_RIGHT		0x02	/* rotate right*/
#define PORTRAIT_DOWN		0x04	/* upside down*/

typedef struct PalEntry {
	INT8		r;
	INT8		g;
	INT8		b;
	INT8		a;
} PalEntry_t, *pPalEntry;

/* Line modes */
#define LINE_SOLID      0
#define LINE_ONOFF_DASH 1

/* Fill mode  */
#define FILL_SOLID          0  
#define FILL_STIPPLE        1  
#define FILL_OPAQUE_STIPPLE 2  
#define FILL_TILE           3

#define ARC		0x0001	/* arc*/
#define OUTLINE	0x0002
#define ARCOUTLINE	0x0003	/* arc + outline*/
#define PIE		0x0004	/* pie (filled)*/
#define ELLIPSE	0x0008	/* ellipse outline*/
#define ELLIPSEFILL	0x0010	/* ellipse filled*/

/* Rasteroperations (ROP) */
#define	ROP_COPY			0	/* src*/
#define	ROP_XOR			1	/* src ^ dst*/
#define	ROP_OR			2	/* src | dst (PAINT)*/
#define	ROP_AND			3	/* src & dst (MASK)*/
#define	ROP_CLEAR			4	/* 0*/
#define	ROP_SET			5	/* ~0*/
#define	ROP_EQUIV			6	/* ~(src ^ dst)*/
#define	ROP_NOR			7	/* ~(src | dst)*/
#define	ROP_NAND			8	/* ~(src & dst)*/
#define	ROP_INVERT		9	/* ~dst*/
#define	ROP_COPYINVERTED	10	/* ~src*/
#define	ROP_ORINVERTED	11	/* ~src | dst*/
#define	ROP_ANDINVERTED	12	/* ~src & dst (SUBTRACT)*/
#define ROP_ORREVERSE		13	/* src | ~dst*/
#define	ROP_ANDREVERSE	14	/* src & ~dst*/
#define	ROP_NOOP			15	/* dst*/
#define	ROP_XOR_FGBG		16	/* src ^ background ^ dst (Java XOR mode)*/
#define ROP_SIMPLE_MAX 	16	/* last non-compositing rop*/

/* Porter-Duff compositing operations.  Only SRC, CLEAR and SRC_OVER are commonly used*/
#define	ROP_SRC			ROP_COPY
#define	ROP_DST			ROP_NOOP
#define	ROP_SRC_OVER		17	/* dst = alphablend src,dst*/
#define	ROP_DST_OVER		18
#define	ROP_SRC_IN		19
#define	ROP_DST_IN		20
#define	ROP_SRC_OUT		21
#define	ROP_DST_OUT		22
#define	ROP_SRC_ATOP		23
#define	ROP_DST_ATOP		24
#define	ROP_PORTERDUFF_XOR 25
#define ROP_SRCTRANSCOPY	26	/* copy src -> dst except for transparent color in src*/
#define	ROP_MAX			26	/* last non-blit rop*/
#define ROP_BLENDCONSTANT		32	/* alpha blend src -> dst with constant alpha*/
#define ROP_BLENDFGBG			33	/* alpha blend fg/bg color -> dst with src alpha channel*/
#define ROP_USE_GC_MODE		255 /* use current GC mode for ROP.  Nano-X CopyArea only*/

typedef struct BlitParms {
	INT32			op;				/* MWROP operation requested*/
	INT32			data_format;	/* MWIF_ image data format*/
	INT32			width, height;	/* width and height for src and dest*/
	INT32			dstx, dsty;		/* dest x, y*/
	INT32			srcx, srcy;		/* source x, y*/
	unsigned int 	src_pitch;		/* source row length in bytes*/
	COLORVAL		fg_colorval;	/* fg color, MWCOLORVAL 0xAARRGGBB format*/
	COLORVAL		bg_colorval;
	UINT32			fg_pixelval;	/* fg color, hw pixel format*/
	UINT32			bg_pixelval;
	BOOL			usebg;			/* set =1 to draw background*/
	void *			data;			/* input image data GdConversionBlit*/

	/* these items filled in by GdConversionBlit*/
	void *			data_out;		/* output image from conversion blits subroutines*/
	unsigned int 	dst_pitch;		/* dest row length in bytes*/

	/* used by GdBlit and GdStretchBlit for GdCheckCursor and fallback blit*/
	pSD				srcpsd;			/* source psd for psd->psd blits*/

	/* used by frameblits only*/
	INT32			src_xvirtres;	/* srcpsd->x/y virtres, used in frameblit for src coord rotation*/
	INT32			src_yvirtres;

	/* used in stretch blits only*/
	INT32			src_x_step;		/* normal steps in source image*/
	INT32			src_y_step;
	INT32			src_x_step_one;	/* 1-unit steps in source image*/
	INT32			src_y_step_one;
	INT32			err_x_step;		/* 1-unit error steps in source image*/
	INT32			err_y_step;
	INT32			err_y;			/* source coordinate error tracking*/
	INT32			err_x;
	INT32			x_denominator;	/* denominator fraction*/
	INT32			y_denominator;

	/* used in palette conversions only*/
	pPalEntry		palette;		/* palette for image*/
	UINT32			transcolor;		/* transparent color in image*/
}BlitParms_t, *pBlitParms;

typedef void (*BLITFUNC)(pSD, pBlitParms);

typedef struct SubDriver
{
	void	 (*DrawPixel)(pSD psd, INT32 x, INT32 y, PIXELVAL c);
	PIXELVAL (*ReadPixel)(pSD psd, INT32 x, INT32 y);
	void	 (*DrawHorzLine)(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c);
	void	 (*DrawVertLine)(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c);
	void	 (*FillRect)(pSD psd,INT32 x1,INT32 y1,INT32 x2, INT32 y2,PIXELVAL c);
	void	 (*BlitFallback)(pSD destpsd, INT32 destx, INT32 desty, INT32 w, INT32 h,
					 pSD srcpsd, INT32 srcx,INT32 srcy,INT32 op);

	BLITFUNC FrameBlit;
	BLITFUNC FrameStretchBlit;
	BLITFUNC BlitCopyMaskMonoByteMSB;
	BLITFUNC BlitCopyMaskMonoByteLSB;
	BLITFUNC BlitCopyMaskMonoWordMSB;
	BLITFUNC BlitBlendMaskAlphaByte;
	BLITFUNC BlitCopyRGBA8888;
	BLITFUNC BlitSrcOverRGBA8888;
	BLITFUNC BlitCopyRGB888;
	BLITFUNC BlitStretchRGBA8888;
} SubDriver_t, *pSubDriver;

#define	PMF_SCREEN			0x0001	/* screen device*/
#define PMF_MEMORY			0x0002	/* memory device*/
#define PMF_ADDRMALLOC		0x0010	/* psd->addr was malloc'd*/
#define PMF_ADDRSHAREDMEM	0x0020	/* psd->addr is shared memory*/
#define PMF_IMAGEHDR		0x0040	/* psd is actually MWIMAGEHDR*/

typedef struct PixMap {
	INT32		flags;
	INT32		xvirtres;
	INT32		yvirtres;
	INT32		planes;
	INT32		bpp;
	INT32		data_format;
	UINT32		pitch;
	void*		addr;
	INT32		palsize;
	pPalEntry	palette;
	INT32		transcolor;
	INT32		xres;
	INT32		yres;
	UINT32		size;
	INT32		ncolors;
	INT32		pixtype;

	pCoreFont	builtin_fonts;

	INT32		portrait;
	
	PIXELVAL	background;
	INT32		mode;

	pSubDriver	orgsubdriver;
	pSubDriver	left_subdriver;
	pSubDriver	right_subdriver;
	pSubDriver	down_subdriver;
	
	
	
	void	(*DrawPixel)(pSD psd, INT32 x, INT32 y, PIXELVAL c);
	PIXELVAL 	(*ReadPixel)(pSD psd, INT32 x, INT32 y);
	void	(*DrawHorzLine)(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c);
	void	(*DrawVertLine)(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c);
	void	(*FillRect)(pSD psd,INT32 x1,INT32 y1,INT32 x2, INT32 y2,PIXELVAL c);
	void	(*BlitFallback)(pSD destpsd, INT32 destx, INT32 desty, INT32 w, INT32 h,
					 pSD srcpsd, INT32 srcx,INT32 srcy,INT32 op);
	void	(*Update)(pSD psd, INT32 x, INT32 y, INT32 width, INT32 height);
	BLITFUNC FrameBlit;
	BLITFUNC FrameStretchBlit;
	BLITFUNC BlitCopyMaskMonoByteMSB;
	BLITFUNC BlitCopyMaskMonoByteLSB;
	BLITFUNC BlitCopyMaskMonoWordMSB;
	BLITFUNC BlitBlendMaskAlphaByte;
	BLITFUNC BlitCopyRGBA8888;
	BLITFUNC BlitSrcOverRGBA8888;
	BLITFUNC BlitCopyRGB888;
	BLITFUNC BlitStretchRGBA8888;
} PixMap_t, *pPixMap;

#define PF_RGB			 0	/* pseudo, convert from packed 32 bit RGB*/
#define PF_PIXELVAL		 1	/* pseudo, no convert from packed PIXELVAL*/
#define PF_PALETTE		 2	/* pixel is packed 8 bits 1, 4 or 8 pal index*/
#define PF_TRUECOLOR888  4	/* pixel is packed 24 bits R/G/B RGB truecolor*/
#define PF_TRUECOLOR565  5	/* pixel is packed 16 bits 5/6/5 RGB truecolor*/
#define PF_TRUECOLOR555  6	/* pixel is packed 16 bits 5/5/5 RGB truecolor*/
#define PF_TRUECOLOR332  7	/* pixel is packed  8 bits 3/3/2 RGB truecolor*/
#define PF_TRUECOLOR8888 8	/* pixel is packed 32 bits A/R/G/B ARGB truecolor with alpha */
#define PF_TRUECOLOR0888 8	/* deprecated*/
#define PF_TRUECOLOR233  9	/* pixel is packed  8 bits 2/3/3 BGR truecolor*/
#define PF_HWPIXELVAL   10	/* pseudo, no convert, pixels are in hw format*/
#define PF_TRUECOLORABGR 11	/* pixel is packed 32 bits A/B/G/R ABGR truecolor with alpha */
#define PF_TRUECOLOR1555 12   /* pixel is packed 16 bits 1/5/5/5 NDS truecolor */

/* bits per pixel*/
#define IF_1BPP			0x00000001L
#define IF_8BPP			0x00000008L
#define IF_15BPP		0x0000000FL
#define IF_16BPP		0x00000010L
#define IF_24BPP		0x00000018L
#define IF_32BPP		0x00000020L
#define IF_BPPMASK		0x0000003FL

/* monochrome bitmap formats*/
#define IF_MONO			0x00000040L
#define IF_HASALPHA		0x00000080L
#define IF_BYTEDATA		0x00000100L
#define IF_LEWORDDATA	0x00000200L		/* 16-bit little endian format (retrofit format)*/
//#define IF_BEQUADDATA	0x00000400L		/* 32-bit big endian format*/
#define IF_MSBFIRST		0x00000800L		/* highest bit displayed leftmost*/
#define IF_LSBFIRST		0x00001000L		/* lowest bit displayed leftmost*/
#define IF_MONOBYTEMSB	(IF_1BPP | IF_MONO | IF_BYTEDATA | IF_MSBFIRST)
#define IF_MONOBYTELSB	(IF_1BPP | IF_MONO | IF_BYTEDATA | IF_LSBFIRST)
#define IF_MONOWORDMSB	(IF_1BPP | IF_MONO | IF_LEWORDDATA | IF_MSBFIRST)
//#define IF_MONOQUADMSB	(IF_1BPP | IF_MONO | IF_BEQUADDATA | IF_MSBFIRST)
#define IF_ALPHABYTE		(IF_8BPP | IF_HASALPHA| IF_BYTEDATA)

/* framebuffer and image data formats - yet unsupported formats commented out*/
#define IF_BGRA8888	(0x00010000L|IF_HASALPHA) /* 32bpp BGRA image byte order (old TRUECOLOR8888)*/
#define IF_RGBA8888	(0x00020000L|IF_HASALPHA)	/* 32bpp RGBA image byte order (old TRUECOLORABGR)*/
//#define IF_ARGB8888	(0x00030000L|IF_HASALPHA)	/* 32bpp ARGB image byte order (new)*/
//#define IF_ABGR8888	(0x00040000L|IF_HASALPHA)	/* 32bpp ABGR image byte order (new)*/
//#define IF_BGRX8888	(0x00050000L|IF_HASALPHA)	/* 32bpp BGRX image order no alpha (new)*/
#define IF_BGR888		 0x00060000L		/* 24bpp BGR image byte order  (old TRUECOLOR888)*/
#define IF_RGB888		 0x00070000L		/* 24bpp RGB image byte order  (png no alpha)*/
#define IF_RGB565		 0x00080000L		/* 16bpp 5/6/5 RGB packed l.endian (old TRUECOLOR565)*/
//#define IF_RGB565_BE 0x00090000L		/* 16bpp 5/6/5 RGB packed b.endian (new)*/
#define IF_RGB555		 0x000A0000L		/* 16bpp 5/5/5 RGB packed l.endian (old TRUECOLOR555)*/
//#define IF_RGB555_BE 0x000B0000L		/* 16bpp 5/5/5 RGB packed b.endian (new)*/
#define IF_RGB1555	 0x000C0000L		        /* 16bpp 1/5/5/5 NDS color format */
//#define IF_BGR555_BE 0x000D0000L		/* 16bpp 5/5/5 BGR packed b.endian (new)*/
#define IF_RGB332		 0x000E0000L		/*  8bpp 3/3/2 RGB packed (old TRUECOLOR332)*/
#define IF_BGR233		 0x000F0000L		/*  8bpp 2/3/3 BGR packed (old TRUECOLOR233)*/
#define IF_PAL1		 IF_MONOBYTEMSB	/*  1bpp palette (old PF_PALETTE)*/
#define IF_PAL2		 0x00200000L		/*  2bpp palette (old PF_PALETTE)*/
#define IF_PAL4		 0x00400000L		/*  4bpp palette (old PF_PALETTE)*/
#define IF_PAL8		 0x00800000L		/*  8bpp palette (old PF_PALETTE)*/


typedef struct PaintBox {
	pSD			pb_PixMap;
	INT32		pb_ClipMinX;
	INT32		pb_ClipMinY;
	INT32		pb_ClipMaxX;
	INT32		pb_ClipMaxY;
	BOOL		pb_ClipResult;
	pClipRegion	pb_ClipRegion;
	
	PIXELVAL	pb_Foreground;
	PIXELVAL	pb_Background;
	BOOL		pb_useBg;
	INT32		pb_Mode;
//	PalEntry	pb_palette[256];
//	INT32		pb_

	COLORVAL	pb_ForegroundRGB;
	COLORVAL	pb_BackgroundRGB;
	INT32		pb_DashMask;
	INT32		pb_DashCount;
	INT32		pb_FillMode;
	Stipple_t	pb_Stipple;
	Tile_t		pb_Tile;
	Point_t		pb_ts_Offset;
	INT32		ts_origin_x;
	INT32 		ts_origin_y;
	
} PaintBox_t, *pPB;

typedef UINT16	IMAGEBITS;

#define IMAGE_WORDS(x)					(((x)+15)/16)
#define IMAGE_BYTES(x)					(IMAGE_WORDS(x)*sizeof(IMAGEBITS))
#define	IMAGE_SIZE(width, height) 		((height) * (((width) + IMAGE_BITSPERIMAGE - 1) / IMAGE_BITSPERIMAGE))
#define	IMAGE_BITSPERIMAGE				(sizeof(IMAGEBITS) * 8)
#define	IMAGE_BITVALUE(n)				((IMAGEBITS) (((IMAGEBITS) 1) << (n)))
#define	IMAGE_FIRSTBIT					(IMAGE_BITVALUE(IMAGE_BITSPERIMAGE - 1))
#define	IMAGE_NEXTBIT(m)				((IMAGEBITS) ((m) >> 1))
#define	IMAGE_TESTBIT(m)				((m) & IMAGE_FIRSTBIT)  /* use with shiftbit*/
#define	IMAGE_SHIFTBIT(m)				((IMAGEBITS) ((m) << 1))  /* for testbit*/

#define	MAX_CURSOR_SIZE				32		/* maximum cursor x and y size*/
#define	MAX_CURSOR_BUFLEN			IMAGE_SIZE(MAX_CURSOR_SIZE, MAX_CURSOR_SIZE)

typedef struct Cursor {
	INT32		width;			/* cursor width in pixels*/
	INT32		height;			/* cursor height in pixels*/
	INT32		hotx;			/* relative x pos of hot spot*/
	INT32		hoty;			/* relative y pos of hot spot*/
	UINT32		fgcolor;		/* foreground color*/
	UINT32		bgcolor;		/* background color*/
	UINT16		image[MAX_CURSOR_SIZE*2];/* cursor image bits*/
	UINT16		mask[MAX_CURSOR_SIZE*2];/* cursor mask bits*/
} Cursor_T;

#define ARGB(a,r,g,b)	((UINT32)(((unsigned char)(r)|\
				(((UINT32)(unsigned char)(g))<<8))|\
				(((UINT32)(unsigned char)(b))<<16)|\
				(((UINT32)(unsigned char)(a))<<24)))
#define RGB(r,g,b)		ARGB(255,(r),(g),(b))		/* argb 255 alpha*/
#define A0RGB(r,g,b)	ARGB(0,(r),(g),(b))		/* rgb 0 alpha*/

#endif
