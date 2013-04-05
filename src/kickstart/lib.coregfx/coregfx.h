#ifndef coregfx_h
#define coregfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "resident.h"
//#include "font.h"

#include "exec_funcs.h"
#include "coregfx_funcs.h"
//void SVGA_DrawPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop)

#define	MAX_CURSOR_SIZE				32		/* maximum cursor x and y size*/
#define	MAX_CURSOR_BUFLEN			IMAGE_SIZE(MAX_CURSOR_SIZE, MAX_CURSOR_SIZE)

#define IMAGE_WORDS(x)					(((x)+15)/16)
#define IMAGE_BYTES(x)					(IMAGE_WORDS(x)*sizeof(UINT16))
#define	IMAGE_SIZE(width, height) 	((height) * (((width) + IMAGE_BITSPERIMAGE - 1) / IMAGE_BITSPERIMAGE))
#define	IMAGE_BITSPERIMAGE			(sizeof(UINT16) * 8)
#define	IMAGE_BITVALUE(n)				((UINT16) (((UINT16) 1) << (n)))
#define	IMAGE_FIRSTBIT				(IMAGE_BITVALUE(IMAGE_BITSPERIMAGE - 1))
#define	IMAGE_NEXTBIT(m)				((UINT16) ((m) >> 1))
#define	IMAGE_TESTBIT(m)				((m) & IMAGE_FIRSTBIT)  /* use with shiftbit*/
#define	IMAGE_SHIFTBIT(m)				((UINT16) ((m) << 1))  /* for testbit*/

/* no color, used for transparency, should not be 0, -1 or any MWRGB color*/
#define NOCOLOR	0x01000000L			/* MWRGBA(1, 0, 0, 0)*/
#define PORTRAIT_NONE		0x00	/* hw framebuffer, no rotation*/
#define PORTRAIT_LEFT		0x01	/* rotate left*/
#define	PORTRAIT_RIGHT		0x02	/* rotate right*/
#define PORTRAIT_DOWN		0x04	/* upside down*/

// Needs to stay here because of MAX_CURSOR_SIZE definition
#include "cursor.h"

typedef struct CGfxCursor {
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
} CGfxCursor;

typedef struct CgfxPalEntry{
	UINT8	r;
	UINT8	g;
	UINT8	b;
	UINT8 _padding;
} CgfxPalEntry, *pCGfxPalEntry;

typedef struct CoreGfxBase {
	struct Library	Library;
	APTR	VgaGfxBase;	
	APTR	SysBase;
	APTR	RegionBase;
	CGfxCursor	Cursor;
	struct View *ActiveView;
	struct CGfxCoreFont *builtin_fonts;
	struct CGfxCoreFont *user_builtin_fonts;
} CoreGfxBase, *pCoreGfxBase;

/* Line modes */
#define LINE_SOLID      0
#define LINE_ONOFF_DASH 1

/* Fill mode  */
#define FILL_SOLID          0  
#define FILL_STIPPLE        1  
#define FILL_OPAQUE_STIPPLE 2  
#define FILL_TILE           3

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

#define ARC		0x0001	/* arc*/
#define OUTLINE	0x0002
#define ARCOUTLINE	0x0003	/* arc + outline*/
#define PIE		0x0004	/* pie (filled)*/
#define ELLIPSE	0x0008	/* ellipse outline*/
#define ELLIPSEFILL	0x0010	/* ellipse filled*/

#define ARGB(a,r,g,b)	((UINT32)(((unsigned char)(r)|\
				(((UINT32)(unsigned char)(g))<<8))|\
				(((UINT32)(unsigned char)(b))<<16)|\
				(((UINT32)(unsigned char)(a))<<24)))
#define RGB(r,g,b)		ARGB(255,(r),(g),(b))		/* argb 255 alpha*/
#define A0RGB(r,g,b)	ARGB(0,(r),(g),(b))		/* rgb 0 alpha*/


static inline void memcpy(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsb" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

#endif
