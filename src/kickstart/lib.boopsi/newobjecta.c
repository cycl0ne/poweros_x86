#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"

Object *boopsi_NewObjectA(pBOOPSI BOOPSIBase, Class *cl, ClassID classid, struct TagItem *tags)
{
	Object  *o = NULL;

	if ( cl ) cl->cl_ObjectCount++;
	else
	{
		LockClassList();
		cl = FindClass(classid);
		if ( cl ) cl->cl_ObjectCount++;
		UnlockClassList();
	}

	if ( cl )
	{
		o =  (Object *) CoerceMessage( cl, (Object *)cl, (Msg)OM_NEW, tags, NULL );
		cl->cl_ObjectCount--;
	}
	return o;
}

Object *boopsi_NewObject(pBOOPSI BOOPSIBase, Class *cl, ClassID classid, Tag tag,...)
{
	return NewObjectA(cl, classid, (struct TagItem *)&tag);
}
