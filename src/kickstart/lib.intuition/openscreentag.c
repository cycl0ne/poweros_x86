#include "windows.h"
#include "screens.h"
#include "tagitem.h"
#include "utility_funcs.h"
#include "region_funcs.h"

#define SysBase IBase->ib_SysBase
//#define UtilBase IBase->ib_UtilBase
#define RegionBase IBase->ib_RgnBase

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

#if 0
typedef struct Screen {
	struct Screen	*NextScreen;
	struct Window	*FirstWindow;
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

    SignalSemaphore	LockScreen;
	ClipRegion		*CRegion;
} Screen_t, *pScreen;
#endif

pScreen intu_OpenScreenTag(IntuitionBase *IBase,  struct TagItem *tagList)
{
	APTR UtilBase = IBase->ib_UtilBase;
	pScreen ret = AllocVec(sizeof(Screen_t), MEMF_CLEAR|MEMF_FAST);
	if (ret)
	{
		ret->NextScreen	= NULL;
		ret->FirstWindow= NULL;
		ret->LeftEdge	= (INT32)  GetTagData(SA_Left	,   0, tagList);
		ret->TopEdge	= (INT32)  GetTagData(SA_Top	,   0, tagList);
		ret->Width		= (INT32)  GetTagData(SA_Width	, 640, tagList);
		ret->Height		= (INT32)  GetTagData(SA_Height	, 480, tagList);
		ret->Title		= (STRPTR) GetTagData(SA_Title	, (UINT32)"Unknown Screen", tagList);
//		ret->DefaultTitle= (STRPTR) GetTagData(SA_DTitle, (UINT32)"Unknown", tagList);
		ret->Font		= (CGfxFont*) GetTagData(SA_Font, (UINT32)IBase->ib_SystemFont[0], tagList);
		ret->Bordercolor= BLACK;
		ret->Background	= BACKGROUND;
		InitSemaphore(&ret->LockScreen);
		// New Idea !
		ret->PixMap		= cgfx_AllocPixMap(IBase->ib_GfxBase, ret->Width, ret->Height, IF_BGRA8888, FPM_Displayable, NULL,0 );
		ret->RastPort	= cgfx_InitRastPort(IBase->ib_GfxBase, ret->PixMap);
		ret->ViewPort	= cgfx_CreateVPort(IBase->ib_GfxBase, ret->PixMap, 0, 0);

		ret->RootWindow.rp = ret->RastPort;
		ret->RootWindow.id = 0; //RWINDOW_ID;
		ret->RootWindow.parent	= NULL;
		ret->RootWindow.owner	= NULL;
		ret->RootWindow.children= NULL;
		ret->RootWindow.siblings= NULL;
		ret->RootWindow.screen	= ret;
		ret->RootWindow.next	= NULL;
		ret->RootWindow.x		= 0;
		ret->RootWindow.y		= 0;
		ret->RootWindow.width	= ret->Width;
		ret->RootWindow.height	= ret->Height;
		ret->RootWindow.bordersize	= 0;
		ret->RootWindow.background	= BACKGROUND; //BLACK;
		ret->RootWindow.bordercolor	= BLACK;
		ret->RootWindow.clipregion	= NULL;
		ret->RootWindow.realized	= TRUE;
		ret->RootWindow.title		= NULL;
		ret->RootWindow.screenTitle	= NULL;

		// Our First Screen, so open it.
		if (IBase->ib_ActiveScreen == NULL)
		{
			DPrintF("ActiveScreen\n");
			IBase->ib_ViewMaster= cgfx_CreateView(IBase->ib_GfxBase, ret->Width, ret->Height, 32);
			cgfx_MakeVPort(IBase->ib_GfxBase, IBase->ib_ViewMaster, ret->ViewPort);
			cgfx_LoadView(IBase->ib_GfxBase, IBase->ib_ViewMaster);
			IBase->ib_ActiveScreen = ret;
		}
		DPrintF("RedrawScreen\n");
		_RedrawScreen(IBase, ret);
	} else
	{
		DPrintF("AllocVec on Screen failed\n");
	}
	return ret;
}
