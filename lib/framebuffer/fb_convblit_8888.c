/**
* File: /fb_convblit．c
* User: cycl0ne
* Date: 2014-11-26
* Time: 08:07 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "framebuffer.h"

#define R		0		/* RGBA parms*/
#define G		1
#define B		2
#define A		3

#define NONE	PORTRAIT_NONE
#define LEFT	PORTRAIT_LEFT
#define RIGHT	PORTRAIT_RIGHT
#define DOWN	PORTRAIT_DOWN

#define COPY	0		/* mode parm*/
#define SRCOVER	1
#define ALWAYS_INLINE __attribute__ ((always_inline))

static inline void ALWAYS_INLINE convblit_8888(pSD psd, pBlitParms gc, int mode,
	int SSZ, int SR, int SG, int SB, int SA,
	int DSZ, int DR, int DG, int DB, int DA, int PORTRAIT)
{
	unsigned char *src, *dst;
	int dsz, dst_pitch;
	int height, tmp;
	int src_pitch = gc->src_pitch;

	/* compiler can optimize out switch statement and most else to constants*/
	switch (PORTRAIT) {
	case NONE:
	default:
		dsz = DSZ;					/* dst: next pixel over*/
		dst_pitch = gc->dst_pitch;	/* dst: next line down*/
		break;

	case LEFT:
		/* change dst top left to lower left for left portrait*/
		/* rotate left: X -> Y, Y -> maxx - X*/
		tmp = gc->dsty;
		gc->dsty = psd->xvirtres - gc->dstx - 1;
		gc->dstx = tmp;

		dsz = -gc->dst_pitch;		/* dst: next row up*/
		dst_pitch = DSZ;			/* dst: next pixel right*/
		break;

	case RIGHT:
		/* change dst top left to upper right for right portrait*/
 		/* Rotate right: X -> maxy - y - h, Y -> X, W -> H, H -> W*/
		tmp = gc->dstx;
		gc->dstx = psd->yvirtres - gc->dsty - 1;
		gc->dsty = tmp;

		dsz = gc->dst_pitch;		/* dst: next pixel down*/
		dst_pitch = -DSZ;			/* dst: next pixel left*/
		break;

	case DOWN:
		/* change dst top left to lower right for down portrait*/
 		/* Rotate down: X -> maxx - x - w, Y -> maxy - y - h*/
		gc->dstx = psd->xvirtres - gc->dstx - 1;
		gc->dsty = psd->yvirtres - gc->dsty - 1;

		dsz = -DSZ;					/* dst: next pixel left*/
		dst_pitch = -gc->dst_pitch;	/* dst: next pixel up*/
		break;
	}

	src = ((unsigned char *)gc->data)     + gc->srcy * gc->src_pitch + gc->srcx * SSZ;
	dst = ((unsigned char *)gc->data_out) + gc->dsty * gc->dst_pitch + gc->dstx * DSZ;

//	DRAWON;
	height = gc->height;
	while (--height >= 0)
	{
		register unsigned char *d = dst;
		register unsigned char *s = src;
		unsigned int alpha;
		int w = gc->width;

		while (--w >= 0)
		{
			/* inline implementation will optimize out all but two compares in inner loop*/
			if (mode == COPY || (alpha = s[SA]) == 255)		/* copy source*/
			{
				if (DSZ == 2)
				{
					if (SSZ == 2)
						((unsigned short *)d)[0] = ((unsigned short *)s)[0];
					else
						((unsigned short *)d)[0] = RGB2PIXEL(s[SR], s[SG], s[SB]);
				}
				else
				{
					if (DA >= 0)			/* compiler will optimize out completely*/
						d[DA] = (SA >= 0)? s[SA]: 255;

					d[DR] = s[SR];
					d[DG] = s[SG];
					d[DB] = s[SB];
				}
			}
			else if (alpha != 0)							/* blend source w/dest*/
			{
				if (DSZ == 2) {
					unsigned short sr = RED2PIXEL(s[SR]);
					unsigned short sg = GREEN2PIXEL(s[SG]);
					unsigned short sb = BLUE2PIXEL(s[SB]);
					alpha = 255 - alpha + 1; /* flip alpha then add 1 (see muldiv255)*/

					/* d = muldiv255(255-a, d - s) + s*/
					((unsigned short *)d)[0] =
						muldiv255_16bpp(((unsigned short *)d)[0], sr, sg, sb, alpha);
				}
				else
				{
 					/* d += muldiv255(a, s - d)*/
					d[DR] += muldiv255(alpha, s[SR] - d[DR]);
					d[DG] += muldiv255(alpha, s[SG] - d[DG]);
					d[DB] += muldiv255(alpha, s[SB] - d[DB]);

 					/* d += muldiv255(a, 255 - d)*/
					if (DA >= 0)
						d[DA] += muldiv255(alpha, 255 - d[DA]);
				}
			}
			d += dsz;
			s += SSZ;				/* src: next pixel right*/
		}
		src += src_pitch;			/* src: next line down*/
		dst += dst_pitch;
	}
//	DRAWOFF;

	/* update screen bits if driver requires it*/
	if (!psd->Update)
		return;

	switch (PORTRAIT) {		/* switch will be optimized out*/
	case NONE:
		psd->Update(psd, gc->dstx, gc->dsty, gc->width, gc->height);
		break;

	case LEFT:
		/* adjust x,y,w,h to physical top left and w/h*/
		psd->Update(psd, gc->dstx, gc->dsty - gc->width + 1, gc->height, gc->width);
		break;

	case RIGHT:
		/* adjust x,y,w,h to physical top left and w/h*/
		psd->Update(psd, gc->dstx - gc->height + 1, gc->dsty, gc->height, gc->width);
		break;

	case DOWN:
		/* adjust x,y,w,h to physical top left and w/h*/
		psd->Update(psd, gc->dstx - gc->width + 1, gc->dsty - gc->height + 1, gc->width, gc->height);
		break;
	}
}

/*---------- 32bpp BGRA output ----------*/

/* MWPF_TRUECOLORABGR and 32bpp RGBA internal pixmaps*/
/* Conversion blit srcover 32bpp RGBA image to 32bpp RGBA image*/
void convblit_srcover_rgba8888_rgba8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, SRCOVER, 4, R,G,B,A, 4, R,G,B,A, psd->portrait);
}

/* Conversion blit copy 32bpp RGBA image to 32bpp RGBA image*/
void convblit_copy_rgba8888_rgba8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, R,G,B,A, 4, R,G,B,A, psd->portrait);
}

/* Conversion blit copy 24bpp RGB image to 32bpp RGBA image*/
void convblit_copy_rgb888_rgba8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 3, R,G,B,-1, 4, R,G,B,A, psd->portrait);
}

/* MWPF_TRUECOLOR8888*/

/* Conversion blit srcover 32bpp RGBA image to 32bpp BGRA image*/
void convblit_srcover_rgba8888_bgra8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, SRCOVER, 4, R,G,B,A, 4, B,G,R,A, psd->portrait);
}

/* Conversion blit copy 32bpp RGBA image to 32bpp BGRA image*/
void convblit_copy_rgba8888_bgra8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, R,G,B,A, 4, B,G,R,A, psd->portrait);
}

/* Conversion blit copy 24bpp RGB image to 32bpp BGRA image*/
void convblit_copy_rgb888_bgra8888(pSD psd, pBlitParms gc)
{
	// -1 forces 255 alpha in destination
	convblit_8888(psd, gc, COPY, 3, R,G,B,-1, 4, B,G,R,A, psd->portrait);
}

/* Copy 32bpp XXXX image to 32bpp XXXX image*/
void convblit_copy_8888_8888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, R,G,B,A, 4, R,G,B,A, psd->portrait);
}

/*---------- 24bpp BGR output ----------*/

/* Conversion blit srcover 32bpp RGBA image to 24bpp BGR image*/
void convblit_srcover_rgba8888_bgr888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, SRCOVER, 4, R,G,B,A, 3, B,G,R,-1, psd->portrait);
}

/* Conversion blit copy 32bpp RGBA image to 24bpp BGR image*/
void convblit_copy_rgba8888_bgr888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, R,G,B,A, 3, B,G,R,-1, psd->portrait);
}

/* Conversion blit copy 24bpp RGB image to 24bpp BGR image*/
void convblit_copy_rgb888_bgr888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 3, R,G,B,-1, 3, B,G,R,-1, psd->portrait);
}

/* Copy 24bpp XXX image to 24bpp XXX image*/
void convblit_copy_888_888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 3, R,G,B,-1, 3, R,G,B,-1, psd->portrait);
}

/* Copy 32bpp BGRA image to 24bpp BGR image (GdArea MWPF_PIXELVAL)*/
void convblit_copy_bgra8888_bgr888(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, B,G,R,A, 3, B,G,R,-1, psd->portrait);
}

/*---------- 16bpp BGR output ----------*/

/* Conversion blit srcover 32bpp RGBA image to 16bpp image*/
void convblit_srcover_rgba8888_16bpp(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, SRCOVER, 4, R,G,B,A, 2, 0,0,0,-1, psd->portrait);
}

/* Conversion blit copy 32bpp RGBA image to 16bpp image*/
void convblit_copy_rgba8888_16bpp(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 4, R,G,B,A, 2, 0,0,0,-1, psd->portrait);
}

/* Conversion blit copy 24bpp RGB image to 16bpp image*/
void convblit_copy_rgb888_16bpp(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 3, R,G,B,-1, 2, 0,0,0,-1, psd->portrait);
}

/* Copy 16bpp image to 16bpp image*/
void convblit_copy_16bpp_16bpp(pSD psd, pBlitParms gc)
{
	convblit_8888(psd, gc, COPY, 2, 0,0,0,-1, 2, 0,0,0,-1, psd->portrait);
}


