#include "intuitionbase.h"
#include "list.h"
#include "pixmap.h"
#include "view.h"
#include "font.h"
#include "regions.h"
#include "coregfx.h"
#include "windows.h"

#include "exec_funcs.h"
#include "utility_funcs.h"

#define SysBase IBase->ib_SysBase
#define UtilBase IBase->ib_UtilBase

// some new Idea ...

#define WA_Dummy	     (TAG_USER + 99)
#define WA_Left 	     (WA_Dummy + 1)
#define WA_Top		     (WA_Dummy + 2)
#define WA_Width	     (WA_Dummy + 3)
#define WA_Height	     (WA_Dummy + 4)

struct nWindow *int_NewWindowTag(IntuitionBase *IBase, struct nScreen *screen, struct TagItem *tagList)
{
	if (screen == NULL) return NULL;
	struct nWindow *ret;
	
	ret = (struct nWindow *) AllocVec(sizeof(struct Window), MEMF_FAST|MEMF_CLEAR);
	if (ret)
	{
		struct nWindow *parent = &screen->root;

		ret->parent 	= parent;
		ret->siblings	= parent->children;
		ret->children	= NULL;
		parent->children= ret;

		ret->frp	= parent->frp;
		ret->x		= (INT32) GetTagData(WA_Left	,   0, tagList) + parent->x;
		ret->y		= (INT32) GetTagData(WA_Top		,   0, tagList) + parent->y;
		ret->width	= (INT32) GetTagData(WA_Width	,  80, tagList);
		ret->height	= (INT32) GetTagData(WA_Height	,  80, tagList);
		ret->owner	= FindTask(NULL);

		ret->realized	= FALSE;
		ret->mapped		= FALSE;
		ret->clipregion	= NULL;
		ret->buffer		= NULL;

		ret->next = screen->list;
		screen->list	= ret;
		ret->screen		= screen;
		ret->bordercolor= BLACK;
		ret->bordersize	= 1;
		ret->background	= WHITE;
		ret->title		= NULL;
		return ret;
	}
	return NULL;
}

struct nWindow *intu_OpenWindowTags(IntuitionBase *IBase, struct nScreen *screen, struct TagItem *tagList)
{
	struct nWindow *ret = int_NewWindowTag(IBase, screen, tagList);
	if (ret) _MapWindow(IBase, ret);
	return ret;
}


