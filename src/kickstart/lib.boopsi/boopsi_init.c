#include "boopsibase.h"
#include "resident.h"
#include "exec_funcs.h"

#define LIBRARY_VERSION_STRING "\0$VER: boopsi.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "boopsi.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

CSTRPTR ButtonGClassName = BUTTONGCLASS;
CSTRPTR FillRectClassName = FILLRECTCLASS;
CSTRPTR FrameIClassName = FRAMEICLASS;
CSTRPTR FrButtonClassName = FRBUTTONCLASS;
CSTRPTR GadgetClassName = GADGETCLASS;
CSTRPTR GroupGClassName = GROUPGCLASS;
CSTRPTR ICClassName = ICCLASS;
CSTRPTR ImageClassName = IMAGECLASS;
CSTRPTR ITextIClassName = ITEXTICLASS;
CSTRPTR ModelClassName = MODELCLASS;
CSTRPTR PropGClassName = PROPGCLASS;
CSTRPTR RootClassName = ROOTCLASS;
CSTRPTR StrGClassName = STRGCLASS;
CSTRPTR SysIClassName = SYSICLASS;
CSTRPTR PointerClassName = POINTERCLASS;

APTR boopsi_OpenLib(pBOOPSI BOOPSIBase);
APTR boopsi_CloseLib(pBOOPSI BOOPSIBase);
APTR boopsi_ExpungeLib(pBOOPSI BOOPSIBase);
APTR boopsi_ExtFuncLib(pBOOPSI BOOPSIBase);
static pBOOPSI boopsi_Init(pBOOPSI BOOPSIBase, UINT32 *segList, APTR SysBase);

void boopsi_LockClassList(pBOOPSI BOOPSIBase);
void boopsi_UnlockClassList(pBOOPSI BOOPSIBase);
void boopsi_AddClass(pBOOPSI BOOPSIBase, Class *cl );
BOOL boopsi_FreeClass(pBOOPSI BOOPSIBase, Class *cl );
Class *boopsi_FindClass(pBOOPSI BOOPSIBase, ClassID classid);
Object *boopsi_MakeClass(pBOOPSI BOOPSIBase, ClassID classid, ClassID superid, Class *superclass, UINT16 instsize, UINT32 flags);
void boopsi_RemoveClass(pBOOPSI BOOPSIBase, Class *cl );
Object *boopsi_NewObjectA(pBOOPSI BOOPSIBase, Class *cl, ClassID classid, struct TagItem *tags);
Object *boopsi_NewObject(pBOOPSI BOOPSIBase, Class *cl, ClassID classid, Tag tag, ...);
void boopsi_DisposeObject(pBOOPSI BOOPSIBase, Object *o);
UINT32 boopsi_GetAttr(pBOOPSI BOOPSIBase, UINT32 AttrID, Object *o, UINT32 *StoragePtr);
UINT32 boopsi_SetAttrsA(pBOOPSI BOOPSIBase, Object *o, struct TagItem *tags);
UINT32 boopsi_SetAttrs(pBOOPSI BOOPSIBase, Object *o, Tag tag1, ...);
UINT32 boopsi_CoerceMessageA(pBOOPSI BOOPSIBase, Class *class,Object *object, Msg msg);
UINT32 boopsi_CoerceMessage(pBOOPSI BOOPSIBase, Class *class,Object *object, Msg msg, ...);
UINT32 boopsi_DoMethodA(pBOOPSI BOOPSIBase, Object *o, Msg MethodID);
UINT32 boopsi_DoMethod(pBOOPSI BOOPSIBase, Object *o, Msg MethodID, ...);

static volatile APTR FuncTab[] = 
{
	(void(*)) boopsi_OpenLib,
	(void(*)) boopsi_CloseLib,
	(void(*)) boopsi_ExpungeLib,
	(void(*)) boopsi_ExtFuncLib,
	(void(*)) boopsi_LockClassList,
	(void(*)) boopsi_UnlockClassList,
	(void(*)) boopsi_AddClass,
	(void(*)) boopsi_FreeClass,
	(void(*)) boopsi_FindClass,
	(void(*)) boopsi_MakeClass,
	(void(*)) boopsi_RemoveClass,
	(void(*)) boopsi_NewObjectA,
	(void(*)) boopsi_NewObject,
	(void(*)) boopsi_DisposeObject,
	(void(*)) boopsi_GetAttr,
	(void(*)) boopsi_SetAttrsA,
	(void(*)) boopsi_SetAttrs,
	(void(*)) boopsi_CoerceMessageA,
	(void(*)) boopsi_CoerceMessage,
	(void(*)) boopsi_DoMethodA,
	(void(*)) boopsi_DoMethod,
	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(struct BOOPSIBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)boopsi_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	LIBRARY_VERSION,
	NT_LIBRARY,
	21,
	(STRPTR)name,
	(STRPTR)&version[7],
	0,
	&InitTab
};

UINT32 hookEntry(struct Hook *hook, Object *object, Msg message)
{
    return (UINT32)(hook->h_SubEntry)(hook, object, message, hook->h_Data);
}

static pBOOPSI boopsi_Init(pBOOPSI BOOPSIBase, UINT32 *segList, APTR SysBase)
{
	BOOPSIBase->Library.lib_OpenCnt = 0;
	BOOPSIBase->Library.lib_Node.ln_Pri = 0;
	BOOPSIBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	BOOPSIBase->Library.lib_Node.ln_Name = (STRPTR)name;
	BOOPSIBase->Library.lib_Version = LIBRARY_VERSION;
	BOOPSIBase->Library.lib_Revision = LIBRARY_REVISION;
	BOOPSIBase->Library.lib_IDString = (STRPTR)&version[7];	
	BOOPSIBase->SysBase	= SysBase;
	NewList(&BOOPSIBase->pubClassList);
	return BOOPSIBase;
}

static const char EndResident = 0;
