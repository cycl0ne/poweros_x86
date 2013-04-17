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
	
}IntuitionBase, *pIntuitionBase;

void InitPool(IntuitionBase *IBase, struct PoolList *pl, INT32 size, UINT32 initNum);
struct PoolNode *GetPool(IntuitionBase *IBase, struct PoolList *pl);
void ReturnPool(IntuitionBase *IBase, struct PoolNode *pn);

#endif
