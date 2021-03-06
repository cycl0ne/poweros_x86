#ifndef REGION_FUNCS_H
#define REGION_FUNCS_H

#include "types.h"
#if 0
ClipRegion *AllocRegion();
ClipRegion *AllocRectRegion(INT32 left, INT32 top, INT32 right, INT32 bottom);
ClipRegion *AllocRectRegionIndirect(Rect *prc);
void CopyRegion(ClipRegion *dst, ClipRegion *src);
void DestroyRegion(ClipRegion *rgn);
BOOL EmptyRegion(ClipRegion *rgn);
BOOL EqualRegion(ClipRegion *r1, ClipRegion *r2);
INT32 GetRegionBox(ClipRegion *rgn, Rect *prc);
void IntersectRegion(ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2);
void OffsetRegion(ClipRegion *rgn, INT32 x, INT32 y);
BOOL PtInRegion(ClipRegion *rgn, INT32 x, INT32 y);
INT32 RectInRegion(ClipRegion *rgn, const Rect *rect);

void SetRectRegion(ClipRegion *rgn, INT32 left, INT32 top, INT32 right, INT32 bottom);
void SetRectRegionIndirect(ClipRegion *rgn, Rect *prc);
void SubtractRegion(ClipRegion *regD, ClipRegion *regM, ClipRegion *regS );
void SubtractRectFromRegion(const Rect *rect, ClipRegion *rgn);
void UnionRegion(ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2);
void UnionRectWithRegion(const Rect *rect, ClipRegion *rgn);
void XorRegion(ClipRegion *dr, ClipRegion *sra, ClipRegion *srb);
#endif

#define AllocRegion()				(((ClipRegion*(*)(APTR))										_GETVECADDR(RegionBase, 5))(RegionBase))
#define AllocRectRegion(a,b,c,d)	(((ClipRegion*(*)(APTR, INT32, INT32, INT32, INT32))			_GETVECADDR(RegionBase, 6))(RegionBase,a,b,c,d))
#define AllocRectRegionIndirect(a)	(((ClipRegion*(*)(APTR, Rect *))								_GETVECADDR(RegionBase, 7))(RegionBase,a))
#define CopyRegion(a,b)				(((void(*)(APTR, ClipRegion*, ClipRegion*))					_GETVECADDR(RegionBase, 8))(RegionBase,a,b))
#define DestroyRegion(a)			(((void(*)(APTR, ClipRegion*))								_GETVECADDR(RegionBase, 9))(RegionBase,a))
#define EmptyRegion(a)				(((BOOL(*)(APTR, ClipRegion*))								_GETVECADDR(RegionBase,10))(RegionBase,a))
#define EqualRegion(a,b)			(((BOOL(*)(APTR, ClipRegion*, ClipRegion*))					_GETVECADDR(RegionBase,11))(RegionBase,a,b))
#define GetRegionBox(a,b)			(((INT32(*)(APTR, ClipRegion*, Rect*))						_GETVECADDR(RegionBase,12))(RegionBase,a,b))
#define IntersectRegion(a,b,c)		(((void(*)(APTR,ClipRegion*,ClipRegion*,ClipRegion*))			_GETVECADDR(RegionBase,13))(RegionBase,a,b,c))
#define OffsetRegion(a,b,c)			(((void(*)(APTR, ClipRegion*, INT32, INT32))					_GETVECADDR(RegionBase,14))(RegionBase,a,b,c))
#define PtInRegion(a,b,c)			(((BOOL(*)(APTR, ClipRegion*, INT32, INT32))					_GETVECADDR(RegionBase,15))(RegionBase,a,b,c))
#define RectInRegion(a,b)			(((INT32(*)(APTR, ClipRegion*, const Rect *))					_GETVECADDR(RegionBase,16))(RegionBase,a,b))
#define SetRectRegion(a,b,c,d,e)	(((void(*)(APTR, ClipRegion*, INT32, INT32, INT32, INT32))	_GETVECADDR(RegionBase,17))(RegionBase,a,b,c,d,e))
#define SetRectRegionIndirect(a,b)	(((void(*)(APTR, ClipRegion*, Rect *))						_GETVECADDR(RegionBase,18))(RegionBase,a,b))
#define SubtractRegion(a,b,c)		(((void(*)(APTR, ClipRegion*, ClipRegion*, ClipRegion*))		_GETVECADDR(RegionBase,19))(RegionBase,a,b,c))
#define SubtractRectFromRegion(a,b)	(((void(*)(APTR, const Rect*, ClipRegion*))					_GETVECADDR(RegionBase,20))(RegionBase,a,b))
#define UnionRegion(a,b,c)			(((void(*)(APTR, ClipRegion*, ClipRegion*, ClipRegion*))		_GETVECADDR(RegionBase,21))(RegionBase,a,b,c))
#define UnionRectWithRegion(a,b)	(((void(*)(APTR, const Rect*, ClipRegion*))					_GETVECADDR(RegionBase,22))(RegionBase,a,b))
#define XorRegion(a,b,c)			(((void(*)(APTR, ClipRegion*, ClipRegion*, ClipRegion*))		_GETVECADDR(RegionBase,23))(RegionBase,a,b,c))

#endif
