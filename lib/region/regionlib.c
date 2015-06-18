#include "regionbase.h"

APTR region_OpenLib(RegionBase *RegionBase)
{
	RegionBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(RegionBase);
}

APTR region_CloseLib(RegionBase *RegionBase)
{
	RegionBase->Library.lib_OpenCnt--;
	return NULL;
}

APTR region_ExpungeLib(RegionBase *RegionBase)
{
	return NULL;
}

APTR region_ExtFuncLib(RegionBase *RegionBase)
{
	return NULL;
}
