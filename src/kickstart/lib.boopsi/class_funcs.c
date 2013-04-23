#include "boopsibase.h"
#include "memory.h"
#include "tagitem.h"
#include "boopsi_internal.h"
#include "exec_funcs.h"
#include "utility_funcs.h"

#define UtilBase 	BOOPSIBase->UtilBase
#define SysBase 	BOOPSIBase->SysBase

void boopsi_AddClass(pBOOPSI BOOPSIBase, Class *cl )
{
    if ( cl && cl->cl_ID && ! (cl->cl_Flags & CLF_INLIST) )
    {
		LockClassList();
		AddHead( &BOOPSIBase->pubClassList, (struct Node *) cl );
		cl->cl_Flags |= CLF_INLIST;
		UnlockClassList();
    }
}

void boopsi_RemoveClass(pBOOPSI BOOPSIBase, Class *cl )
{
    if ( cl && (cl->cl_Flags & CLF_INLIST) )
    {
		LockClassList();
		Remove( (struct Node *)cl );
		cl->cl_Flags &= ~CLF_INLIST;
		UnlockClassList();
    }
}

BOOL boopsi_FreeClass(pBOOPSI BOOPSIBase, Class *cl )
{
    if ( ! cl ) return TRUE;

    RemoveClass( cl );
    if ( !( cl->cl_SubclassCount || cl->cl_ObjectCount ) )
    {
		if ( cl->cl_Super ) cl->cl_Super->cl_SubclassCount--;
		FreeVec( cl );
		return TRUE;
    }
    return FALSE;
}

Class *boopsi_FindClass(pBOOPSI BOOPSIBase, ClassID classid)
{
	Class *cl;
	LockClassList();
	ForeachNode(&BOOPSIBase->pubClassList, cl)
	{
		if ( Strcmp((STRPTR)cl->cl_ID, (CSTRPTR) classid) )
		{
			UnlockClassList();
			return cl;
		}
	}
	UnlockClassList();
	return NULL;
}

