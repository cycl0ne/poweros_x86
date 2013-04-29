#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"
#include "exec_funcs.h"

#define SysBase BOOPSIBase->SysBase

static UINT32 rootDispatch(pBOOPSI BOOPSIBase, Class * cl, Object * o, Msg msg);

Class *initRootClass(pBOOPSI BOOPSIBase)
{
    extern CSTRPTR RootClassName;
    return ( makePublicClass( BOOPSIBase, (ClassID)RootClassName, NULL, 0, (HOOKFUNC)rootDispatch) );
}

static UINT32 rootDispatch(pBOOPSI BOOPSIBase, Class *cl, Object *o, Msg msg)
{
    Class   *trueclass;
    switch ( msg->MethodID  )
    {
        case OM_NEW:
	        trueclass = (struct IClass *) o;
	        o = (Object *) AllocVec( SIZEOF_INSTANCE(trueclass), MEMF_PUBLIC|MEMF_CLEAR );
        	if ( o )
	        {
	            o = BASEOBJECT( o );
	            OCLASS( o ) = trueclass;
	            trueclass->cl_ObjectCount++;
	        }
	        return ( (UINT32) o );

        case OM_DISPOSE:
	        OCLASS(o)->cl_ObjectCount--;
	        FreeVec( _OBJECT( o ) );
	        return ( NULL );

        case OM_ADDTAIL:
	        AddTail( ((struct opAddTail *)msg)->opat_List, (struct Node *)&_OBJECT(o)->o_Node );
	        break;
    
        case OM_REMOVE:
            Remove( (struct Node *)&_OBJECT( o )->o_Node );
	        break;

       default:
        	return ( 0 );
    }
    return ( 1 );
}
