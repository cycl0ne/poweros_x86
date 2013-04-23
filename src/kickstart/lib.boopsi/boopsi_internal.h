#ifndef boopsi_internal_h
#define boopsi_internal_h

#include "boopsibase.h"

#include "types.h"
#include "list.h"
#include "hooks.h"
#include "boopsi.h"

typedef struct tPoint
{
    INT16 X,Y;
} Point;

struct IBox
{
    INT16 Left;
    INT16 Top;
    INT16 Width;
    INT16 Height;
};

#define OM_Dummy 	(0x100)
#define OM_NEW		(0x101)	/* 'object' parameter is "true class"	*/
#define OM_DISPOSE	(0x102)	/* delete self (no parameters)		*/
#define OM_SET		(0x103)	/* set attributes (in tag list)		*/
#define OM_GET		(0x104)	/* return single attribute value	*/
#define OM_ADDTAIL	(0x105)	/* add self to a List (let root do it)	*/
#define OM_REMOVE	(0x106)	/* remove self from list		*/	
#define OM_NOTIFY	(0x107)	/* send to self: notify dependents	*/
#define OM_UPDATE	(0x108)	/* notification message from somebody	*/
#define OM_ADDMEMBER	(0x109)	/* used by various classes with lists	*/
#define OM_REMMEMBER	(0x10A)	/* used by various classes with lists	*/

#define	CLF_INLIST	0x00000001

typedef struct IClass {
    struct Hook		cl_Dispatcher;
    UINT32			cl_Reserved;
    struct IClass	*cl_Super;
    ClassID			cl_ID;

    UINT16			cl_InstOffset;
    UINT16			cl_InstSize;

    UINT32			cl_UserData;
    UINT32			cl_SubclassCount;
    UINT32			cl_ObjectCount;
    UINT32			cl_Flags;
} Class;

struct _Object {
    struct MinNode	o_Node;
    struct IClass	*o_Class;
};

#define _OBJ( o )			((struct _Object *)(o))
#define BASEOBJECT( _obj )	( (Object *) (_OBJ(_obj)+1) )
#define _OBJECT( o )		(_OBJ(o) - 1)
#define OCLASS( o )			( (_OBJECT(o))->o_Class )

#define INST_DATA( cl, o )		((VOID *) (((UINT8 *)o)+cl->cl_InstOffset))
#define SIZEOF_INSTANCE( cl )	((cl)->cl_InstOffset + (cl)->cl_InstSize + sizeof (struct _Object ))

UINT32 hookEntry(struct Hook *hook, Object *object, Msg message);

static inline UINT32 CM(Class *class, Object *object, Msg msg)
{
    struct Hook *tmp;
    if (class == NULL) return 0;
    if (object == NULL) return 0;
    tmp = &class->cl_Dispatcher;
   	return (((UINT32(*)(struct Hook *, Object *, Msg)) (tmp->h_Entry))(tmp, object, msg));
}

static inline UINT32 SSM(Class *class, Object *object, Msg msg)
{
    if (class == NULL) return 0;
    return CM(class->cl_Super, object, msg);
}

static inline UINT32 SM(Object *object, Msg msg)
{
    if (object == NULL) return 0;
    return CM(OCLASS( object) , object, msg);
}

static inline UINT32 SendMessage(Object *object, Msg msg, ...)
{
    return SM(object, (Msg)&msg);
}

static inline UINT32 SendSuperMessage(Class *class, Object *object, Msg msg,...)
{
    return SSM(class, object, (Msg)&msg);
}

static inline void initHook(struct Hook * hook, HOOKFUNC dispatch)
{
    hook->h_Entry = (HOOKFUNC)hookEntry;
    hook->h_SubEntry = (HOOKFUNC)dispatch;
    hook->h_Data = (APTR) 0xdeadc0de;
}

#include "boopsi_funcs.h"

static inline Class *makePublicClass(APTR BOOPSIBase, ClassID classid, ClassID superid, UINT16 instsize, HOOKFUNC dispatch)
{
    Class	*cl = (Class *)MakeClass( classid, superid, (Class *)NULL, instsize, 0 );
    if ( cl )
    {
	    initHook( &cl->cl_Dispatcher, dispatch);
	    AddClass( cl );
    }
    return ( cl );
}

static inline BOOL ptInBox(struct tPoint *pt, struct IBox *box)
{
	INT16 tmp;

	return((BOOL)	((tmp = pt->X - box->Left) >= 0	&&
					(tmp < box->Width)		&&
					(tmp = pt->Y - box->Top) >= 0	&&
					(tmp < box->Height)));
}
#endif
