#ifndef SCREENS_H
#define SCREENS_H

#include "intuitionbase.h"
#include "pixmap.h"
#include "view.h"
#include "font.h"
#include "regions.h"
#include "coregfx.h"
#include "tagitem.h"
#include "windows.h"

typedef struct Screen {
	struct Screen	*NextScreen;
	struct Window	*FirstWindow;
	struct Window	RootWindow;
	
	INT32			LeftEdge, TopEdge;
	INT32			Width, Height;
	INT32			MouseX, MouseY;
	UINT32			Flags;
	STRPTR			Title;
	STRPTR			DefaultTitle;
    INT8			BarHeight, BarVBorder, BarHBorder, MenuVBorder, MenuHBorder;
    INT8			WBorTop, WBorLeft, WBorRight, WBorBottom;
	UINT32			Bordercolor, Background;
	CGfxFont		*Font;
    struct ViewPort *ViewPort;
    CRastPort 		*RastPort;
	PixMap			*PixMap;
	
    SignalSemaphore	LockScreen;
	ClipRegion		*CRegion;
} Screen_t, *pScreen;

#define SA_Dummy	(TAG_USER + 32)
#define SA_Left		(SA_Dummy + 0x0001)
#define SA_Top		(SA_Dummy + 0x0002)
#define SA_Width	(SA_Dummy + 0x0003)
#define SA_Height	(SA_Dummy + 0x0004)
#define SA_Depth	(SA_Dummy + 0x0005)
#define SA_DetailPen	(SA_Dummy + 0x0006)
#define SA_BlockPen	(SA_Dummy + 0x0007)
#define SA_Title	(SA_Dummy + 0x0008)
#define SA_Colors	(SA_Dummy + 0x0009)
#define SA_ErrorCode	(SA_Dummy + 0x000A)
#define SA_Font		(SA_Dummy + 0x000B)
#define SA_SysFont	(SA_Dummy + 0x000C)
#define SA_Type		(SA_Dummy + 0x000D)
#define SA_BitMap	(SA_Dummy + 0x000E)
#define SA_PubName	(SA_Dummy + 0x000F)
#define SA_PubSig	(SA_Dummy + 0x0010)
#define SA_PubTask	(SA_Dummy + 0x0011)
#define SA_DisplayID	(SA_Dummy + 0x0012)
#define SA_DClip	(SA_Dummy + 0x0013)
#define SA_Overscan	(SA_Dummy + 0x0014)

/** booleans **/
#define SA_ShowTitle	(SA_Dummy + 0x0016)
#define SA_Behind	(SA_Dummy + 0x0017)
#define SA_Quiet	(SA_Dummy + 0x0018)
#define SA_AutoScroll	(SA_Dummy + 0x0019)
#define SA_Pens		(SA_Dummy + 0x001A)
#define SA_FullPalette	(SA_Dummy + 0x001B)
#define SA_ColorMapEntries (SA_Dummy + 0x001C)
#define SA_Parent	(SA_Dummy + 0x001D)
#define SA_Draggable	(SA_Dummy + 0x001E)
#define SA_Exclusive	(SA_Dummy + 0x001F)
#define SA_SharePens	(SA_Dummy + 0x0020)
#define SA_BackFill	(SA_Dummy + 0x0021)
#define SA_Interleaved	(SA_Dummy + 0x0022)
#define SA_Colors32	(SA_Dummy + 0x0023)
#define SA_VideoControl	(SA_Dummy + 0x0024)
#define SA_FrontChild	(SA_Dummy + 0x0025)
#define SA_BackChild	(SA_Dummy + 0x0026)
#define SA_LikeWorkbench	(SA_Dummy + 0x0027)
#define SA_MinimizeISG		(SA_Dummy + 0x0029)

#endif
