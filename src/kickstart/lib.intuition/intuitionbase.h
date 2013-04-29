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
#include "cursor.h"
#include "windows.h"
#include "screens.h"

#define	LIBRARY_NAME			"intuition.library";

#define ISTATELOCK	0
#define IBASELOCK	1
#define IVIEWLOCK	2
#define IRPLOCK		3
#define NUMLOCKS	4

typedef struct IMousePointer
{
	struct MinNode	imp_Node;
	struct Cursor	imp_Cursor;
} IMPointer, *pIMPointer;

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

	struct nWindow		*ib_ActiveWindow;	// Active Window, who gets all Input
	struct nScreen		*ib_ActiveScreen;	// Active Screen
	struct nWindow 		*clipwp;			// Cache last Window clipped
	UINT32				nextid;				// Just  a Counter
	
	struct List			listcursorp;	/* list of all cursors */
	pIMPointer			stdcursor;		/* root window cursor */
	pIMPointer			curcursor;		/* currently enabled cursor */
	INT32				cursorx;		/* current x position of cursor */
	INT32				cursory;		/* current y position of cursor */
	UINT32				curbuttons;		/* current state of buttons */

}IntuitionBase, *pIntuitionBase;

#define	GR_DRAW_TYPE_NONE	0	/* none or error */
#define	GR_DRAW_TYPE_WINDOW	1	/* windows */
#define	GR_DRAW_TYPE_PIXMAP	2	/* pixmaps */

void InitPool(IntuitionBase *IBase, struct PoolList *pl, INT32 size, UINT32 initNum);
struct PoolNode *GetPool(IntuitionBase *IBase, struct PoolList *pl);
void ReturnPool(IntuitionBase *IBase, struct PoolNode *pn);

void _ExposeArea(IntuitionBase *IBase, struct nWindow *wp, INT32 rootx, INT32 rooty, INT32 width, INT32 height, struct nWindow *stopwp);
void _RedrawScreen(IntuitionBase *IBase, struct nScreen *screen);
void _ClearWindow(IntuitionBase *IBase, struct nWindow *wp, INT32 x, INT32 y, INT32 width, INT32 height, INT32 exposeflag);
void _DrawBorder(IntuitionBase *IBase, struct nWindow *wp);
void _SetClipWindow(pIntuitionBase IBase, struct nWindow *wp, ClipRegion *userregion, INT32 flags);
void _RealizeWindow(IntuitionBase *IBase, struct nWindow  *wp, BOOL temp);
void _UnrealizeWindow(IntuitionBase *IBase, struct nWindow *wp, BOOL temp);
void _MapWindow(IntuitionBase *IBase, struct nWindow *wp);
UINT32 _PrepareDrawing(IntuitionBase *IBase, struct nWindow *wp);

BOOL _CheckOverlap(struct nWindow *topwp, struct nWindow *botwp);


#endif
