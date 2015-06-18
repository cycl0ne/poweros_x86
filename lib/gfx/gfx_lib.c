/**
* File: /gfx_lib．c
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"
#include "residents.h"
#include "dos_io.h"

#define LIBRARY_VERSION_STRING "\0$VER: gfx.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

static const char Name[] = "gfx.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static pGfxBase OpenLib(pGfxBase GfxBase)
{
	GfxBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return GfxBase;
}

static pGfxBase CloseLib(pGfxBase GfxBase)
{
	GfxBase->Library.lib_OpenCnt--;
	return NULL;
}

static pGfxBase ExpungeLib(pGfxBase GfxBase)
{
	if (GfxBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

static pGfxBase ExtFuncLib(pGfxBase GfxBase)
{
	if (GfxBase->Library.lib_OpenCnt > 0) return NULL;
	return NULL;
}

extern CoreFont_t gen_fonts[4];

static pGfxBase InitLibrary(pGfxBase GfxBase, pSegment segment, pSysBase SysBase)
{
	GfxBase->SysBase = SysBase;
	GfxBase->RegionBase = OpenLibrary("region.library",0);
//	GfxBase->DOSBase = OpenLibrary("dos.library",0);

	GfxBase->Cursor.buttons = 0;
	GfxBase->Cursor.xpos = 0;
	GfxBase->Cursor.ypos = 0;
	GfxBase->Cursor.minx = MIN_COORD;
	GfxBase->Cursor.miny = MIN_COORD;
	GfxBase->Cursor.maxx = MAX_COORD;
	GfxBase->Cursor.maxy = MAX_COORD;
	GfxBase->Cursor.changed = TRUE;
	GfxBase->Cursor.curvisible = 0;
	GfxBase->Cursor.curneedsrestore = FALSE;
	GfxBase->Cursor.curminx = GfxBase->Cursor.minx;
	GfxBase->Cursor.curminy = GfxBase->Cursor.miny;
	GfxBase->Cursor.curmaxx = GfxBase->Cursor.curminx + MAX_CURSOR_SIZE - 1;
	GfxBase->Cursor.curmaxy = GfxBase->Cursor.curminy + MAX_CURSOR_SIZE - 1;
	
	GfxBase->builtin_fonts = gen_fonts;
	GfxBase->user_builtin_fonts = NULL;
	return GfxBase;
}

//Prototypes
void gfx_ClipArea(void);
void gfx_ClipPoint(void);
void gfx_SetClipRegion(void);
void gfx_SetMode(void);
void gfx_SetFillMode(void);
void gfx_SetUseBackground(void);
void gfx_SetBackgroundPixelVal(void);
void gfx_SetBackgroundPixelVal(void);
void gfx_SetForegroundColor(void);
void gfx_SetBackgroundColor(void);
void gfx_SetDash(void);
void gfx_FixCursor(void);
void gfx_CheckCursor(void);
void gfx_ShowCursor(void);
void gfx_HideCursor(void);
void gfx_SetCursor(void);
void gfx_GetCursorPos(void);
void gfx_MoveCursor(void);
void gfx_FindNearestColor(void);
void gfx_FindColor(void);
void gfx_Point(void);
void gfx_Line(void);
void gfx_Rect(void);
void gfx_RectFill(void);
void gfx_ConvBlitInternal(void);
void gfx_StretchBlit(void);
void gfx_Blit(void);
void gfx_ConversionBlit(void);
void gfx_FindConvBlit(void);
void gfx_BitmapByPoint(void);
void gfx_ConvertEncoding(void);
void gfx_GetTextSize(void);
void gfx_Text(void);
void gfx_GetFontInfo(void);
void gfx_DestroyFont(void);
void gfx_SetFontAttr(void);
void gfx_SetFontRotation(void);
void gfx_SetFontSize(void);
void gfx_CreateFont(void);
void gfx_GetScreenInfo(void);
void gfx_Poly(void);
void gfx_FillPoly(void);
void gfx_ArcAngle(void);
void gfx_Arc(void);
void gfx_Ellipse(void);

/*******************

Function Table

********************/

static volatile APTR FuncTab[] =
{
	(void(*)) OpenLib,
	(void(*)) CloseLib,
	(void(*)) ExpungeLib,
	(void(*)) ExtFuncLib,
	
	(void(*)) gfx_ClipArea,
	(void(*)) gfx_ClipPoint,
	(void(*)) gfx_SetClipRegion,
	(void(*)) gfx_SetMode,
	(void(*)) gfx_SetFillMode,
	(void(*)) gfx_SetUseBackground,
	(void(*)) gfx_SetBackgroundPixelVal,
	(void(*)) gfx_SetBackgroundPixelVal,
	(void(*)) gfx_SetForegroundColor,
	(void(*)) gfx_SetBackgroundColor,
	(void(*)) gfx_SetDash,
	(void(*)) gfx_FixCursor,
	(void(*)) gfx_CheckCursor,
	(void(*)) gfx_ShowCursor,
	(void(*)) gfx_HideCursor,
	(void(*)) gfx_SetCursor,
	(void(*)) gfx_GetCursorPos,
	(void(*)) gfx_MoveCursor,
	(void(*)) gfx_FindNearestColor,
	(void(*)) gfx_FindColor,
	(void(*)) gfx_Point,
	(void(*)) gfx_Line,
	(void(*)) gfx_Rect,
	(void(*)) gfx_RectFill,
	(void(*)) gfx_ConvBlitInternal,
	(void(*)) gfx_StretchBlit,
	(void(*)) gfx_Blit,
	(void(*)) gfx_ConversionBlit,
	(void(*)) gfx_FindConvBlit,
	(void(*)) gfx_BitmapByPoint,
	(void(*)) gfx_ConvertEncoding,
	(void(*)) gfx_GetTextSize,
	(void(*)) gfx_Text,
	(void(*)) gfx_GetFontInfo,
	(void(*)) gfx_DestroyFont,
	(void(*)) gfx_SetFontAttr,
	(void(*)) gfx_SetFontRotation,
	(void(*)) gfx_SetFontSize,
	(void(*)) gfx_CreateFont,
	(void(*)) gfx_GetScreenInfo,
	(void(*)) gfx_Poly,
	(void(*)) gfx_FillPoly,
	(void(*)) gfx_ArcAngle,
	(void(*)) gfx_Arc,
	(void(*)) gfx_Ellipse,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct GfxBase GfxBaseData =
{
	.Library.lib_Node.ln_Name = (APTR)&Name[0],
	.Library.lib_Node.ln_Type = NT_LIBRARY,
	.Library.lib_Node.ln_Pri = 65,
	.Library.lib_OpenCnt = 0,
	.Library.lib_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.Library.lib_NegSize = 0,
	.Library.lib_PosSize = 0,
	.Library.lib_Version = LIBRARY_VERSION,
	.Library.lib_Revision = LIBRARY_REVISION,
	.Library.lib_Sum = 0,
	.Library.lib_IDString = (APTR)&Version[7],
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(GfxBase_t),
	(APTR)FuncTab,
	(APTR)&GfxBaseData,
	(APTR)InitLibrary
};

static const char EndResident = 0;

static const volatile RESIDENT_TAG ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART, // |RTF_AFTERDOS,
	LIBRARY_VERSION,
	NT_LIBRARY,
	65,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};



