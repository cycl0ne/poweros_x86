/**
* File: /pixmap．c
* User: cycl0ne
* Date: 2014-11-24
* Time: 09:55 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfx.h"

typedef struct PixMap {
	
	
}PixMap_t, *pPixMap;

// INTERNAL
pPixMap gfx_AllocPixMap(pGfxBase GfxBase, pPixMap psd)
{
	pPixMap mempsd;
	
}

// EXTERNAL
pPixMap gfx_CreatePixMap(pGfxBase GfxBase, INT32 width, INT32 height, INT32 format, void *pixels, INT32 palsize)
{
	if (width <= 0 || height <= 0) return NULL;

	
	
	
}

void gfx_FreePixMap(pGfxBase GfxBase, pPixMap pmd)
{
	if (pmd != NULL)
	{
		if (!pmd->flags & PMF_MEMORY) return;
		if (pmd->addr && (pmd->flags & PMF_ADDRALLOC)) FreeVec(pmd->addr);
		if (pmd->palette) FreeVec(pmd->palette);
		FreeVec(pmd);
	}
}

INT32 gfx_CalcMemAlloc(pGfxBase GfxBase, pPixMap psd, INT32 width, INT32 height, INT32 planes, INT32 bpp, UINT32 *psize, UIN32 *ppitch)
{
	unsigned int pitch;

	if (!planes) planes = psd->planes;
	if (!bpp) bpp = psd->bpp;
	/* 
	 * swap width and height in left/right portrait modes,
	 * so imagesize is calculated properly
	 */
	if(psd->portrait & (MWPORTRAIT_LEFT|MWPORTRAIT_RIGHT)) 
	{
		int tmp = width;
		width = height;
		height = tmp;
	}

	/* use 4bpp linear for VGA 4 planes memdc format*/
	if (planes == 4) bpp = 4;

	/* compute pitch: bytes per line*/
	switch(bpp) 
	{
	case 1:
		pitch = (width+7)/8;
		break;
	case 2:
		pitch = (width+3)/4;
		break;
	case 4:
		pitch = (width+1)/2;
		break;
	case 8:
		pitch = width;
		break;
	case 16:
		pitch = width * 2;
		break;
	case 18:
	case 24:
		pitch = width * 3;
		break;
	case 32:
		pitch = width * 4;
		break;
	default:
		*ppitch = *psize = 0;
		return 0;
	}

	/* right align pitch to DWORD boundary*/
	pitch = (pitch + 3) & ~3;

	*psize = pitch * height;
	*ppitch = pitch;
	return 1;
}
