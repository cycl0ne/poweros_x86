#ifndef vgagfx_h
#define vgagfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "expansionbase.h"
#include "resident.h"

#include "svga_reg.h"
#include "pci.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"

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

void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ); 
void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, INT32 x, INT32 y, INT32 width, INT32 height );
APTR SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp );
void SVGA_WaitForFB(VgaGfxBase *VgaGfxBase);


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

APTR SVGA_SetDisplayMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp);
UINT32 SVGA_CheckCapabilities(VgaGfxBase *VgaGfxBase);
void SVGA_FifoUpdateFullscreen(VgaGfxBase *VgaGfxBase);
void SVGA_CopyRect(VgaGfxBase *VgaGfxBase, UINT32 srcX, UINT32 srcY, UINT32 dstX, UINT32 dstY, UINT32 width, UINT32 height ); 

#endif
