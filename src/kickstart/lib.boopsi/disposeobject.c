#include "boopsibase.h"
#include "boopsi_internal.h"

void boopsi_DisposeObject(pBOOPSI BOOPSIBase, Object *o)
{
    SendMessage( o, (Msg)OM_DISPOSE );
}

