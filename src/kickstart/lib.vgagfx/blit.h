#ifndef BLIT_H
#define BLIT_H
#include "coregfx.h"

#define	APPLYOP(op, width, STYPE, s, DTYPE, d, ssz, dsz)	\
	{											\
		int  count = width;						\
		switch (op) {							\
		case ROP_COPY:						\
		case ROP_SRC_OVER:					\
		case ROP_SRC_IN:						\
		case ROP_SRC_ATOP:					\
			while(--count >= 0) {				\
				DTYPE d = STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_XOR_FGBG:					\
		case ROP_PORTERDUFF_XOR:				\
			while(--count >= 0) {				\
				DTYPE d ^= (STYPE s) ^ gr_background; \
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_SRCTRANSCOPY:				\
			while(--count >= 0) {				\
				DTYPE d = (DTYPE d)? DTYPE d: STYPE s; \
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_XOR:							\
			while(--count >= 0) {				\
				DTYPE d ^= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_AND:							\
			while(--count >= 0) {				\
				DTYPE d &= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_OR:							\
			while(--count >= 0) {				\
				DTYPE d |= STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_SRC_OUT:						\
		case ROP_DST_OUT:						\
		case ROP_CLEAR:						\
			while(--count >= 0) {				\
				DTYPE d = 0;					\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_SET:							\
			while(--count >= 0) {				\
				DTYPE d = ~0;					\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_EQUIV:						\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d ^ STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_NOR:							\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d | STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_NAND:						\
			while(--count >= 0) {				\
				DTYPE d = ~(DTYPE d & STYPE s); \
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_INVERT:						\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_COPYINVERTED:				\
			while(--count >= 0) {				\
				DTYPE d = ~STYPE s;				\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_ORINVERTED:					\
			while(--count >= 0) {				\
				DTYPE d |= ~STYPE s;			\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_ANDINVERTED:					\
			while(--count >= 0) {				\
				DTYPE d &= ~STYPE s;			\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_ORREVERSE:					\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d | STYPE s; 	\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_ANDREVERSE:					\
			while(--count >= 0) {				\
				DTYPE d = ~DTYPE d & STYPE s; 	\
				d += dsz; s += ssz; }			\
			break;								\
		case ROP_NOOP:						\
		case ROP_DST_OVER:					\
		case ROP_DST_IN:						\
		case ROP_DST_ATOP:					\
			break;								\
		}										\
	}


typedef struct BlitParameter{
	INT32		op;				/* MWROP operation requested*/
	INT32		data_format;	/* MWIF_ image data format*/
	INT32		width, height;	/* width and height for src and dest*/
	INT32		dstx, dsty;		/* dest x, y*/
	INT32		srcx, srcy;		/* source x, y*/
	UINT32		src_pitch;		/* source row length in bytes*/
	UINT32		fg_colorval;	/* fg color, MWCOLORVAL 0xAARRGGBB format*/
	UINT32		bg_colorval;
	UINT32		fg_pixelval;	/* fg color, hw pixel format*/
	UINT32		bg_pixelval;
	BOOL		usebg;			/* set =1 to draw background*/
	void *		data;			/* input image data GdConversionBlit*/

	/* these items filled in by GdConversionBlit*/
	void *		data_out;		/* output image from conversion blits subroutines*/
	UINT32		dst_pitch;		/* dest row length in bytes*/

	/* used by GdBlit and GdStretchBlit for GdCheckCursor and fallback blit*/
	struct PixMap	*srcpsd;			/* source psd for psd->psd blits*/

	/* used by frameblits only*/
	INT32		src_xvirtres;	/* srcpsd->x/y virtres, used in frameblit for src coord rotation*/
	INT32		src_yvirtres;

	/* used in stretch blits only*/
	INT32		src_x_step;		/* normal steps in source image*/
	INT32		src_y_step;
	INT32		src_x_step_one;	/* 1-unit steps in source image*/
	INT32		src_y_step_one;
	INT32		err_x_step;		/* 1-unit error steps in source image*/
	INT32		err_y_step;
	INT32		err_y;			/* source coordinate error tracking*/
	INT32		err_x;
	INT32		x_denominator;	/* denominator fraction*/
	INT32		y_denominator;

	/* used in palette conversions only*/
	CgfxPalEntry *palette;		/* palette for image*/
	UINT32		transcolor;		/* transparent color in image*/

//	PSD			alphachan;		/* alpha chan for MWROP_BLENDCHANNEL*/
} CGfxBlitParms, *pCGfxBlitParms;

typedef void (*BLITFUNC)(struct PixMap*, pCGfxBlitParms);		/* proto for blitter functions*/

/* extract MWCOLORVAL (0xAABBGGRR) values*/
#define REDVALUE(rgb)	((rgb) & 0xff)
#define GREENVALUE(rgb) (((rgb) >> 8) & 0xff)
#define BLUEVALUE(rgb)	(((rgb) >> 16) & 0xff)
#define ALPHAVALUE(rgb)	(((rgb) >> 24) & 0xff)

#endif
