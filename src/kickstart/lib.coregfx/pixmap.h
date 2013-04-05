#ifndef PIXMAP_H
#define PIXMAP_H
#include "types.h"
#include "rastport.h"
#include "blit.h"

#define	PSF_SCREEN			0x0001	/* screen device*/
#define PSF_MEMORY			0x0002	/* memory device*/
#define PSF_ADDRMALLOC		0x0010	/* psd->addr was malloc'd*/
#define PSF_ADDRSHAREDMEM	0x0020	/* psd->addr is shared memory*/
#define PSF_IMAGEHDR		0x0040	/* psd is actually MWIMAGEHDR*/

#define FPM_Memory		PSF_MEMORY
#define FPM_AllocAddr	PSF_ADDRMALLOC
#define FPM_Displayable	PSF_SCREEN
#define FPM_Framebuffer	PSF_SCREEN
#define FPM_AllocVec	PSF_ADDRMALLOC

#define PF_RGB	   0	/* pseudo, convert from packed 32 bit RGB*/
#define PF_PIXELVAL	   1	/* pseudo, no convert from packed PIXELVAL*/
#define PF_PALETTE	   2	/* pixel is packed 8 bits 1, 4 or 8 pal index*/
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
#define IF_15BPP			0x0000000FL
#define IF_16BPP			0x00000010L
#define IF_24BPP			0x00000018L
#define IF_32BPP			0x00000020L
#define IF_BPPMASK		0x0000003FL

/* monochrome bitmap formats*/
#define IF_MONO			0x00000040L
#define IF_HASALPHA		0x00000080L
#define IF_BYTEDATA		0x00000100L
#define IF_LEWORDDATA		0x00000200L		/* 16-bit little endian format (retrofit format)*/
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

/* MWIF_BGR233*/
#define RMASK233	0x07
#define GMASK233	0x38
#define BMASK233	0xc0
#define AMASK233	0x00

/* MWIF_RGB332*/
#define RMASK332	0xe0
#define GMASK332	0x1c
#define BMASK332	0x03
#define AMASK332	0x00

/* MWIF_RGB555*/
#define RMASK555	0x7c00
#define GMASK555	0x03e0
#define BMASK555	0x001f
#define AMASK555	0x8000

/* MWIF_RGB565*/
#define RMASK565	0xf800
#define GMASK565	0x07e0
#define BMASK565	0x001f
#define AMASK565	0x0000

/* MWIF_BGR888*/
#define RMASKBGR	0xff0000
#define GMASKBGR	0x00ff00
#define BMASKBGR	0x0000ff
#define AMASKBGR	0x000000

/* MWIF_BGRA8888*/
#define RMASKBGRA	0x00ff0000
#define GMASKBGRA	0x0000ff00
#define BMASKBGRA	0x000000ff
#define AMASKBGRA	0xff000000

/* MWIF_RGBA8888*/
#define RMASKRGBA	0x000000ff
#define GMASKRGBA	0x0000ff00
#define BMASKRGBA	0x00ff0000
#define AMASKRGBA	0xff000000
typedef void (*BLITFUNC)(struct PixMap*, pCGfxBlitParms);		/* proto for blitter functions*/

typedef struct PixMap {
	UINT32	flags;
	UINT32	xvirtres, yvirtres;
	UINT32	planes;
	UINT32	bpp;
	UINT32	data_format;
	UINT32	pitch;
	UINT8	*addr;
	INT32	palsize;
	void	*palette;
	INT32	transcolor;
	UINT32	xres, yres;
	UINT32	size;
	UINT32	ncolors;
	UINT32	pixtype;
	
	int	portrait;
	void	(*_GetScreenInfo)(struct CRastPort *rp,pCGfxScreenInfo psi);

	void	(*_DrawPixel)(struct CRastPort *psd, INT32 x,INT32 y,UINT32 c);
	UINT32	(*_ReadPixel)(struct CRastPort *psd, INT32 x,INT32 y);
	void	(*_DrawHorzLine)(struct CRastPort *psd, INT32 x1,INT32 x2,INT32 y, UINT32 c);
	void	(*_DrawVertLine)(struct CRastPort *psd, INT32 x,INT32 y1,INT32 y2, UINT32 c);
	void	(*_FillRect)(struct CRastPort *psd, INT32 x1,INT32 y1,INT32 x2,INT32 y2, UINT32 c);
	void	(*_Update)(struct PixMap *pix, INT32 x, INT32 y, INT32 width, INT32 height);
	void	(*BlitFallback)(struct PixMap * destpsd,INT32 destx,INT32 desty,INT32 w,INT32 h,
							struct PixMap * srcpsd,INT32 srcx,INT32 srcy,int op);
	BLITFUNC FrameBlit;
	BLITFUNC FrameStretchBlit;
	/* fast conversion blits for text and images*/
	BLITFUNC BlitCopyMaskMonoByteMSB;				/* ft non-alias*/
	BLITFUNC BlitCopyMaskMonoByteLSB;				/* t1 non-alias*/
	BLITFUNC BlitCopyMaskMonoWordMSB;				/* core/pcf non-alias*/
	BLITFUNC BlitBlendMaskAlphaByte;				/* ft2/t1 antialias*/
	BLITFUNC BlitCopyRGBA8888;					/* GdArea RGBA MWPF_RGB image copy*/
	BLITFUNC BlitSrcOverRGBA8888;					/* png RGBA image w/alpha*/
	BLITFUNC BlitCopyRGB888;						/* png RGB image no alpha*/
	BLITFUNC BlitStretchRGBA8888;					/* conversion stretch blit for RGBA src*/

} PixMap;


#endif
