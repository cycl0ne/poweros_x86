#ifndef COREGFX_FUNCS_H
#define COREGFX_FUNCS_H

#include "types.h"
#include "coregfx.h"
#include "rastport.h"
#include "regions.h"

#if 0
INT32 cgfx_ClipArea(CoreGfxBase *CoreGfxBase, 		CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2);
BOOL cgfx_ClipPoint(CoreGfxBase *CoreGfxBase, 		CRastPort *rp, INT32 x, INT32 y);
void cgfx_SetClipRegion(CoreGfxBase *CoreGfxBase, 	CRastPort *rp, ClipRegion *reg);

UINT32 cgfx_SetMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode);
UINT32 cgfx_SetFillMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode);
BOOL cgfx_SetUseBackground(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, BOOL flag);
UINT32 cgfx_SetForegroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg);
UINT32 cgfx_SetBackgroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg);
UINT32 cgfx_SetForegroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg);
UINT32 cgfx_SetBackgroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg);
void cgfx_SetDash(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 *mask, UINT32 *count);

void cgfx_FixCursor(	CoreGfxBase *CoreGfxBase, CRastPort *rp);
void cgfx_CheckCursor(	CoreGfxBase *CoreGfxBase, CRastPort *rp,INT32 x1,INT32 y1,INT32 x2,INT32 y2);
INT32 cgfx_ShowCursor(	CoreGfxBase *CoreGfxBase, CRastPort *rp);
INT32 cgfx_HideCursor(	CoreGfxBase *CoreGfxBase, CRastPort *rp);
void cgfx_SetCursor(	CoreGfxBase *CoreGfxBase, struct Cursor *pcursor);
BOOL cgfx_GetCursorPos(	CoreGfxBase *CoreGfxBase, INT32 *px, INT32 *py);
void cgfx_MoveCursor(	CoreGfxBase *CoreGfxBase, INT32 newx, INT32 newy);

UINT32 cgfx_FindNearestColor(CRastPort *rp, int size, UINT32 cr);
UINT32 cgfx_FindColor(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, UINT32 c);

void cgfx_Point(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
void cgfx_Line(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint) 
void cgfx_Rect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height)
void cgfx_FillRect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 width, INT32 height)

void cgfx_ConvBlitInternal(CoreGfxBase *CoreGfxBase, CRastPort *rp, pCGfxBlitParms gc, BLITFUNC convblit);
void cgfx_StretchBlit(CoreGfxBase *CoreGfxBase, CRastPort *dstrp, INT32 dx1, INT32 dy1, INT32 dx2,
	INT32 dy2, CRastPort *srcrp, INT32 sx1, INT32 sy1, INT32 sx2,INT32 sy2, int rop);
void cgfx_Blit(CoreGfxBase *CoreGfxBase, CRastPort *dstrp, INT32 dstx, INT32 dsty, INT32 width, INT32 height,
	CRastPort *srcrp, INT32 srcx, INT32 srcy, int rop);
void cgfx_ConversionBlit(CoreGfxBase *CoreGfxBase, PixMap *psd, pCGfxBlitParms parms);
BLITFUNC cgfx_FindConvBlit(CoreGfxBase *CoreGfxBase, PixMap *psd, int data_format, int op);
void cgfx_BitmapByPoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height, const UINT16 *imagebits, int clipresult);

int cgfx_ConvertEncoding(CoreGfxBase *CoreGfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags);
void cgfx_GetTextSize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags);
void cgfx_Text(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, pCGfxFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags);
BOOL cgfx_GetFontInfo(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, pCGfxFontInfo pfontinfo);
void cgfx_DestroyFont(CoreGfxBase *CoreGfxBase, pCGfxFont pfont);
int cgfx_SetFontAttr(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int setflags, int clrflags);
int cgfx_SetFontRotation(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int tenthdegrees);
INT16 cgfx_SetFontSize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, INT16 height, INT16 width);
CGfxFont *cgfx_CreateFont(CoreGfxBase *CoreGfxBase, CRastPort *rp, const char *name, UINT16 height, UINT16 width, const pCGfxLogFont plogfont);

void cgfx_Poly(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, int count, CGfxPoint *points);
void cgfx_FillPoly(CoreGfxBase *CoreGfxBase, CRastPort *rp, int count, CGfxPoint *pointtable);

void cgfx_ArcAngle(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type);
void cgfx_Arc(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 ax, INT32 ay, INT32 bx, INT32 by, int type);
void cgfx_Ellipse(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill);

#endif

#define ClipArea(a,b,c,d,e)	(((INT32(*)(APTR, CRastPort *, INT32, INT32, INT32, INT32))	_GETVECADDR(CoreGfxBase, 5))(CoreGfxBase, a, b, c, d, e))
#define ClipPoint(a,b,c)	(((BOOL (*)(APTR, CRastPort *, INT32, INT32))					_GETVECADDR(CoreGfxBase, 6))(CoreGfxBase,a,b,c))
#define SetClipRegion(a,b)	(((void (*)(APTR, CRastPort *, ClipRegion *))					_GETVECADDR(CoreGfxBase, 7))(CoreGfxBase,a,b))

#define SetMode(a,b)				(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 8))(CoreGfxBase,a,b))
#define SetFillMode(a,b)			(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 9))(CoreGfxBase,a,b))
#define SetUseBackground(a,b)		(((BOOL  (*)(APTR, CRastPort *, BOOL))					_GETVECADDR(CoreGfxBase, 10))(CoreGfxBase,a,b))
#define SetForegroundPixelVal(a,b)	(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 11))(CoreGfxBase,a,b))
#define SetBackgroundPixelVal(a,b)	(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 12))(CoreGfxBase,a,b))
#define SetForegroundColor(a,b)		(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 13))(CoreGfxBase,a,b))
#define SetBackgroundColor(a,b)		(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase, 14))(CoreGfxBase,a,b))
#define SetDash(a,b,c)				(((void  (*)(APTR, CRastPort *, UINT32*, UINT32*))		_GETVECADDR(CoreGfxBase, 15))(CoreGfxBase,a,b,c))

#define FixCursor(a)				(((void (*)(pCoreGfxBase, PixMap *))							_GETVECADDR(CoreGfxBase,16))(CoreGfxBase,a))
#define CheckCursor(a,b,c,d,e)		(((void (*)(pCoreGfxBase, PixMap *,INT32,INT32,INT32,INT32))	_GETVECADDR(CoreGfxBase,17))(CoreGfxBase,a,b,c,d,e))
#define ShowCursor()				(((INT32(*)(pCoreGfxBase))										_GETVECADDR(CoreGfxBase,18))(CoreGfxBase))
#define HideCursor()				(((INT32(*)(pCoreGfxBase))										_GETVECADDR(CoreGfxBase,19))(CoreGfxBase))
#define SetCursor(a)				(((void (*)(pCoreGfxBase, struct Cursor*))						_GETVECADDR(CoreGfxBase,20))(CoreGfxBase,a))
#define GetCursorPos(a,b)			(((BOOL (*)(pCoreGfxBase, INT32*,INT32*))						_GETVECADDR(CoreGfxBase,21))(CoreGfxBase,a,b))
#define MoveCursor(a,b)				(((void (*)(pCoreGfxBase, INT32,INT32))							_GETVECADDR(CoreGfxBase,22))(CoreGfxBase,a,b))

#define FindNearestColor(a,b,c)		(((UINT32(*)(pCoreGfxBase, CRastPort *, int, UINT32))			_GETVECADDR(CoreGfxBase,23))(CoreGfxBase,a,b,c))
#define FindColor(a,b)				(((UINT32(*)(pCoreGfxBase, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase,24))(CoreGfxBase,a,b))

#define Point(a,b,c)				(((void(*)(pCoreGfxBase, CRastPort *, INT32,INT32))						_GETVECADDR(CoreGfxBase,25))(CoreGfxBase,a,b,c))
#define Line(a,b,c,d,e,f)			(((void(*)(pCoreGfxBase, CRastPort *, INT32,INT32,INT32,INT32,BOOL))	_GETVECADDR(CoreGfxBase,26))(CoreGfxBase,a,b,c,d,e,f))
#define Rect(a,b,c,d,e)				(((void(*)(pCoreGfxBase, CRastPort *, INT32,INT32,INT32,INT32))			_GETVECADDR(CoreGfxBase,27))(CoreGfxBase,a,b,c,d,e))
#define FillRect(a,b,c,d,e)			(((void(*)(pCoreGfxBase, CRastPort *, INT32,INT32,INT32,INT32))			_GETVECADDR(CoreGfxBase,28))(CoreGfxBase,a,b,c,d,e))

#define ConvBlitInternal(a,b,c)				(((void(*)(pCoreGfxBase, CRastPort *, pCGfxBlitParms, BLITFUNC))	_GETVECADDR(CoreGfxBase,29))(CoreGfxBase,a,b,c))
#define StretchBlit(a,b,c,d,e,f,g,h,i,j,k)	(((void(*)(pCoreGfxBase, CRastPort *, INT32 dx1, INT32 dy1, INT32 dx2,	INT32 dy2, CRastPort *srcrp, INT32 sx1, INT32 sy1, INT32 sx2,INT32 sy2, int rop))	_GETVECADDR(CoreGfxBase,30))(CoreGfxBase,a,b,c,d,e,f,g,h,i,j,k))
#define Blit(a,b,c,d,e,f,g,h,i)				(((void(*)(pCoreGfxBase, CRastPort *, INT32 dstx, INT32 dsty, INT32 width, INT32 height, CRastPort *srcrp, INT32 srcx, INT32 srcy, int rop))				_GETVECADDR(CoreGfxBase,31))(CoreGfxBase,a,b,c,d,e,f,g,h,i))
#define ConversionBlit(a,b)					(((void(*)(pCoreGfxBase, CRastPort *, pCGfxBlitParms parms))		_GETVECADDR(CoreGfxBase,32))(CoreGfxBase,a,b))
#define FindConvBlit(a,b,c)				(((BLITFUNC(*)(pCoreGfxBase, CRastPort *, int data_format, int op))		_GETVECADDR(CoreGfxBase,33))(CoreGfxBase,a,b,c))

#define BitmapByPoint(a,b,c,d,e,f,g)	(((void(*)(pCoreGfxBase, CRastPort *, INT32,INT32,INT32,INT32, const UINT16*, int))			_GETVECADDR(CoreGfxBase,34))(CoreGfxBase,a,b,c,d,e,f,g))

#define ConvertEncoding(a,b,c,d,e)		(((int (*)(pCoreGfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags))								_GETVECADDR(CoreGfxBase,35))(CoreGfxBase,a,b,c,d,e))
#define GetTextSize(a,b,c,d,e,f,g) 		(((void(*)(pCoreGfxBase, pCGfxFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags))	_GETVECADDR(CoreGfxBase,36))(CoreGfxBase,a,b,c,d,e,f,g))
#define Text(a,b,c,d,e,f,g) 			(((void(*)(pCoreGfxBase, struct CRastPort *rp, pCGfxFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags))	_GETVECADDR(CoreGfxBase,37))(CoreGfxBase,a,b,c,d,e,f,g))
#define GetFontInfo(a,b) 				(((BOOL(*)(pCoreGfxBase, pCGfxFont pfont, pCGfxFontInfo pfontinfo))															_GETVECADDR(CoreGfxBase,38))(CoreGfxBase,a,b))
#define DestroyFont(a) 					(((void(*)(pCoreGfxBase, pCGfxFont pfont))																					_GETVECADDR(CoreGfxBase,39))(CoreGfxBase,a))
#define SetFontAttr(a,b,c) 				(((int (*)(pCoreGfxBase, pCGfxFont pfont, int setflags, int clrflags))														_GETVECADDR(CoreGfxBase,40))(CoreGfxBase,a,b,c))
#define SetFontRotation(a,b)			(((int (*)(pCoreGfxBase, pCGfxFont pfont, int tenthdegrees))																_GETVECADDR(CoreGfxBase,41))(CoreGfxBase,a,b))
#define SetFontSize(a,b,c)				(((int (*)(pCoreGfxBase, pCGfxFont pfont, INT32 height, INT32 width))														_GETVECADDR(CoreGfxBase,42))(CoreGfxBase,a,b,c))
#define CreateFont(a,b,c,d,e)			(((CGfxFont *(*)(pCoreGfxBase, CRastPort *rp, const char *name, INT32 height, INT32 width, const pCGfxLogFont plogfont))	_GETVECADDR(CoreGfxBase,43))(CoreGfxBase,a,b,c,d,e))

#define GetScreenInfo(a,b)				(((void(*)(pCoreGfxBase, CRastPort *rp, pCGfxScreenInfo psi)) _GETVECADDR(CoreGfxBase,44))(CoreGfxBase,a,b))


#define Poly(a,b,c) 					(((void (*)(pCoreGfxBase, struct CRastPort *rp, int count, CGfxPoint *points)) 	_GETVECADDR(CoreGfxBase,45))(CoreGfxBase,a,b,c))
#define FillPoly(a,b,c) 				(((void (*)(pCoreGfxBase, CRastPort *rp, int count, CGfxPoint *pointtable))		_GETVECADDR(CoreGfxBase,46))(CoreGfxBase,a,b,c))

#define ArcAngle(a,b,c,d,e,f,g,h)	 	(((void (*)(pCoreGfxBase, CRastPort *rp, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type))					_GETVECADDR(CoreGfxBase,47))(CoreGfxBase,a,b,c,d,e,f,g,h))
#define Arc(a,b,c,d,e,f,g,h,i,j)		(((void (*)(pCoreGfxBase, CRastPort *psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 ax, INT32 ay, INT32 bx, INT32 by, int type))	_GETVECADDR(CoreGfxBase,48))(CoreGfxBase,a,b,c,d,e,f,g,h,i,j))
#define Ellipse(a,b,c,d,e,f)		 	(((void (*)(pCoreGfxBase, CRastPort *psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill))												_GETVECADDR(CoreGfxBase,49))(CoreGfxBase,a,b,c,d,e,f))


#endif
