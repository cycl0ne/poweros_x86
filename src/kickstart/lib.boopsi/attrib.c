#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"

UINT32 boopsi_GetAttr(pBOOPSI BOOPSIBase, UINT32 AttrID, Object *o, UINT32 *StoragePtr)
{
    return ( SendMessage( o, (Msg)OM_GET, AttrID, StoragePtr ) );
}

UINT32 boopsi_SetAttrsA(pBOOPSI BOOPSIBase, Object *o, struct TagItem *tags)
{
    return ( SendMessage( o, (Msg)OM_SET, tags, NULL ) );
}

UINT32 boopsi_SetAttrs(pBOOPSI BOOPSIBase, Object *o, Tag tag1, ...)
{
    return SetAttrsA(o, (struct TagItem *)&tag1);
}
