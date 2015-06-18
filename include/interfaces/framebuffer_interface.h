/**
* File: /frambuffer_interface．h
* User: cycl0ne
* Date: 2014-11-29
* Time: 04:02 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef framebuffer_interface_H
#define framebuffer_interface_H

typedef struct FBBase *pFBBase;

#if 1
#define OpenFB(a,b,c)				_LIBCALL(FBBase,  5, pSD , (pFBBase, int32_t w, int32_t h, int32_t bpp), (FBBase, a, b, c))
#define CloseFB(a)					_LIBCALL(FBBase,  6, void, (pFBBase, pSD)	, (FBBase,a))

#define AllocateMemGC(a)			_LIBCALL(FBBase,  7, pSD , (pFBBase FBBase, pSD psd), (FBBase,a))
#define FreeMemGC(a)				_LIBCALL(FBBase,  8, void, (pFBBase FBBase, pSD psd), (FBBase,a))
#define MapMemGC(a,b,c,d,e,f,g,h,i)	_LIBCALL(FBBase,  9, pSD , (pFBBase FBBase, pSD mempsd, int32_t w, int32_t h, int32_t planes, int32_t bpp, int32_t data_format, uint32_t pitch, int32_t size, void *addr), (FBBase,a,b,c,d,e,f,g,h,i))

#else

#define OpenFB()	(((pSD	(*)(APTR))			_GETVECADDR(FBBase, 5))(FBBase))
#define CloseFB(a)	(((void	(*)(APTR, pSD))		_GETVECADDR(FBBase, 6))(FBBase,a))
#endif

#endif
