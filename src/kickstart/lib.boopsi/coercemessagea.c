#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"

UINT32 boopsi_CoerceMessageA(pBOOPSI BOOPSIBase, Class *class,Object *object, Msg msg)
{
    return CM(class, object, (Msg)&msg);
}

UINT32 boopsi_CoerceMessage(pBOOPSI BOOPSIBase, Class *class,Object *object, Msg msg, ...)
{
    return CM(class, object, (Msg)&msg);
}

