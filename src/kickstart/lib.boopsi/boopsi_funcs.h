#ifndef boopsi_funcs_h
#define boopsi_funcs_h

#if 0
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
#endif

#define LockClassList()			(((void(*)(APTR))				_GETVECADDR(BOOPSIBase, 5))(BOOPSIBase))
#define UnlockClassList()		(((void(*)(APTR))				_GETVECADDR(BOOPSIBase, 6))(BOOPSIBase))
#define AddClass(a)				(((void(*)(APTR, Class *))		_GETVECADDR(BOOPSIBase, 7))(BOOPSIBase, a))
#define FreeClass(a)			(((BOOL(*)(APTR, Class *))		_GETVECADDR(BOOPSIBase, 8))(BOOPSIBase, a))
#define FindClass(a)			(((Class*(*)(APTR, ClassID))	_GETVECADDR(BOOPSIBase, 9))(BOOPSIBase, a))
#define MakeClass(a,b,c,d,e)	(((Object*(*)(APTR,ClassID,ClassID,Class *,UINT16,UINT32))	_GETVECADDR(BOOPSIBase,10))(BOOPSIBase, a,b,c,d,e))
#define RemoveClass(a)			(((void(*)(APTR, Class *))		_GETVECADDR(BOOPSIBase,11))(BOOPSIBase, a))
#define NewObjectA(a,b,c) 		(((Object *(*)(pBOOPSI, Class *, ClassID, struct TagItem *))	_GETVECADDR(BOOPSIBase,12))(BOOPSIBase, a,b,c))
#define NewObject(a,b,c...)		(((Object *(*)(pBOOPSI, Class *, ClassID, Tag, ...))			_GETVECADDR(BOOPSIBase,13))(BOOPSIBase, a,b,##c))
#define DisposeObject(a)		(((void(*)(pBOOPSI, Object *))									_GETVECADDR(BOOPSIBase,14))(BOOPSIBase, a))
#define GetAttr(a,b,c) 			(((UINT32(*)(pBOOPSI, UINT32, Object*,  UINT32*))				_GETVECADDR(BOOPSIBase,15))(BOOPSIBase, a,b,c))
#define SetAttrsA(a,b) 			(((UINT32(*)(pBOOPSI, Object *o, struct TagItem *))				_GETVECADDR(BOOPSIBase,16))(BOOPSIBase, a,b))
#define SetAttrs(a,b...)		(((UINT32(*)(pBOOPSI, Object *o, Tag, ...))						_GETVECADDR(BOOPSIBase,17))(BOOPSIBase, a,##b))


#define CoerceMessageA(a,b)		(((UINT32(*)(pBOOPSI, Class *,Object *, Msg msg))				_GETVECADDR(BOOPSIBase,18))(BOOPSIBase, a,b))
#define CoerceMessage(a,b...)	(((UINT32(*)(pBOOPSI, Class *,Object *, Msg msg, ...))			_GETVECADDR(BOOPSIBase,19))(BOOPSIBase, a,##b))
#define DoMethodA(a,b)			(((UINT32(*)(pBOOPSI, Object *o, Msg MethodID))					_GETVECADDR(BOOPSIBase,20))(BOOPSIBase, a,b))
#define DoMethod(a,b...)		(((UINT32(*)(pBOOPSI, Object *o, Msg MethodID, ...))			_GETVECADDR(BOOPSIBase,21))(BOOPSIBase, a,##b))


#endif
