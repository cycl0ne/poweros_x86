#include "windows.h"
#include "screens.h"
#include "tagitem.h"
#include "utility_funcs.h"
#include "region_funcs.h"

#define SysBase IBase->ib_SysBase
//#define UtilBase IBase->ib_UtilBase
#define RegionBase IBase->ib_RgnBase

struct ViewPort *cgfx_CreateVPort(CoreGfxBase *CoreGfxBase, PixMap *pix, INT32 xOffset, INT32 yOffset);
struct View *cgfx_CreateView(CoreGfxBase *CoreGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 bpp);
BOOL cgfx_MakeVPort(CoreGfxBase *CoreGfxBase, struct View *view, struct ViewPort *vp);
void cgfx_LoadView(CoreGfxBase *CoreGfxBase, struct View *view);
PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 format, UINT32 flags, APTR pixels, int palsize);
struct CRastPort *cgfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm);

#if 0
	struct nWindow		root;
	struct nWindow		*list;
	CGfxFont			*Font;
    struct ViewPort 	*ViewPort;
	INT32				MouseX, MouseY;
	UINT32				Flags;
	CSTRPTR				DefaultTitle;
    SignalSemaphore		LockScreen;
#endif

struct nScreen *intu_OpenScreenTag(IntuitionBase *IBase,  struct TagItem *tagList)
{
	APTR UtilBase = IBase->ib_UtilBase;
	struct nScreen *ret = AllocVec(sizeof(struct nScreen), MEMF_CLEAR|MEMF_FAST);
	if (ret)
	{
		ret->root.x		= (INT32)  GetTagData(SA_Left	,   0, tagList);
		ret->root.y		= (INT32)  GetTagData(SA_Left	,   0, tagList);
		ret->root.width	= (INT32)  GetTagData(SA_Width	, 640, tagList);
		ret->root.height= (INT32)  GetTagData(SA_Height	, 480, tagList);

		ret->root.parent		= NULL;
		ret->root.siblings		= NULL;
		ret->root.children		= NULL;
		ret->root.realized		= TRUE;
		ret->root.mapped		= TRUE;
		ret->root.clipregion	= NULL;
		ret->root.buffer		= NULL;

		ret->root.screen		= ret;
		ret->root.bordercolor	= BLACK;
		ret->root.bordersize	= 0;
		ret->root.background	= BACKGROUND;
		ret->root.title			= (STRPTR) GetTagData(SA_Title	, (UINT32)"Workbench Screen", tagList);
		ret->root.owner			= FindTask(NULL);

		ret->root.buffer		= cgfx_AllocPixMap(IBase->ib_GfxBase, ret->root.width, ret->root.height, IF_BGRA8888, FPM_Displayable, NULL,0 );
		ret->root.frp			= cgfx_InitRastPort(IBase->ib_GfxBase, ret->root.buffer);
		ret->ViewPort			= cgfx_CreateVPort(IBase->ib_GfxBase, ret->root.buffer, 0, 0);
		
		ret->list = NULL;
		ret->Font		= (CGfxFont*) GetTagData(SA_Font, (UINT32)IBase->ib_SystemFont[0], tagList);

		InitSemaphore(&ret->LockScreen);
		
		if (IBase->ib_ActiveScreen == NULL)
		{
			DPrintF("ActiveScreen\n");
			IBase->ib_ViewMaster= cgfx_CreateView(IBase->ib_GfxBase, ret->root.width, ret->root.height, 32);
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
