#ifndef vgagfx_h
#define vgagfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "expansionbase.h"
#include "resident.h"
#include "pixmap.h"
#include "blit.h"

#include "svga_reg.h"
#include "pci.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"

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


typedef struct VgaGfxBase {
	struct Library	Library;
	SysBase			*SysBase;
	ExpansionBase	*ExpansionBase;
	PCIAddress		pciAddr;
//	UINT32			ioBase;
//	UINT32			*fifoMem;
//	UINT8			*fbMem;
//	UINT32     		fifoSize;
//	UINT32     		fbSize;
//	UINT32			vramSize;

//	UINT32			deviceVersionId;
	
	UINT32			width;
	UINT32			height;
	UINT32			bpp;
	UINT32			pitch;
////////////////////////////////////////////////////
	UINT16			vendorId;
	UINT16			deviceId;
	UINT8			revision;
	UINT32			maxWidth;
	UINT32			maxHeight;
	void			*fbDma;
	UINT32			fbSize;
	void			*fifoDma;
	UINT32			fifoSize;
	UINT32			fifoMin;
	UINT32			capabilities;
	UINT32			fifoCapabilities;
	UINT32			fifoFlags;

	/* For registers access */
	volatile UINT16			indexPort;
	volatile UINT16			valuePort;

	/* Mapped areas */
	void			*fb;
	void			*fifo;

	/* This changes when we switch to another mode */
	UINT32			fbOffset;
	UINT32			bytesPerRow;

	/* Current display mode */

	//Benaphore		engineLock;
	//Benaphore		fifoLock;
	UINT32			fifoNext;
	
	/* Cursor state */
	BOOL			cursorShow;
	UINT16			cursorX;
	UINT16			cursorY;
} VgaGfxBase;

#define	MWROP_COPY			0	/* src*/
#define	MWROP_XOR			1	/* src ^ dst*/
#define	MWROP_OR			2	/* src | dst (PAINT)*/
#define	MWROP_AND			3	/* src & dst (MASK)*/
#define	MWROP_CLEAR			4	/* 0*/
#define	MWROP_SET			5	/* ~0*/
#define	MWROP_EQUIV			6	/* ~(src ^ dst)*/
#define	MWROP_NOR			7	/* ~(src | dst)*/
#define	MWROP_NAND			8	/* ~(src & dst)*/
#define	MWROP_INVERT		9	/* ~dst*/
#define	MWROP_COPYINVERTED	10	/* ~src*/
#define	MWROP_ORINVERTED	11	/* ~src | dst*/
#define	MWROP_ANDINVERTED	12	/* ~src & dst (SUBTRACT)*/
#define MWROP_ORREVERSE		13	/* src | ~dst*/
#define	MWROP_ANDREVERSE	14	/* src & ~dst*/
#define	MWROP_NOOP			15	/* dst*/
#define	MWROP_XOR_FGBG		16	/* src ^ background ^ dst (Java XOR mode)*/
#define MWROP_SIMPLE_MAX 	16	/* last non-compositing rop*/

/* Porter-Duff compositing operations.  Only SRC, CLEAR and SRC_OVER are commonly used*/
#define	MWROP_SRC			MWROP_COPY
#define	MWROP_DST			MWROP_NOOP
//#define MWROP_CLEAR		MWROP_CLEAR
#define	MWROP_SRC_OVER		17	/* dst = alphablend src,dst*/
#define	MWROP_DST_OVER		18
#define	MWROP_SRC_IN		19
#define	MWROP_DST_IN		20
#define	MWROP_SRC_OUT		21
#define	MWROP_DST_OUT		22
#define	MWROP_SRC_ATOP		23
#define	MWROP_DST_ATOP		24
#define	MWROP_PORTERDUFF_XOR 25
#define MWROP_SRCTRANSCOPY	26	/* copy src -> dst except for transparent color in src*/
#define	MWROP_MAX			26	/* last non-blit rop*/

/* blit ROP modes in addtion to MWROP_xxx */
#define MWROP_BLENDCONSTANT		32	/* alpha blend src -> dst with constant alpha*/
#define MWROP_BLENDFGBG			33	/* alpha blend fg/bg color -> dst with src alpha channel*/
//#define MWROP_BLENDCHANNEL	35	/* alpha blend src -> dst with separate per pixel alpha chan*/
//#define MWROP_STRETCH			36	/* stretch src -> dst*/
#define MWROP_USE_GC_MODE		255 /* use current GC mode for ROP.  Nano-X CopyArea only*/

//void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ); 
void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, INT32 x, INT32 y, INT32 width, INT32 height );
APTR SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp );
void SVGA_WaitForFB(VgaGfxBase *VgaGfxBase);
/*
void SVGA_DrawHorzLine32(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 x2, UINT32 y, UINT32 c, UINT32 rop);
void SVGA_DrawHorzLine16(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 x2, UINT32 y, UINT16 c, UINT32 rop);
void SVGA_DrawFillRect32(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, UINT32 c, UINT32 rop);
void SVGA_DrawFillRect16(VgaGfxBase *VgaGfxBase, UINT32 x1, UINT32 y1, UINT32 x2, UINT32 y2, UINT16 c, UINT32 rop);
UINT32 SVGA_ReadPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y);
UINT32 SVGA_ReadPixel24(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y);
UINT16 SVGA_ReadPixel16(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y);
UINT8 SVGA_ReadPixel8(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y);
void SVGA_DrawPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop);
void SVGA_DrawPixel24(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop);
void SVGA_DrawPixel16(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT16 c, UINT32 rop);
void SVGA_DrawPixel8(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT8 c, UINT32 rop);
*/

APTR SVGA_SetDisplayMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp);
UINT32 SVGA_CheckCapabilities(VgaGfxBase *VgaGfxBase);
void SVGA_FifoUpdateFullscreen(VgaGfxBase *VgaGfxBase);
void SVGA_CopyRect(VgaGfxBase *VgaGfxBase, UINT32 srcX, UINT32 srcY, UINT32 dstX, UINT32 dstY, UINT32 width, UINT32 height ); 

void frameblit_stretch_xxxa8888(PixMap *psd, pCGfxBlitParms gc);
void frameblit_stretch_24bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_stretch_16bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_stretch_8bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_stretch_rgba8888_bgra8888(PixMap * psd, pCGfxBlitParms gc);
void frameblit_stretch_rgba8888_bgr888(PixMap * psd, pCGfxBlitParms gc);
void frameblit_stretch_rgba8888_16bpp(PixMap * psd, pCGfxBlitParms gc);

void frameblit_8bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_16bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_24bpp(PixMap * psd, pCGfxBlitParms gc);
void frameblit_xxxa8888(PixMap *psd, pCGfxBlitParms gc);

void convblit_srcover_rgba8888_rgba8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgba8888_rgba8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgb888_rgba8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_srcover_rgba8888_bgra8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgba8888_bgra8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgb888_bgra8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_8888_8888(PixMap *psd, pCGfxBlitParms gc);
void convblit_srcover_rgba8888_bgr888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgba8888_bgr888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgb888_bgr888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_888_888(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_bgra8888_bgr888(PixMap *psd, pCGfxBlitParms gc);
void convblit_srcover_rgba8888_16bpp(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgba8888_16bpp(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_rgb888_16bpp(PixMap *psd, pCGfxBlitParms gc);
void convblit_copy_16bpp_16bpp(PixMap *psd, pCGfxBlitParms gc);

void convblit_copy_mask_mono_byte_lsb_rgba(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_bgra(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_bgr(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_lsb_16bpp(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_word_msb_rgba(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_word_msb_bgra(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_word_msb_bgr(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_word_msb_16bpp(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_msb_rgba(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_msb_bgra(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_msb_bgr(PixMap * psd, pCGfxBlitParms gc);
void convblit_copy_mask_mono_byte_msb_16bpp(PixMap * psd, pCGfxBlitParms gc);
void convblit_blend_mask_alpha_byte_rgba(PixMap * psd, pCGfxBlitParms gc);
void convblit_blend_mask_alpha_byte_bgra(PixMap * psd, pCGfxBlitParms gc);
void convblit_blend_mask_alpha_byte_bgr(PixMap * psd, pCGfxBlitParms gc);
void convblit_blend_mask_alpha_byte_16bpp(PixMap * psd, pCGfxBlitParms gc);


#endif
