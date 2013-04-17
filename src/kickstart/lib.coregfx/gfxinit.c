#include "coregfx.h"
#include "vgagfx.h"
#include "pixmap.h"
#include "font.h"

#define LIBRARY_VERSION_STRING "\0$VER: coregfx.library 0.2 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "coregfx.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static CoreGfxBase *cgfx_Init(CoreGfxBase *CoreGfxBase, UINT32 *segList, APTR SysBase);
APTR cgfx_OpenLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_CloseLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_ExpungeLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_ExtFuncLib(CoreGfxBase *CoreGfxBase);

INT32 cgfx_ClipArea(CoreGfxBase *CoreGfxBase, 		CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2);
BOOL cgfx_ClipPoint(CoreGfxBase *CoreGfxBase, 		CRastPort *rp, INT32 x, INT32 y);
void cgfx_SetClipRegion(CoreGfxBase *CoreGfxBase, 	CRastPort *rp, ClipRegion *reg);
UINT32 cgfx_SetMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode);
UINT32 cgfx_SetFillMode(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 mode);
BOOL cgfx_SetUseBackground(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, BOOL flag);
UINT32 cgfx_SetForegroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg);
UINT32 cgfx_SetBackgroundPixelVal(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg);
UINT32 cgfx_SetForegroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 fg);
UINT32 cgfx_SetBackgroundColor(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 bg);
void cgfx_SetDash(CoreGfxBase *CoreGfxBase, CRastPort *CRastPort, UINT32 *mask, UINT32 *count);
void cgfx_FixCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp);
void cgfx_CheckCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp,INT32 x1,INT32 y1,INT32 x2,INT32 y2);
INT32 cgfx_ShowCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp);
INT32 cgfx_HideCursor(CoreGfxBase *CoreGfxBase, CRastPort *rp);
void cgfx_SetCursor(CoreGfxBase *CoreGfxBase, struct Cursor *pcursor);
BOOL cgfx_GetCursorPos(CoreGfxBase *CoreGfxBase, INT32 *px, INT32 *py);
void cgfx_MoveCursor(CoreGfxBase *CoreGfxBase, INT32 newx, INT32 newy);
UINT32 cgfx_FindNearestColor(CRastPort *rp, int size, UINT32 cr);
UINT32 cgfx_FindColor(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, UINT32 c);
void cgfx_Point(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y);
void cgfx_Line(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint);
void cgfx_Rect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height);
void cgfx_FillRect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 width, INT32 height);

void cgfx_ConvBlitInternal(CoreGfxBase *CoreGfxBase, CRastPort *rp, pCGfxBlitParms gc, BLITFUNC convblit);
void cgfx_StretchBlit(CoreGfxBase *CoreGfxBase, CRastPort *dstrp, INT32 dx1, INT32 dy1, INT32 dx2,
	INT32 dy2, CRastPort *srcrp, INT32 sx1, INT32 sy1, INT32 sx2,INT32 sy2, int rop);
void cgfx_Blit(CoreGfxBase *CoreGfxBase, CRastPort *dstrp, INT32 dstx, INT32 dsty, INT32 width, INT32 height,
	CRastPort *srcrp, INT32 srcx, INT32 srcy, int rop);
void cgfx_ConversionBlit(CoreGfxBase *CoreGfxBase, PixMap *psd, pCGfxBlitParms parms);
BLITFUNC cgfx_FindConvBlit(CoreGfxBase *CoreGfxBase, PixMap *psd, int data_format, int op);
void cgfx_BitmapByPoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y, INT32 width, INT32 height, const UINT16 *imagebits, int clipresult);

int cgfx_ConvertEncoding(CoreGfxBase *CoreGfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags);
void cgfx_GetTextSize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags);
void cgfx_Text(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, pCGfxFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags);
BOOL cgfx_GetFontInfo(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, pCGfxFontInfo pfontinfo);
void cgfx_DestroyFont(CoreGfxBase *CoreGfxBase, pCGfxFont pfont);
int cgfx_SetFontAttr(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int setflags, int clrflags);
int cgfx_SetFontRotation(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int tenthdegrees);
INT16 cgfx_SetFontSize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, INT16 height, INT16 width);
CGfxFont *cgfx_CreateFont(CoreGfxBase *CoreGfxBase, CRastPort *rp, const char *name, UINT16 height, UINT16 width, const pCGfxLogFont plogfont);
void cgfx_GetScreenInfo(CoreGfxBase *CoreGfxBase, CRastPort *rp, pCGfxScreenInfo psi);

void cgfx_Poly(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, int count, CGfxPoint *points);
void cgfx_FillPoly(CoreGfxBase *CoreGfxBase, CRastPort *rp, int count, CGfxPoint *pointtable);

void cgfx_ArcAngle(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type);
void cgfx_Arc(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 ax, INT32 ay, INT32 bx, INT32 by, int type);
void cgfx_Ellipse(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill);

void SVGA_Init(CoreGfxBase *CoreGfxBase);

static volatile APTR FuncTab[] = 
{
	(void(*)) cgfx_OpenLib,
	(void(*)) cgfx_CloseLib,
	(void(*)) cgfx_ExpungeLib,
	(void(*)) cgfx_ExtFuncLib,

	(void(*)) cgfx_ClipArea,
	(void(*)) cgfx_ClipPoint,
	(void(*)) cgfx_SetClipRegion,
	(void(*)) cgfx_SetMode,
	(void(*)) cgfx_SetFillMode,
	(void(*)) cgfx_SetUseBackground,
	(void(*)) cgfx_SetBackgroundPixelVal,
	(void(*)) cgfx_SetBackgroundPixelVal,
	(void(*)) cgfx_SetForegroundColor,
	(void(*)) cgfx_SetBackgroundColor,
	(void(*)) cgfx_SetDash,
	(void(*)) cgfx_FixCursor,
	(void(*)) cgfx_CheckCursor,
	(void(*)) cgfx_ShowCursor,
	(void(*)) cgfx_HideCursor,
	(void(*)) cgfx_SetCursor,
	(void(*)) cgfx_GetCursorPos,
	(void(*)) cgfx_MoveCursor,
	(void(*)) cgfx_FindNearestColor,
	(void(*)) cgfx_FindColor,
	(void(*)) cgfx_Point,
	(void(*)) cgfx_Line,
	(void(*)) cgfx_Rect,
	(void(*)) cgfx_FillRect,
	(void(*)) cgfx_ConvBlitInternal,
	(void(*)) cgfx_StretchBlit,
	(void(*)) cgfx_Blit,
	(void(*)) cgfx_ConversionBlit,
	(void(*)) cgfx_FindConvBlit,
	(void(*)) cgfx_BitmapByPoint,
	(void(*)) cgfx_ConvertEncoding,
	(void(*)) cgfx_GetTextSize,
	(void(*)) cgfx_Text,
	(void(*)) cgfx_GetFontInfo,
	(void(*)) cgfx_DestroyFont,
	(void(*)) cgfx_SetFontAttr,
	(void(*)) cgfx_SetFontRotation,
	(void(*)) cgfx_SetFontSize,
	(void(*)) cgfx_CreateFont,
	(void(*)) cgfx_GetScreenInfo,
	(void(*)) cgfx_Poly,
	(void(*)) cgfx_FillPoly,
	(void(*)) cgfx_ArcAngle,
	(void(*)) cgfx_Arc,
	(void(*)) cgfx_Ellipse,
	(APTR) ((UINT32)-1)
};

static const struct CoreGfxBase CoreGfxLibData =
{
  .Library.lib_Node.ln_Name = (APTR)&name[0],
  .Library.lib_Node.ln_Type = NT_LIBRARY,
  .Library.lib_Node.ln_Pri = 90,

  .Library.lib_OpenCnt = 0,
  .Library.lib_Flags = 0,
  .Library.lib_NegSize = 0,
  .Library.lib_PosSize = 0,
  .Library.lib_Version = LIBRARY_VERSION,
  .Library.lib_Revision = LIBRARY_REVISION,
  .Library.lib_Sum = 0,
  .Library.lib_IDString = (APTR)&version[7]
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(CoreGfxBase),
	(APTR)FuncTab,
	(APTR)&CoreGfxLibData,
	(APTR)cgfx_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK | RTF_AUTOINIT,
	LIBRARY_VERSION,
	NT_LIBRARY,
	65,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

void SVGA_DrawPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop);
//CoreGfxBase->DrawMemoryPixel(super, x, y, rp->crp_Foreground, rp->crp_Mode);

void DrawMemoryPixel32(PixMap *dst, UINT32 x, UINT32 y, UINT32 c, UINT32 mode)
{
	register unsigned char *addr = dst->addr + y * dst->pitch + (x << 2);
	*((UINT32*)addr) = c;
}

void __TestView(CoreGfxBase *CoreGfxBase);
extern CGfxCoreFont gen_fonts[4];

static CoreGfxBase *cgfx_Init(CoreGfxBase *CoreGfxBase, UINT32 *segList, APTR SysBase)
{
	CoreGfxBase->SysBase	= SysBase;

	CoreGfxBase->VgaGfxBase = OpenLibrary("vgagfx.library", 0);
	if (!CoreGfxBase->VgaGfxBase) DPrintF("Failed to open vgagfx.library\n");
	CoreGfxBase->RegionBase = OpenLibrary("region.library", 0);
	if (!CoreGfxBase->RegionBase) DPrintF("Failed to open region.library\n");

	CoreGfxBase->ActiveView = NULL; // Initialize to NULL (No View Opened)
	
	CoreGfxBase->Cursor.buttons = 0;
	CoreGfxBase->Cursor.xpos = 0;
	CoreGfxBase->Cursor.ypos = 0;
	CoreGfxBase->Cursor.minx = MIN_COORD;
	CoreGfxBase->Cursor.miny = MIN_COORD;
	CoreGfxBase->Cursor.maxx = MAX_COORD;
	CoreGfxBase->Cursor.maxy = MAX_COORD;
	CoreGfxBase->Cursor.changed = TRUE;

	/* init cursor position and size info*/
	CoreGfxBase->Cursor.curvisible = 0;
	CoreGfxBase->Cursor.curneedsrestore = FALSE;
	CoreGfxBase->Cursor.curminx = CoreGfxBase->Cursor.minx;
	CoreGfxBase->Cursor.curminy = CoreGfxBase->Cursor.miny;
	CoreGfxBase->Cursor.curmaxx = CoreGfxBase->Cursor.curminx + MAX_CURSOR_SIZE - 1;
	CoreGfxBase->Cursor.curmaxy = CoreGfxBase->Cursor.curminy + MAX_CURSOR_SIZE - 1;
	
	CoreGfxBase->builtin_fonts = gen_fonts;
	CoreGfxBase->user_builtin_fonts = NULL;

//__TestView(CoreGfxBase);
	return CoreGfxBase;
}

static const char EndResident = 0;
