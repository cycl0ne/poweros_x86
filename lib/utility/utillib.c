#include "utility.h"

APTR util_OpenLib(pUtility UtilBase)
{
	UtilBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(UtilBase);
}

APTR util_CloseLib(pUtility UtilBase)
{
	UtilBase->Library.lib_OpenCnt--;
	return NULL;
}

APTR util_ExpungeLib(pUtility UtilBase)
{
	return NULL;
}

APTR util_ExtFuncLib(pUtility UtilBase)
{
	return NULL;
}
