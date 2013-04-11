#include "utility.h"
#include "hooks.h"

UINT32 util_CallHookPkt(pUtility UtilBase, struct Hook *hook, APTR object, APTR message)
{
	return (UINT32)hook->h_Entry(hook, object, message);
}

