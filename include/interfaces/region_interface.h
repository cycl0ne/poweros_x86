/**
* File: /region_interface．h
* User: cycl0ne
* Date: 2014-11-23
* Time: 10:24 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef region_interface_H
#define region_interface_H

typedef struct RegionBase * pRegionBase;

#if 1
#define AllocRegion()				_LIBCALL(RegionBase,  5, pClipRegion , (APTR), (RegionBase))
#define AllocRectRegion(a,b,c,d)	_LIBCALL(RegionBase,  6, pClipRegion , (APTR, INT32, INT32, INT32, INT32), (RegionBase,a,b,c,d))
#define AllocRectRegionIndirect(a)	_LIBCALL(RegionBase,  7, pClipRegion , (APTR, 								_GETVECADDR(RegionBase, 7), (RegionBase,a))
#define CopyRegion(a,b)				_LIBCALL(RegionBase,  8, void, (APTR, pClipRegion , pClipRegion ), (RegionBase,a,b))
#define DestroyRegion(a)			_LIBCALL(RegionBase,  9, void, (APTR, pClipRegion ), (RegionBase,a))
#define EmptyRegion(a)				_LIBCALL(RegionBase, 10, BOOL, (APTR, pClipRegion ), (RegionBase,a))
#define EqualRegion(a,b)			_LIBCALL(RegionBase, 11, BOOL, (APTR, pClipRegion , pClipRegion ), (RegionBase,a,b))
#define GetRegionBox(a,b)			_LIBCALL(RegionBase, 12, INT32, (APTR, pClipRegion , pRect ), (RegionBase,a,b))
#define IntersectRegion(a,b,c)		_LIBCALL(RegionBase, 13, void, (APTR,pClipRegion ,pClipRegion ,pClipRegion ), (RegionBase,a,b,c))
#define OffsetRegion(a,b,c)			_LIBCALL(RegionBase, 14, void, (APTR, pClipRegion , INT32, INT32), (RegionBase,a,b,c))
#define PtInRegion(a,b,c)			_LIBCALL(RegionBase, 15, BOOL, (APTR, pClipRegion , INT32, INT32), (RegionBase,a,b,c))
#define RectInRegion(a,b)			_LIBCALL(RegionBase, 16, INT32, (APTR, pClipRegion , const pRect ), (RegionBase,a,b))
#define SetRectRegion(a,b,c,d,e)	_LIBCALL(RegionBase, 17, void, (APTR, pClipRegion , INT32, INT32, INT32, INT32), (RegionBase,a,b,c,d,e))
#define SetRectRegionIndirect(a,b)	_LIBCALL(RegionBase, 18, void, (APTR, pClipRegion , pRect ), (RegionBase,a,b))
#define SubtractRegion(a,b,c)		_LIBCALL(RegionBase, 19, void, (APTR, pClipRegion , pClipRegion , pClipRegion ), (RegionBase,a,b,c))
#define SubtractRectFromRegion(a,b)	_LIBCALL(RegionBase, 20, void, (APTR, const pRect , pClipRegion ), (RegionBase,a,b))
#define UnionRegion(a,b,c)			_LIBCALL(RegionBase, 21, void, (APTR, pClipRegion , pClipRegion , pClipRegion ), (RegionBase,a,b,c))
#define UnionRectWithRegion(a,b)	_LIBCALL(RegionBase, 22, void, (APTR, const pRect , pClipRegion ), (RegionBase,a,b))
#define XorRegion(a,b,c)			_LIBCALL(RegionBase, 23, void, (APTR, pClipRegion , pClipRegion , pClipRegion ), (RegionBase,a,b,c))

#else

#define AllocRegion()				(((pClipRegion (*)(APTR))										_GETVECADDR(RegionBase, 5))(RegionBase))
#define AllocRectRegion(a,b,c,d)	(((pClipRegion (*)(APTR, INT32, INT32, INT32, INT32))			_GETVECADDR(RegionBase, 6))(RegionBase,a,b,c,d))
#define AllocRectRegionIndirect(a)	(((pClipRegion (*)(APTR, 								_GETVECADDR(RegionBase, 7))(RegionBase,a))
#define CopyRegion(a,b)				(((void(*)(APTR, pClipRegion , pClipRegion ))					_GETVECADDR(RegionBase, 8))(RegionBase,a,b))
#define DestroyRegion(a)			(((void(*)(APTR, pClipRegion ))								_GETVECADDR(RegionBase, 9))(RegionBase,a))
#define EmptyRegion(a)				(((BOOL(*)(APTR, pClipRegion ))								_GETVECADDR(RegionBase,10))(RegionBase,a))
#define EqualRegion(a,b)			(((BOOL(*)(APTR, pClipRegion , pClipRegion ))					_GETVECADDR(RegionBase,11))(RegionBase,a,b))
#define GetRegionBox(a,b)			(((INT32(*)(APTR, pClipRegion , pRect ))						_GETVECADDR(RegionBase,12))(RegionBase,a,b))
#define IntersectRegion(a,b,c)		(((void(*)(APTR,pClipRegion ,pClipRegion ,pClipRegion ))			_GETVECADDR(RegionBase,13))(RegionBase,a,b,c))
#define OffsetRegion(a,b,c)			(((void(*)(APTR, pClipRegion , INT32, INT32))					_GETVECADDR(RegionBase,14))(RegionBase,a,b,c))
#define PtInRegion(a,b,c)			(((BOOL(*)(APTR, pClipRegion , INT32, INT32))					_GETVECADDR(RegionBase,15))(RegionBase,a,b,c))
#define RectInRegion(a,b)			(((INT32(*)(APTR, pClipRegion , const pRect ))					_GETVECADDR(RegionBase,16))(RegionBase,a,b))
#define SetRectRegion(a,b,c,d,e)	(((void(*)(APTR, pClipRegion , INT32, INT32, INT32, INT32))	_GETVECADDR(RegionBase,17))(RegionBase,a,b,c,d,e))
#define SetRectRegionIndirect(a,b)	(((void(*)(APTR, pClipRegion , pRect ))						_GETVECADDR(RegionBase,18))(RegionBase,a,b))
#define SubtractRegion(a,b,c)		(((void(*)(APTR, pClipRegion , pClipRegion , pClipRegion ))		_GETVECADDR(RegionBase,19))(RegionBase,a,b,c))
#define SubtractRectFromRegion(a,b)	(((void(*)(APTR, const pRect , pClipRegion ))					_GETVECADDR(RegionBase,20))(RegionBase,a,b))
#define UnionRegion(a,b,c)			(((void(*)(APTR, pClipRegion , pClipRegion , pClipRegion ))		_GETVECADDR(RegionBase,21))(RegionBase,a,b,c))
#define UnionRectWithRegion(a,b)	(((void(*)(APTR, const pRect , pClipRegion ))					_GETVECADDR(RegionBase,22))(RegionBase,a,b))
#define XorRegion(a,b,c)			(((void(*)(APTR, pClipRegion , pClipRegion , pClipRegion ))		_GETVECADDR(RegionBase,23))(RegionBase,a,b,c))

#endif



#endif
