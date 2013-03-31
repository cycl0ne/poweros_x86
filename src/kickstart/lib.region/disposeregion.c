#include "regionbase.h"
#include "_region_int.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

void region_DisposeRegion(RegionBase *RegionBase, Region *rgn)
{
	ClearRegion(rgn);
    FreeVec(rgn);
}
