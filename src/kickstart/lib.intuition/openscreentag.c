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
		ret->CRegion	= AllocRectRegion(ret->LeftEdge, ret->TopEdge, ret->Width, ret->Height);
	}
	return ret;
}
