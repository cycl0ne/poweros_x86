#ifndef RASTPORT_H
#define RASTPORT_H

#include "types.h"
#include "pixmap.h"
#include "layers.h"

typedef struct CRastPort {
	struct PixMap	*crp_PixMap;
	struct Layer	*crp_Layer;
	UINT32	crp_Mode;
	UINT32	crp_Dashmask;
	UINT32	crp_Dashcount;
	UINT32	crp_Fillmode;
	BOOL	crp_useBg;
	UINT32	crp_Foreground;
	UINT32	crp_ForegroundRGB;
	UINT32	crp_Background;
	UINT32	crp_BackgroundRGB;
} CRastPort;

#endif
