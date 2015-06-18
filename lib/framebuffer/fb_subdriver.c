/**
* File: /fb_subdriver．c
* User: cycl0ne
* Date: 2014-11-26
* Time: 02:59 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "framebuffer.h"
#include "applyop.h"

static void linear32_drawpixel(pSD psd, INT32 x, INT32 y, PIXELVAL c)
{
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	INT32	gr_mode = psd->mode;
	PIXELVAL gr_background = psd->background;

//	DRAWON;
	if (gr_mode == ROP_COPY) *((ADDR32)addr) = c;
	else
		APPLYOP(gr_mode, 1, (UINT32), c, *(ADDR32), addr, 0, 0);
//	DRAWOFF;

	if (psd->Update) psd->Update(psd, x, y, 1, 1);
}

static PIXELVAL linear32_readpixel(pSD psd, INT32 x, INT32 y)
{
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	return *((ADDR32)addr);
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void linear32_drawhorzline(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c)
{
	register unsigned char *addr = psd->addr + y * psd->pitch + (x1 << 2);
	INT32	gr_mode = psd->mode;
	PIXELVAL gr_background = psd->background;
	int width = x2-x1+1;

	//DRAWON;
	if(gr_mode == ROP_COPY)
	{
		int w = width;
		while (--w >= 0)
		{
			*((ADDR32)addr) = c;
			addr += 4;
		}
	}
	else
		APPLYOP(gr_mode, width, (UINT32), c, *(ADDR32), addr, 0, 4);
	//DRAWOFF;

	if (psd->Update) psd->Update(psd, x1, y, width, 1);
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void linear32_drawvertline(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c)
{
	int	pitch = psd->pitch;
	register unsigned char *addr = psd->addr + y1 * pitch + (x << 2);
	int height = y2-y1+1;
	INT32	gr_mode = psd->mode;
	PIXELVAL gr_background = psd->background;

	//DRAWON;
	if(gr_mode == ROP_COPY)
	{
		int h = height;
		while (--h >= 0)
		{
			*((ADDR32)addr) = c;
			addr += pitch;
		}
	}
	else
		APPLYOP(gr_mode, height, (UINT32), c, *(ADDR32), addr, 0, pitch);
	//DRAWOFF;

	if (psd->Update) psd->Update(psd, x, y1, 1, height);
}

static void gen_fillrect(pSD psd,INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c)
{
	/* temporarily stop updates for speed*/
	void (*Update)(pSD psd, INT32 x, INT32 y, INT32 width, INT32 height) = psd->Update;
	int X1 = x1;
	int Y1 = y1;
	psd->Update = NULL;

	if (psd->portrait & (PORTRAIT_LEFT|PORTRAIT_RIGHT))
		while(x1 <= x2)
			psd->DrawVertLine(psd, x1++, y1, y2, c);
	else
		while(y1 <= y2)
			psd->DrawHorzLine(psd, x1, x2, y1++, c);

	/* now redraw once if external update required*/
	if (Update) {
		Update(psd, X1, Y1, x2-X1+1, y2-Y1+1);
		psd->Update = Update;
	}
}

void fbportrait_down_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c)
{
	x = psd->xvirtres-x-1;
	psd->orgsubdriver->DrawPixel(psd, x, psd->yvirtres-y-1, c);
}

PIXELVAL fbportrait_down_readpixel(pSD psd, INT32 x, INT32 y)
{
	x = psd->xvirtres-x-1;
	return psd->orgsubdriver->ReadPixel(psd, x, psd->yvirtres-y-1);
}

void fbportrait_down_drawhorzline(pSD psd, INT32 x1, INT32 x2, INT32 y, PIXELVAL c)
{
	x1 = psd->xvirtres-x1-1;
	x2 = psd->xvirtres-x2-1;
	psd->orgsubdriver->DrawHorzLine(psd, x2, x1, psd->yvirtres-y-1, c);
}

void fbportrait_down_drawvertline(pSD psd, INT32 x, INT32 y1, INT32 y2, PIXELVAL c)
{
	x = psd->xvirtres-x-1;
	psd->orgsubdriver->DrawVertLine(psd, x, psd->yvirtres-y2-1, psd->yvirtres-y1-1, c);
}

void fbportrait_down_fillrect(pSD psd, INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c)
{
	/* temporarily stop updates for speed*/
	void (*Update)(pSD psd, INT32 x, INT32 y, INT32 width, INT32 height) = psd->Update;
	INT32 Y1 = y1;
	psd->Update = NULL;

	//y2 = psd->yvirtres-y2-1;
	//y1 = psd->yvirtres-y1-1;
	//x1 = psd->xvirtres-x1-1;
	//x2 = psd->xvirtres-x2-1;
	while (y1 <= y2)
		psd->DrawHorzLine(psd, x1, x2, y1++, c);

	/* now redraw once if external update required*/
	if (Update) {
		INT32 W = x2-x1+1;
		INT32 H = y2-Y1+1;
		y2 = psd->yvirtres-y2-1;
		x2 = psd->xvirtres-x2-1;
		Update(psd, x2, y2, W, H);
		psd->Update = Update;
	}
}

void fbportrait_left_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c)
{
	psd->orgsubdriver->DrawPixel(psd, y, psd->xvirtres-x-1, c);
}

PIXELVAL fbportrait_left_readpixel(pSD psd,INT32 x, INT32 y)
{
	return psd->orgsubdriver->ReadPixel(psd, y, psd->xvirtres-x-1);
}

void fbportrait_left_drawhorzline(pSD psd,INT32 x1, INT32 x2, INT32 y, PIXELVAL c)
{
	/*x2 = psd->xvirtres-x2-1;
	while (x2 <= (psd->xvirtres-x1-1))
		fbportrait_left_drawpixel(psd, y, x2++, c);*/

	psd->orgsubdriver->DrawVertLine(psd, y, psd->xvirtres-x2-1, psd->xvirtres-x1-1, c);
}

void fbportrait_left_drawvertline(pSD psd,INT32 x, INT32 y1, INT32 y2, PIXELVAL c)
{
	/*while (y1 <= y2)
		fbportrait_left_drawpixel(psd, y1++, psd->xvirtres-x-1, c);*/

	psd->orgsubdriver->DrawHorzLine(psd, y1, y2, psd->xvirtres-x-1, c);
}

void fbportrait_left_fillrect(pSD psd,INT32 x1, INT32 y1, INT32 x2, INT32 y2, PIXELVAL c)
{
	/* temporarily stop updates for speed*/
	void (*Update)(pSD psd, INT32 x, INT32 y, INT32 width, INT32 height) = psd->Update;
	INT32 X2;
	INT32 W = y2-y1+1;
	INT32 H = x2-x1+1;
	psd->Update = NULL;

	x1 = psd->xvirtres-x1-1;
	X2 = x2 = psd->xvirtres-x2-1;
	while(x2 <= x1)
		psd->orgsubdriver->DrawHorzLine(psd, y1, y2, x2++, c);

	/* now redraw once if external update required*/
	if (Update) {
		Update(psd, y1, X2, W, H);
		psd->Update = Update;
	}
}

void fbportrait_right_drawpixel(pSD psd,INT32 x, INT32 y, PIXELVAL c)
{
	psd->orgsubdriver->DrawPixel(psd, psd->yvirtres-y-1, x, c);
}

PIXELVAL fbportrait_right_readpixel(pSD psd,INT32 x, INT32 y)
{
	return psd->orgsubdriver->ReadPixel(psd, psd->yvirtres-y-1, x);
}

void fbportrait_right_drawhorzline(pSD psd,INT32 x1, INT32 x2, INT32 y, PIXELVAL c)
{
	/*x2 = x2;
	while (x2 <= x1)
		fbportrait_right_drawpixel(psd, psd->yvirtres-y-1, x2++, c);*/

	psd->orgsubdriver->DrawVertLine(psd, psd->yvirtres-y-1, x1, x2, c);
}

void fbportrait_right_drawvertline(pSD psd,INT32 x, INT32 y1, INT32 y2, PIXELVAL c)
{
	/*while (y1 <= y2)
		fbportrait_right_drawpixel(psd, psd->yvirtres-1-y1++, x, c);*/

	psd->orgsubdriver->DrawHorzLine(psd, psd->yvirtres-y2-1, psd->yvirtres-y1-1, x, c);
}

void fbportrait_right_fillrect(pSD psd,INT32 x1, INT32 y1, INT32 x2, INT32 y2,
	PIXELVAL c)
{
	/* temporarily stop updates for speed*/
	void (*Update)(pSD psd, INT32 x, INT32 y, INT32 width, INT32 height) = psd->Update;
	INT32 X1 = x1;
	INT32 H = x2-x1+1;
	psd->Update = NULL;

	while(x1 <= x2)
		psd->orgsubdriver->DrawHorzLine(psd, psd->yvirtres-y2-1, psd->yvirtres-y1-1, x1++, c);

	/* now redraw once if external update required*/
	if (Update) {
		Update(psd, psd->yvirtres-y2-1, X1, y2-y1+1, H);
		psd->Update = Update;
	}
}

/* BGRA subdriver*/
static SubDriver_t fblinear32bgra_none = {
	linear32_drawpixel,
	linear32_readpixel,
	linear32_drawhorzline,
	linear32_drawvertline,
	gen_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_bgra,		/* ft2 non-alias*/
	convblit_copy_mask_mono_byte_lsb_bgra,		/* t1 non-alias*/
	convblit_copy_mask_mono_word_msb_bgra,		/* core/pcf non-alias*/
	convblit_blend_mask_alpha_byte_bgra,		/* ft2/t1 antialias*/
	convblit_copy_rgba8888_bgra8888,			/* RGBA image copy (GdArea PF_RGB)*/
	convblit_srcover_rgba8888_bgra8888,			/* RGBA images w/alpha*/
	convblit_copy_rgb888_bgra8888,				/* RGB images no alpha*/
	frameblit_stretch_rgba8888_bgra8888			/* RGBA stretchblit*/
};

static SubDriver_t fblinear32bgra_left = {
	fbportrait_left_drawpixel,
	fbportrait_left_readpixel,
	fbportrait_left_drawhorzline,
	fbportrait_left_drawvertline,
	fbportrait_left_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_bgra,
	convblit_copy_mask_mono_byte_lsb_bgra,
	convblit_copy_mask_mono_word_msb_bgra,
	convblit_blend_mask_alpha_byte_bgra,
	convblit_copy_rgba8888_bgra8888,
	convblit_srcover_rgba8888_bgra8888,
	convblit_copy_rgb888_bgra8888,
	frameblit_stretch_rgba8888_bgra8888
};

static SubDriver_t fblinear32bgra_right = {
	fbportrait_right_drawpixel,
	fbportrait_right_readpixel,
	fbportrait_right_drawhorzline,
	fbportrait_right_drawvertline,
	fbportrait_right_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_bgra,
	convblit_copy_mask_mono_byte_lsb_bgra,
	convblit_copy_mask_mono_word_msb_bgra,
	convblit_blend_mask_alpha_byte_bgra,
	convblit_copy_rgba8888_bgra8888,
	convblit_srcover_rgba8888_bgra8888,
	convblit_copy_rgb888_bgra8888,
	frameblit_stretch_rgba8888_bgra8888
};

static SubDriver_t fblinear32bgra_down = {
	fbportrait_down_drawpixel,
	fbportrait_down_readpixel,
	fbportrait_down_drawhorzline,
	fbportrait_down_drawvertline,
	fbportrait_down_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_bgra,
	convblit_copy_mask_mono_byte_lsb_bgra,
	convblit_copy_mask_mono_word_msb_bgra,
	convblit_blend_mask_alpha_byte_bgra,
	convblit_copy_rgba8888_bgra8888,
	convblit_srcover_rgba8888_bgra8888,
	convblit_copy_rgb888_bgra8888,
	frameblit_stretch_rgba8888_bgra8888
};

pSubDriver fblinear32bgra[4] = {
	&fblinear32bgra_none, &fblinear32bgra_left, &fblinear32bgra_right, &fblinear32bgra_down
};

/* RGBA subdriver*/
static SubDriver_t fblinear32rgba_none = {
	linear32_drawpixel,
	linear32_readpixel,
	linear32_drawhorzline,
	linear32_drawvertline,
	gen_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_rgba,		/* ft2 non-alias*/
	convblit_copy_mask_mono_byte_lsb_rgba,		/* t1 non-alias*/
	convblit_copy_mask_mono_word_msb_rgba,		/* core/pcf non-alias*/
	convblit_blend_mask_alpha_byte_rgba,		/* ft2/t1 antialias*/
	convblit_copy_rgba8888_rgba8888,			/* RGBA image copy (GdArea PF_RGB)*/
	convblit_srcover_rgba8888_rgba8888,			/* RGBA images w/alpha*/
	convblit_copy_rgb888_rgba8888,				/* RGB images no alpha*/
	frameblit_stretch_xxxa8888					/* RGBA -> RGBA stretchblit*/
};

static SubDriver_t fblinear32rgba_left = {
	fbportrait_left_drawpixel,
	fbportrait_left_readpixel,
	fbportrait_left_drawhorzline,
	fbportrait_left_drawvertline,
	fbportrait_left_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_rgba,
	convblit_copy_mask_mono_byte_lsb_rgba,
	convblit_copy_mask_mono_word_msb_rgba,
	convblit_blend_mask_alpha_byte_rgba,
	convblit_copy_rgba8888_rgba8888,
	convblit_srcover_rgba8888_rgba8888,
	convblit_copy_rgb888_rgba8888,
	frameblit_stretch_xxxa8888
};

static SubDriver_t fblinear32rgba_right = {
	fbportrait_right_drawpixel,
	fbportrait_right_readpixel,
	fbportrait_right_drawhorzline,
	fbportrait_right_drawvertline,
	fbportrait_right_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_rgba,
	convblit_copy_mask_mono_byte_lsb_rgba,
	convblit_copy_mask_mono_word_msb_rgba,
	convblit_blend_mask_alpha_byte_rgba,
	convblit_copy_rgba8888_rgba8888,
	convblit_srcover_rgba8888_rgba8888,
	convblit_copy_rgb888_rgba8888,
	frameblit_stretch_xxxa8888
};

static SubDriver_t fblinear32rgba_down = {
	fbportrait_down_drawpixel,
	fbportrait_down_readpixel,
	fbportrait_down_drawhorzline,
	fbportrait_down_drawvertline,
	fbportrait_down_fillrect,
	NULL,			/* no fallback Blit - uses BlitFrameBlit*/
	frameblit_xxxa8888,
	frameblit_stretch_xxxa8888,
	convblit_copy_mask_mono_byte_msb_rgba,
	convblit_copy_mask_mono_byte_lsb_rgba,
	convblit_copy_mask_mono_word_msb_rgba,
	convblit_blend_mask_alpha_byte_rgba,
	convblit_copy_rgba8888_rgba8888,
	convblit_srcover_rgba8888_rgba8888,
	convblit_copy_rgb888_rgba8888,
	frameblit_stretch_xxxa8888
};

pSubDriver fblinear32rgba[4] = {
	&fblinear32rgba_none, &fblinear32rgba_left, &fblinear32rgba_right, &fblinear32rgba_down
};

void setSubDriver(pSD psd, pSubDriver subdriver)
{
	psd->DrawPixel 				= subdriver->DrawPixel;
	psd->ReadPixel 				= subdriver->ReadPixel;
	psd->DrawHorzLine 			= subdriver->DrawHorzLine;
	psd->DrawVertLine 			= subdriver->DrawVertLine;
	psd->FillRect	 			= subdriver->FillRect;
	psd->BlitFallback 			= subdriver->BlitFallback;
	psd->FrameBlit 				= subdriver->FrameBlit;
	psd->FrameStretchBlit 		= subdriver->FrameStretchBlit;
	psd->BlitCopyMaskMonoByteMSB= subdriver->BlitCopyMaskMonoByteMSB;
	psd->BlitCopyMaskMonoByteLSB= subdriver->BlitCopyMaskMonoByteLSB;
	psd->BlitCopyMaskMonoWordMSB= subdriver->BlitCopyMaskMonoWordMSB;
	psd->BlitBlendMaskAlphaByte = subdriver->BlitBlendMaskAlphaByte;
	psd->BlitCopyRGBA8888     	= subdriver->BlitCopyRGBA8888;
	psd->BlitSrcOverRGBA8888    = subdriver->BlitSrcOverRGBA8888;
	psd->BlitCopyRGB888         = subdriver->BlitCopyRGB888;
	psd->BlitStretchRGBA8888    = subdriver->BlitStretchRGBA8888;
}

pSubDriver selectSubDriver(pSD psd)
{
	pSubDriver *pdriver = NULL;
	
	if (psd->data_format == IF_RGBA8888) pdriver = fblinear32rgba;
	else pdriver = fblinear32bgra;

	psd->orgsubdriver	= pdriver[0];
	psd->left_subdriver	= pdriver[1];
	psd->right_subdriver= pdriver[2];
	psd->down_subdriver	= pdriver[3];
	return pdriver[0];
}


