#ifndef intuitionbase_h
#define intuitionbase_h

#include "types.h"
#include "sysbase.h"
#include "resident.h"
#include "intupools.h"
#include "io.h"
#include "list.h"
#include "semaphores.h"
#include "view.h"
#include "pixmap.h"
#include "font.h"
#include "coregfx.h"
#include "windows.h"
#include "screens.h"

#define	LIBRARY_NAME			"intuition.library";

#define ISTATELOCK	0
#define IBASELOCK	1
#define IVIEWLOCK	2
#define IRPLOCK		3
#define NUMLOCKS	4

typedef struct IntuitionBase 
{
	struct Library		ib_Library;
	APTR				ib_SysBase;
	APTR				ib_UtilBase;
	APTR				ib_RgnBase;
	APTR				ib_GfxBase;
	struct PoolList		ib_IEFreeList;
	struct List			ib_IEFoodList;
	struct List			ib_IEQueue;
	struct List			ib_IECloneList;

	struct PoolList		ib_ITFreeList;
	struct List			ib_TokenQueue;
	struct List			ib_DeferredQueue;
	
	struct IOStdReq		ib_IIOR;
	struct Interrupt	ib_InputInterrupt;
	Task				*ib_InputDeviceTask;
	SignalSemaphore		ib_Locks[NUMLOCKS];
	
	CRastPort			*ib_RP;
	PixMap				*ib_PixMap;
	struct View			*ib_ViewMaster;
	struct ViewPort		*ib_ViewPort;

	CGfxFont			*ib_SystemFont[2];

	struct Window		*ib_ActiveWindow;
	struct Screen		*ib_ActiveScreen;
	struct Window 		*clipwp;
	UINT32				nextid;
}IntuitionBase, *pIntuitionBase;

#define	GR_DRAW_TYPE_NONE	0	/* none or error */
#define	GR_DRAW_TYPE_WINDOW	1	/* windows */
#define	GR_DRAW_TYPE_PIXMAP	2	/* pixmaps */

void InitPool(IntuitionBase *IBase, struct PoolList *pl, INT32 size, UINT32 initNum);
struct PoolNode *GetPool(IntuitionBase *IBase, struct PoolList *pl);
void ReturnPool(IntuitionBase *IBase, struct PoolNode *pn);

void _ExposeArea(IntuitionBase *IBase, struct Window *wp, INT32 rootx, INT32 rooty, INT32 width, INT32 height, struct Window *stopwp);
void _RedrawScreen(IntuitionBase *IBase, struct Screen *screen);
void _ClearWindow(IntuitionBase *IBase, struct Window *wp, INT32 x, INT32 y, INT32 width, INT32 height, INT32 exposeflag);
void _DrawBorder(IntuitionBase *IBase, Window *wp);
void _SetClipWindow(pIntuitionBase IBase, Window *wp, ClipRegion *userregion, INT32 flags);
void _RealizeWindow(IntuitionBase *IBase, Window  *wp, BOOL temp);
void _MapWindow(IntuitionBase *IBase, struct Window *wp);
UINT32 _PrepareDrawing(IntuitionBase *IBase, struct Window *wp);

#endif
