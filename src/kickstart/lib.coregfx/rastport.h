#ifndef RASTPORT_H
#define RASTPORT_H

#include "types.h"
#include "regions.h"

#define CLIP_VISIBLE		0
#define CLIP_INVISIBLE		1
#define CLIP_PARTIAL		2

typedef struct CRastPort {
	struct PixMap	*crp_PixMap;
	UINT32	crp_Mode;
	UINT32	crp_DashMask;
	UINT32	crp_DashCount;
	UINT32	crp_FillMode;
	BOOL	crp_useBg;
	UINT32	crp_Foreground;
	UINT32	crp_ForegroundRGB;
	UINT32	crp_Background;
	UINT32	crp_BackgroundRGB;
	INT32	crp_ClipMinX;
	INT32	crp_ClipMinY;
	INT32	crp_ClipMaxX;
	INT32	crp_ClipMaxY;
	BOOL	crp_ClipResult;
	struct ClipRegion	*crp_ClipRegion;
} CRastPort;

#endif
