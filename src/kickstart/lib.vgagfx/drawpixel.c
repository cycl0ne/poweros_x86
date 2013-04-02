#include "vgagfx.h"
#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "exec_funcs.h"

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


extern APTR g_SysBase;
extern APTR g_VgaGfxBase;

void SVGA_DrawPixel32(struct CRastPort *rp, INT32 x,INT32 y,UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);

	if (rp->crp_Mode == ROP_COPY)
		*((UINT32*)addr) = c;
	else
	{
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, 1, (UINT32), c, *(UINT32*), addr, 0, 0);
	}
}

UINT32 SVGA_ReadPixel32(struct CRastPort *rp, INT32 x,INT32 y)
{
	APTR SysBase = g_SysBase;
	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x << 2);
	return *((UINT32*)addr);
}

void SVGA_DrawHorzLine32(struct CRastPort *rp, INT32 x1, INT32 x2, INT32 y, UINT32 c)
{
	APTR SysBase = g_SysBase;

	struct PixMap *psd = rp->crp_PixMap;
	register unsigned char *addr = psd->addr + y * psd->pitch + (x1 << 2);
	int width = x2-x1+1;
	
	if(rp->crp_Mode == ROP_COPY)
	{
		int w = width;
		while (--w >= 0)
		{
			*((UINT32*)addr) = c;
			addr += 4;		
		}
	} else {
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, width, (UINT32), c, *(UINT32*), addr, 0, 4);
	}
}

void SVGA_DrawVertLine32(struct CRastPort *rp, INT32 x, INT32 y1, INT32 y2, UINT32 c)
{
	struct PixMap *psd = rp->crp_PixMap;
//	if (psd == NULL) return;
	int	pitch = psd->pitch;
	register unsigned char *addr = psd->addr + y1 * pitch + (x << 2);
	int height = y2-y1+1;

	if(rp->crp_Mode == ROP_COPY)
	{
		int h = height;
		while (--h >= 0)
		{
			*((UINT32*)addr) = c;
			addr += pitch;
		}
	} else {
		UINT32 gr_background = rp->crp_Background;
		APPLYOP(rp->crp_Mode, height, (UINT32), c, *(UINT32*), addr, 0, pitch)
	}
}

void SVGA_DrawFillRect32(struct CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, UINT32 c)
{
	int X1 = x1;
	int Y1 = y1;
		while(y1 <= y2)
			SVGA_DrawHorzLine32(rp, x1, x2, y1++, c);
}

BOOL SVGA_SelectSubdriver(PixMap *psd)
{
//	APTR SysBase = g_SysBase;
//	DPrintF("planes %d\n", psd->planes);
	if(psd->planes == 1) 
	{
		switch(psd->bpp) 
		{
		case 1:
			break;
		case 2:
			break;
		case 4:
			break;
		case 8:
			break;
		case 16:
			break;
		case 24:
			break;
		case 32:
			if (psd->data_format == IF_RGBA8888)	/* RGBA pixmaps*/
			{
				psd->DrawPixel		= SVGA_DrawPixel32;
				psd->ReadPixel		= SVGA_ReadPixel32;
				psd->DrawHorzLine	= SVGA_DrawHorzLine32;
				psd->DrawVertLine	= SVGA_DrawVertLine32;
				psd->FillRect 		= SVGA_DrawFillRect32;
			}
			else
			{
				psd->DrawPixel		= SVGA_DrawPixel32;
				psd->ReadPixel		= SVGA_ReadPixel32;
				psd->DrawHorzLine	= SVGA_DrawHorzLine32;
				psd->DrawVertLine	= SVGA_DrawVertLine32;
				psd->FillRect 		= SVGA_DrawFillRect32;
			}
			break;
		}
	}
}


