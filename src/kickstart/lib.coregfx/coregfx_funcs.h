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

#define FixCursor(a)				(((void (*)(APTR, CRastPort *))							_GETVECADDR(CoreGfxBase,16))(CoreGfxBase,a))
#define CheckCursor(a,b,c,d,e)		(((void (*)(APTR, CRastPort*,INT32,INT32,INT32,INT32))	_GETVECADDR(CoreGfxBase,17))(CoreGfxBase,a,b,c,d,e))
#define ShowCursor()				(((INT32(*)(APTR))										_GETVECADDR(CoreGfxBase,18))(CoreGfxBase))
#define HideCursor()				(((INT32(*)(APTR))										_GETVECADDR(CoreGfxBase,19))(CoreGfxBase))
#define SetCursor(a)				(((void (*)(APTR, struct Cursor*))						_GETVECADDR(CoreGfxBase,20))(CoreGfxBase,a))
#define GetCursorPos(a,b)			(((BOOL (*)(APTR, INT32*,INT32*))						_GETVECADDR(CoreGfxBase,21))(CoreGfxBase,a,b))
#define MoveCursor(a,b)				(((void (*)(APTR, INT32,INT32))							_GETVECADDR(CoreGfxBase,22))(CoreGfxBase,a,b))

#define FindNearestColor(a,b,c)		(((UINT32(*)(APTR, CRastPort *, int, UINT32))			_GETVECADDR(CoreGfxBase,23))(CoreGfxBase,a,b,c))
#define FindColor(a,b)				(((UINT32(*)(APTR, CRastPort *, UINT32))				_GETVECADDR(CoreGfxBase,23))(CoreGfxBase,a,b))

#endif
