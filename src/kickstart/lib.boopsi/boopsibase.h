#ifndef boopsibase_h
#define boopsibase_h
#include "types.h"
#include "semaphores.h"
#include "library.h"
#include "tagitem.h"
#include "boopsi.h"
#include "boopsi_internal.h"
#include "boopsi_funcs.h"

typedef struct BOOPSIBase {
	struct Library	Library;
	APTR			SysBase;
	APTR			UtilBase;
	APTR			CoreGfxBase;
	
	SignalSemaphore	LockClass;
	List			pubClassList;
} BOOPSI, *pBOOPSI;

#if 0
void LockClassList();
void UnlockClassList();
Class *FindClass(ClassID classid);
UINT32 CoerceMessageA(Class *class,Object *object, Msg msg);
UINT32 CoerceMessage(Class *class,Object *object, Msg msg, ...);

BOOL FreeClass( Class *cl );
void RemoveClass( Class *cl );
void AddClass( Class *cl );
UINT32 SetAttrsA(Object *o, struct TagItem *tags);
//Class *makePublicClass(pBOOPSI BOOPSIBase, ClassID classid, ClassID superid, UINT16 instsize, HOOKFUNC dispatch);
Object *MakeClass(ClassID classid, ClassID superid, Class *superclass, UINT16 instsize, UINT32 flags);
#endif

#endif
