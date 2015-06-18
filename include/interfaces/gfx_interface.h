/**
* File: /gfx_interface．h
* User: cycl0ne
* Date: 2014-11-27
* Time: 09:23 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef GFX_INTERFACE_H
#define GFX_INTERFACE_H

typedef struct GfxBase * pGfxBase;

#if 1
#define ClipArea(a,b,c,d,e)					_LIBCALL(GfxBase,  5, INT32, (pGfxBase, pPB , INT32, INT32, INT32, INT32), (GfxBase,a,b,c,d,e))
#define ClipPoint(a,b,c)					_LIBCALL(GfxBase,  6, BOOL , (pGfxBase, pPB , INT32, INT32), (GfxBase,a,b,c))
#define SetClipRegion(a,b)					_LIBCALL(GfxBase,  7, void , (pGfxBase, pPB , ClipRegion *), (GfxBase,a,b))
#define SetMode(a,b)						_LIBCALL(GfxBase,  8, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetFillMode(a,b)					_LIBCALL(GfxBase,  9, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetUseBackground(a,b)				_LIBCALL(GfxBase, 10, BOOL  , (pGfxBase, pPB , BOOL), (GfxBase,a,b))
#define SetForegroundPixelVal(a,b)			_LIBCALL(GfxBase, 11, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetBackgroundPixelVal(a,b)			_LIBCALL(GfxBase, 12, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetForegroundColor(a,b)				_LIBCALL(GfxBase, 13, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetBackgroundColor(a,b)				_LIBCALL(GfxBase, 14, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define SetDash(a,b,c)						_LIBCALL(GfxBase, 15, void  , (pGfxBase, pPB , UINT32*, UINT32*), (GfxBase,a,b,c))
#define FixCursor(a)						_LIBCALL(GfxBase, 16, void , (pGfxBase, pSD), (GfxBase,a))
#define CheckCursor(a,b,c,d,e)				_LIBCALL(GfxBase, 17, void , (pGfxBase, pSD,INT32,INT32,INT32,INT32), (GfxBase,a,b,c,d,e))
#define ShowCursor(a)						_LIBCALL(GfxBase, 18, INT32, (pGfxBase, pSD), (GfxBase,a))
#define HideCursor(a)						_LIBCALL(GfxBase, 19, INT32, (pGfxBase, pSD), (GfxBase,a))
#define SetCursor(a,b)						_LIBCALL(GfxBase, 20, void , (pGfxBase, pPB,struct Cursor*), (GfxBase,a,b))
#define GetCursorPos(a,b)					_LIBCALL(GfxBase, 21, BOOL , (pGfxBase, INT32*,INT32*), (GfxBase,a,b))
#define MoveCursor(a,b,c)					_LIBCALL(GfxBase, 22, void , (pGfxBase, pPB,INT32,INT32), (GfxBase,a,b,c))
#define FindNearestColor(a,b,c)				_LIBCALL(GfxBase, 23, UINT32, (pGfxBase, pPB , int, UINT32), (GfxBase,a,b,c))
#define FindColor(a,b)						_LIBCALL(GfxBase, 24, UINT32, (pGfxBase, pPB , UINT32), (GfxBase,a,b))
#define Point(a,b,c)						_LIBCALL(GfxBase, 25, void, (pGfxBase, pPB , INT32,INT32), (GfxBase,a,b,c))
#define Line(a,b,c,d,e,f)					_LIBCALL(GfxBase, 26, void, (pGfxBase, pPB , INT32,INT32,INT32,INT32,BOOL), (GfxBase,a,b,c,d,e,f))
#define Rect(a,b,c,d,e)						_LIBCALL(GfxBase, 27, void, (pGfxBase, pPB , INT32,INT32,INT32,INT32), (GfxBase,a,b,c,d,e))
#define RectFill(a,b,c,d,e)					_LIBCALL(GfxBase, 28, void, (pGfxBase, pPB , INT32,INT32,INT32,INT32), (GfxBase,a,b,c,d,e))
#define ConvBlitInternal(a,b,c)				_LIBCALL(GfxBase, 29, void, (pGfxBase, pPB , pBlitParms, BLITFUNC), (GfxBase,a,b,c))
#define StretchBlit(a,b,c,d,e,f,g,h,i,j,k)	_LIBCALL(GfxBase, 30, void, (pGfxBase, pPB , INT32 dx1, INT32 dy1, INT32 dx2,	INT32 dy2, pPB srcrp, INT32 sx1, INT32 sy1, INT32 sx2,INT32 sy2, int rop), (GfxBase,a,b,c,d,e,f,g,h,i,j,k))
#define Blit(a,b,c,d,e,f,g,h,i)				_LIBCALL(GfxBase, 31, void, (pGfxBase, pPB , INT32 dstx, INT32 dsty, INT32 width, INT32 height, pPB srcrp, INT32 srcx, INT32 srcy, int rop), (GfxBase,a,b,c,d,e,f,g,h,i))
#define ConversionBlit(a,b)					_LIBCALL(GfxBase, 32, void, (pGfxBase, pPB , pBlitParms parms), (GfxBase,a,b))
#define FindConvBlit(a,b,c)					_LIBCALL(GfxBase, 33, BLITFUNC, (pGfxBase, pPB , int data_format, int op), (GfxBase,a,b,c))
#define BitmapByPoint(a,b,c,d,e,f,g)		_LIBCALL(GfxBase, 34, void, (pGfxBase, pPB , INT32,INT32,INT32,INT32, const UINT16*, int), (GfxBase,a,b,c,d,e,f,g))
#define ConvertEncoding(a,b,c,d,e)			_LIBCALL(GfxBase, 35, int, (pGfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags), (GfxBase,a,b,c,d,e))
#define GetTextSize(a,b,c,d,e,f,g)			_LIBCALL(GfxBase, 36, void, (pGfxBase, pFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags), (GfxBase,a,b,c,d,e,f,g))
#define Text(a,b,c,d,e,f,g)					_LIBCALL(GfxBase, 37, void, (pGfxBase, pPB rp, pFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags), (GfxBase,a,b,c,d,e,f,g))
#define GetFontInfo(a,b)					_LIBCALL(GfxBase, 38, BOOL, (pGfxBase, pFont pfont, pFontInfo pfontinfo), (GfxBase,a,b))
#define DestroyFont(a)						_LIBCALL(GfxBase, 39, void, (pGfxBase, pFont pfont), (GfxBase,a))
#define SetFontAttr(a,b,c)					_LIBCALL(GfxBase, 40, int, (pGfxBase, pFont pfont, int setflags, int clrflags), (GfxBase,a,b,c))
#define SetFontRotation(a,b)				_LIBCALL(GfxBase, 41, int, (pGfxBase, pFont pfont, int tenthdegrees), (GfxBase,a,b))
#define SetFontSize(a,b,c)					_LIBCALL(GfxBase, 42, int, (pGfxBase, pFont pfont, INT32 height, INT32 width), (GfxBase,a,b,c))
#define CreateFont(a,b,c,d,e)				_LIBCALL(GfxBase, 43, pFont, (pGfxBase, pPB rp, const char *name, INT32 height, INT32 width, const pLogFont plogfont), (GfxBase,a,b,c,d,e))
#define GetScreenInfo(a,b)					_LIBCALL(GfxBase, 44, void, (pGfxBase, pPB rp, pScreenInfo psi), (GfxBase,a,b))
#define Poly(a,b,c)							_LIBCALL(GfxBase, 45, void, (pGfxBase, pPB rp, int count, Point_t *points), (GfxBase,a,b,c))
#define FillPoly(a,b,c)						_LIBCALL(GfxBase, 46, void, (pGfxBase, pPB rp, int count, Point_t *pointtable), (GfxBase,a,b,c))
#define ArcAngle(a,b,c,d,e,f,g,h)			_LIBCALL(GfxBase, 47, void, (pGfxBase, pPB rp, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type), (GfxBase,a,b,c,d,e,f,g,h))
#define Arc(a,b,c,d,e,f,g,h,i,j)			_LIBCALL(GfxBase, 48, void, (pGfxBase, pPB psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 ax, INT32 ay, INT32 bx, INT32 by, int type), (GfxBase,a,b,c,d,e,f,g,h,i,j))
#define Ellipse(a,b,c,d,e,f)				_LIBCALL(GfxBase, 49, void, (pGfxBase, pPB psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill), (GfxBase,a,b,c,d,e,f))


#else

#define ClipArea(a,b,c,d,e)	(((INT32(*)(pGfxBase, pPB , INT32, INT32, INT32, INT32))	_GETVECADDR(GfxBase, 5))(GfxBase, a, b, c, d, e))
#define ClipPoint(a,b,c)	(((BOOL (*)(pGfxBase, pPB , INT32, INT32))					_GETVECADDR(GfxBase, 6))(GfxBase,a,b,c))
#define SetClipRegion(a,b)	(((void (*)(pGfxBase, pPB , ClipRegion *))					_GETVECADDR(GfxBase, 7))(GfxBase,a,b))

#define SetMode(a,b)				(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 8))(GfxBase,a,b))
#define SetFillMode(a,b)			(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 9))(GfxBase,a,b))
#define SetUseBackground(a,b)		(((BOOL  (*)(pGfxBase, pPB , BOOL))					_GETVECADDR(GfxBase, 10))(GfxBase,a,b))
#define SetForegroundPixelVal(a,b)	(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 11))(GfxBase,a,b))
#define SetBackgroundPixelVal(a,b)	(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 12))(GfxBase,a,b))
#define SetForegroundColor(a,b)		(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 13))(GfxBase,a,b))
#define SetBackgroundColor(a,b)		(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase, 14))(GfxBase,a,b))
#define SetDash(a,b,c)				(((void  (*)(pGfxBase, pPB , UINT32*, UINT32*))		_GETVECADDR(GfxBase, 15))(GfxBase,a,b,c))

#define FixCursor(a)				(((void (*)(pGfxBase, PixMap_t *))							_GETVECADDR(GfxBase,16))(GfxBase,a))
#define CheckCursor(a,b,c,d,e)		(((void (*)(pGfxBase, PixMap_t *,INT32,INT32,INT32,INT32))	_GETVECADDR(GfxBase,17))(GfxBase,a,b,c,d,e))
#define ShowCursor()				(((INT32(*)(pGfxBase))										_GETVECADDR(GfxBase,18))(GfxBase))
#define HideCursor()				(((INT32(*)(pGfxBase))										_GETVECADDR(GfxBase,19))(GfxBase))
#define SetCursor(a)				(((void (*)(pGfxBase, struct Cursor*))						_GETVECADDR(GfxBase,20))(GfxBase,a))
#define GetCursorPos(a,b)			(((BOOL (*)(pGfxBase, INT32*,INT32*))						_GETVECADDR(GfxBase,21))(GfxBase,a,b))
#define MoveCursor(a,b)				(((void (*)(pGfxBase, INT32,INT32))							_GETVECADDR(GfxBase,22))(GfxBase,a,b))

#define FindNearestColor(a,b,c)		(((UINT32(*)(pGfxBase, pPB , int, UINT32))			_GETVECADDR(GfxBase,23))(GfxBase,a,b,c))
#define FindColor(a,b)				(((UINT32(*)(pGfxBase, pPB , UINT32))				_GETVECADDR(GfxBase,24))(GfxBase,a,b))

#define Point(a,b,c)				(((void(*)(pGfxBase, pPB , INT32,INT32))						_GETVECADDR(GfxBase,25))(GfxBase,a,b,c))
#define Line(a,b,c,d,e,f)			(((void(*)(pGfxBase, pPB , INT32,INT32,INT32,INT32,BOOL))	_GETVECADDR(GfxBase,26))(GfxBase,a,b,c,d,e,f))
#define Rect(a,b,c,d,e)				(((void(*)(pGfxBase, pPB , INT32,INT32,INT32,INT32))			_GETVECADDR(GfxBase,27))(GfxBase,a,b,c,d,e))
#define RectFill(a,b,c,d,e)			(((void(*)(pGfxBase, pPB , INT32,INT32,INT32,INT32))			_GETVECADDR(GfxBase,28))(GfxBase,a,b,c,d,e))

#define ConvBlitInternal(a,b,c)				(((void(*)(pGfxBase, pPB , pBlitParms, BLITFUNC))	_GETVECADDR(GfxBase,29))(GfxBase,a,b,c))
#define StretchBlit(a,b,c,d,e,f,g,h,i,j,k)	(((void(*)(pGfxBase, pPB , INT32 dx1, INT32 dy1, INT32 dx2,	INT32 dy2, pPB srcrp, INT32 sx1, INT32 sy1, INT32 sx2,INT32 sy2, int rop))	_GETVECADDR(GfxBase,30))(GfxBase,a,b,c,d,e,f,g,h,i,j,k))
#define Blit(a,b,c,d,e,f,g,h,i)				(((void(*)(pGfxBase, pPB , INT32 dstx, INT32 dsty, INT32 width, INT32 height, pPB srcrp, INT32 srcx, INT32 srcy, int rop))				_GETVECADDR(GfxBase,31))(GfxBase,a,b,c,d,e,f,g,h,i))
#define ConversionBlit(a,b)					(((void(*)(pGfxBase, pPB , pBlitParms parms))		_GETVECADDR(GfxBase,32))(GfxBase,a,b))
#define FindConvBlit(a,b,c)				(((BLITFUNC(*)(pGfxBase, pPB , int data_format, int op))		_GETVECADDR(GfxBase,33))(GfxBase,a,b,c))

#define BitmapByPoint(a,b,c,d,e,f,g)	(((void(*)(pGfxBase, pPB , INT32,INT32,INT32,INT32, const UINT16*, int))			_GETVECADDR(GfxBase,34))(GfxBase,a,b,c,d,e,f,g))

#define ConvertEncoding(a,b,c,d,e)		(((int (*)(pGfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags))								_GETVECADDR(GfxBase,35))(GfxBase,a,b,c,d,e))
#define GetTextSize(a,b,c,d,e,f,g) 		(((void(*)(pGfxBase, pFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags))	_GETVECADDR(GfxBase,36))(GfxBase,a,b,c,d,e,f,g))
#define Text(a,b,c,d,e,f,g) 			(((void(*)(pGfxBase, pPB rp, pFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags))	_GETVECADDR(GfxBase,37))(GfxBase,a,b,c,d,e,f,g))
#define GetFontInfo(a,b) 				(((BOOL(*)(pGfxBase, pFont pfont, pFontInfo pfontinfo))															_GETVECADDR(GfxBase,38))(GfxBase,a,b))
#define DestroyFont(a) 					(((void(*)(pGfxBase, pFont pfont))																					_GETVECADDR(GfxBase,39))(GfxBase,a))
#define SetFontAttr(a,b,c) 				(((int (*)(pGfxBase, pFont pfont, int setflags, int clrflags))														_GETVECADDR(GfxBase,40))(GfxBase,a,b,c))
#define SetFontRotation(a,b)			(((int (*)(pGfxBase, pFont pfont, int tenthdegrees))																_GETVECADDR(GfxBase,41))(GfxBase,a,b))
#define SetFontSize(a,b,c)				(((int (*)(pGfxBase, pFont pfont, INT32 height, INT32 width))														_GETVECADDR(GfxBase,42))(GfxBase,a,b,c))
#define CreateFont(a,b,c,d,e)			(((pFont (*)(pGfxBase, pPB rp, const char *name, INT32 height, INT32 width, const pLogFont plogfont))	_GETVECADDR(GfxBase,43))(GfxBase,a,b,c,d,e))

#define GetScreenInfo(a,b)				(((void(*)(pGfxBase, pPB rp, pScreenInfo psi)) _GETVECADDR(GfxBase,44))(GfxBase,a,b))


#define Poly(a,b,c) 					(((void (*)(pGfxBase, pPB rp, int count, Point_t *points)) 	_GETVECADDR(GfxBase,45))(GfxBase,a,b,c))
#define FillPoly(a,b,c) 				(((void (*)(pGfxBase, pPB rp, int count, Point_t *pointtable))		_GETVECADDR(GfxBase,46))(GfxBase,a,b,c))

#define ArcAngle(a,b,c,d,e,f,g,h)	 	(((void (*)(pGfxBase, pPB rp, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type))					_GETVECADDR(GfxBase,47))(GfxBase,a,b,c,d,e,f,g,h))
#define Arc(a,b,c,d,e,f,g,h,i,j)		(((void (*)(pGfxBase, pPB psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 ax, INT32 ay, INT32 bx, INT32 by, int type))	_GETVECADDR(GfxBase,48))(GfxBase,a,b,c,d,e,f,g,h,i,j))
#define Ellipse(a,b,c,d,e,f)		 	(((void (*)(pGfxBase, pPB psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill))	
#endif

#endif
