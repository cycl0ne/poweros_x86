#include "boopsibase.h"

APTR boopsi_OpenLib(pBOOPSI BOOPSIBase)
{
	BOOPSIBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return BOOPSIBase;
}

APTR boopsi_CloseLib(pBOOPSI BOOPSIBase)
{
	BOOPSIBase->Library.lib_OpenCnt--;
	return NULL;
}

APTR boopsi_ExpungeLib(pBOOPSI BOOPSIBase)
{
	return NULL;
}

APTR boopsi_ExtFuncLib(pBOOPSI BOOPSIBase)
{
	return NULL;
}
