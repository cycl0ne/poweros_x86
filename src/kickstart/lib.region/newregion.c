#include "regionbase.h"
#include "_region_int.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

struct Region *region_NewRegion(RegionBase *RegionBase)
{
	return (Region *) AllocVec(sizeof(Region), MEMF_FAST|MEMF_CLEAR);
}

