#include "intuitionbase.h"
#include "inputevent.h"
#include "pixmap.h"
#include "view.h"
#include "coregfx.h"

#include "font.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

#define BARFILL			RGB(225,225,225)
#define BARLINEDARK		RGB( 96, 96, 96)
#define BARLINELIGHT	RGB(155,155,155)
#define BLACK			RGB(  0,  0,  0)

#define BACKGROUND		RGB(170,170,170)
#define WHITE			RGB(255,255,255)


struct ViewPort *cgfx_CreateVPort(CoreGfxBase *CoreGfxBase, PixMap *pix, INT32 xOffset, INT32 yOffset);
struct View *cgfx_CreateView(CoreGfxBase *CoreGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 bpp);
BOOL cgfx_MakeVPort(CoreGfxBase *CoreGfxBase, struct View *view, struct ViewPort *vp);
void cgfx_LoadView(CoreGfxBase *CoreGfxBase, struct View *view);
PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 format, UINT32 flags, APTR pixels, int palsize);
struct CRastPort *cgfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm);

#define SysBase IBase->ib_SysBase
#define CoreGfxBase IBase->ib_GfxBase

struct IENode {
	struct PoolNode		ien_Node;
	struct InputEvent	ien_IE;
};

struct InputToken {
	struct PoolNode		it_Node;
};

void initIEvents(pIntuitionBase IBase)
{
	InitPool(IBase, &IBase->ib_IEFreeList, sizeof(struct IENode), 10);
	NewList(&IBase->ib_IEFoodList);
	NewList(&IBase->ib_IEQueue);
	NewList(&IBase->ib_IECloneList);	
}

void initStateMachine(pIntuitionBase IBase)
{
    InitPool(IBase, &IBase->ib_ITFreeList, sizeof (struct InputToken), 10 );
	NewList( &IBase->ib_TokenQueue );
	NewList( &IBase->ib_DeferredQueue );	
}

void DrawScreenBar(IntuitionBase *IBase, INT32 x, INT32 y, STRPTR text)
{
	SetForegroundColor(IBase->ib_RP, WHITE);
	FillRect(IBase->ib_RP, 0, 0, x, 15);
	SetForegroundColor(IBase->ib_RP, BLACK);
	Line(IBase->ib_RP,  1, 15, x, 15, TRUE);
	Line(IBase->ib_RP,  x,  0, x, 15, TRUE);
	
	SetUseBackground(IBase->ib_RP, FALSE);
	SetForegroundColor(IBase->ib_RP, BLACK);
	Text(IBase->ib_RP, IBase->ib_SystemFont[1], 3, 12, text, -1, TF_BASELINE);
}

void _DrawScreenBar(IntuitionBase *IBase, INT32 x, INT32 y, STRPTR text)
{
	SetForegroundColor(IBase->ib_RP, BARFILL);
	FillRect(IBase->ib_RP, 0, 0, x, 15);
	SetForegroundColor(IBase->ib_RP, BARLINELIGHT);
	Line(IBase->ib_RP,  0,  0, x,  0, TRUE);
	Line(IBase->ib_RP,  0,  0,   0, 15, TRUE);
	SetForegroundColor(IBase->ib_RP, BARLINEDARK);
	Line(IBase->ib_RP,  1, 15, x, 15, TRUE);
	Line(IBase->ib_RP,  x,  0, x, 15, TRUE);
	
	SetUseBackground(IBase->ib_RP, FALSE);
	SetForegroundColor(IBase->ib_RP, BLACK);
	Text(IBase->ib_RP, IBase->ib_SystemFont[0], 3, 12, text, -1, TF_BASELINE);
}

void initIntuition(pIntuitionBase IBase)
{
	
	for (int i = 0; i < NUMLOCKS; i++) 
    {
		InitSemaphore( &IBase->ib_Locks[i] );
    }
	IBase->ib_RgnBase	= OpenLibrary("region.library" , 0);
	IBase->ib_GfxBase	= OpenLibrary("coregfx.library", 0);
	IBase->ib_UtilBase	= OpenLibrary("utility.library", 0);

#if 0
	DPrintF("IBase: %x\n", IBase);
	DPrintF("RgnBase: %x\n", IBase->ib_RgnBase);
	DPrintF("GfxBase: %x\n", IBase->ib_GfxBase);
	DPrintF("UtilBase: %x\n", IBase->ib_UtilBase);
#endif

	IBase->ib_SystemFont[0] = CreateFont(FONT_SYSTEM_VAR, 0, 0, NULL);
	IBase->ib_SystemFont[1] = CreateFont(FONT_SYSTEM_FIXED, 0, 0, NULL);
#if 0
	static const UINT16 cursorbits[16] = {
	      0xe000, 0x9800, 0x8600, 0x4180,
	      0x4060, 0x2018, 0x2004, 0x107c,
	      0x1020, 0x0910, 0x0988, 0x0544,
	      0x0522, 0x0211, 0x000a, 0x0004
	};

	static const UINT16 cursormask[16] = {
	      0xe000, 0xf800, 0xfe00, 0x7f80,
	      0x7fe0, 0x3ff8, 0x3ffc, 0x1ffc,
	      0x1fe0, 0x0ff0, 0x0ff8, 0x077c,
	      0x073e, 0x021f, 0x000e, 0x0004
	};
#endif
	IBase->curcursor = NULL;
	IBase->cursorx = -1;
	IBase->cursory = -1;

#if 000000
	ShowCursor(psd);
	GrMoveCursor(psd->xvirtres / 2, psd->yvirtres / 2);
	cid = GrNewCursor(16, 16, 0, 0, WHITE, BLACK, (MWIMAGEBITS *)cursorbits, (MWIMAGEBITS *)cursormask);
	GrSetWindowCursor(GR_ROOT_WINDOW_ID, cid);
	stdcursor = GsFindCursor(cid);
#endif

#if 0
	UINT32 xres = 640, yres = 480;
	
	IBase->ib_PixMap 	= cgfx_AllocPixMap(IBase->ib_GfxBase, xres, yres, IF_BGRA8888, FPM_Displayable, NULL,0 );
	IBase->ib_RP 		= cgfx_InitRastPort(IBase->ib_GfxBase, IBase->ib_PixMap);
	SetForegroundColor(IBase->ib_RP, BACKGROUND);
	FillRect(IBase->ib_RP, 0, 0, xres, yres);

	IBase->ib_ViewMaster= cgfx_CreateView(IBase->ib_GfxBase, xres, yres, 32);
	IBase->ib_ViewPort	= cgfx_CreateVPort(IBase->ib_GfxBase, IBase->ib_PixMap, 0, 0);
	cgfx_MakeVPort(IBase->ib_GfxBase, IBase->ib_ViewMaster, IBase->ib_ViewPort);
	cgfx_LoadView(IBase->ib_GfxBase, IBase->ib_ViewMaster);

	DrawScreenBar(IBase, xres-1, 0, "PowerOS Screen");
#endif	
	initStateMachine(IBase);
	initIEvents(IBase);
	IBase->ib_ActiveScreen = NULL;
}

