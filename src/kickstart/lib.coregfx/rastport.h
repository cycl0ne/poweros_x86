#ifndef RASTPORT_H
#define RASTPORT_H

#include "types.h"
#include "pixmap.h"
#include "layers.h"

typedef struct CRastPort {
	PixMap	*crp_pixmap;
	Layer	*crp_layer;
	UINT32	crp_mode;
	UINT32	crp_dashmask;
	UINT32	crp_dashcount;
	UINT32	crp_fillmode;
	BOOL	crp_usebg;
	UINT32	crp_foreground;
	UINT32	crp_foreground_rgb;
	UINT32	crp_background;
	UINT32	crp_background_rgb;
} CRastPort;

#endif
