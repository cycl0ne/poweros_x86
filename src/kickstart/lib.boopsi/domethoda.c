#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"

UINT32 boopsi_DoMethodA(pBOOPSI BOOPSIBase, Object *o, Msg MethodID)
{
	Class *cl;
    if (o == NULL) return 0;
	cl = OCLASS (o);
    if (cl == NULL) return 0;
	return (UINT32)(cl->cl_Dispatcher.h_Entry) ((APTR)cl, (APTR)o, (APTR)&MethodID);
}

UINT32 boopsi_DoMethod(pBOOPSI BOOPSIBase, Object *o, Msg MethodID, ...)
{
	Class *cl;
    if (o == NULL) return 0;
	cl = OCLASS (o);
    if (cl == NULL) return 0;
	return (UINT32)(cl->cl_Dispatcher.h_Entry) ((APTR)cl, (APTR)o, (APTR)&MethodID);
}
