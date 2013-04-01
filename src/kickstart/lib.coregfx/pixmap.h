#ifndef PIXMAP_H
#define PIXMAP_H
#include "types.h"

#define FPM_Memory		(1<<0)
#define FPM_AllocAddr	(1<<1)
#define FPM_Displayable	(1<<2)
#define FPM_Framebuffer	(1<<30)
#define FPM_AllocVec	(1<<31)

typedef struct PixMap {
	UINT32	flags;
	UINT32	xres, yres;
	UINT32	xvirtres, yvirtres;
	UINT32	planes;
	UINT32	bpp;
	UINT32	data_format;
	UINT32	pixtype;
	UINT32	pitch;
	void	*addr;
	UINT32	memsize;
	UINT32	palsize;
	void	*palette;
	UINT32	ncolors;
} PixMap;

#endif
