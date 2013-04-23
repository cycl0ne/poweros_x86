#include "boopsibase.h"
#include "memory.h"
#include "tagitem.h"
#include "boopsi_internal.h"
#include "exec_funcs.h"

#define SysBase BOOPSIBase->SysBase

static void stubReturn(void)
{
    return;    
}

Object *boopsi_MakeClass(pBOOPSI BOOPSIBase, ClassID classid, ClassID superid, Class *superclass, UINT16 instsize, UINT32 flags)
{
	Class	*cl = FindClass(classid);

	// If class already exists under this Name
    if ( classid && cl ) return NULL;
    if ( superid )	/* public superclass	*/
    {
		LockClassList();
		superclass = FindClass(superid);
		if ( superclass )
		{
			superclass->cl_SubclassCount++;
			UnlockClassList();
		} else 
		{
			UnlockClassList();
			return NULL;
		}
    } else if (superclass)
    {
		superclass->cl_SubclassCount++;
    }
	cl = (Class *) AllocVec( sizeof(struct IClass), MEMF_FAST | MEMF_CLEAR );
    if ( cl )
    {
		cl->cl_Super		= superclass;
		cl->cl_InstOffset	= superclass? superclass->cl_InstOffset + superclass->cl_InstSize: 0;
		cl->cl_InstSize		= instsize;
		cl->cl_ID	  		= classid;
		cl->cl_Dispatcher.h_Entry = (HOOKFUNC)stubReturn;
		cl->cl_Dispatcher.h_SubEntry = 0;
		cl->cl_Dispatcher.h_Data = (VOID *) 0xDEADC0DE;
    }
    return (Object *)cl;
}

