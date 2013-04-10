#include "regionbase.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

void reg_DestroyRegion(RegionBase *RegionBase, ClipRegion *rgn)
{
	if(rgn) {
		FreeVec(rgn->rects);
		FreeVec(rgn);
	}
}
