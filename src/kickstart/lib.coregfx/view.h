#ifndef VIEW_H
#define VIEW_H

#include "types.h"
#include "pixmap.h"

struct ViewPort {
	struct ViewPort 	*Next;			// NULL = Only Screen Visible, !NULL = Other Screen visible
	struct CRastPort	*RastPort;
	struct PixMap		*PixMap;		// Screenbitmap
	APTR				oldPMAddr;
	INT32				DWidth,DHeight;		// Actual Width and Height
	INT32				DxOffset,DyOffset;	// At which Offset?
};

struct View {
	struct ViewPort	*vp;	// first visible View
	UINT32	width, height;	// Resolution of the View
	UINT32	bpp;			// BPP of the View
	APTR	fbAddr;			// Framebuffer Address
	UINT32	fbSize;			// Precalculated Size
	//------- TODO: Here Palette for 8BPP
};

#endif
