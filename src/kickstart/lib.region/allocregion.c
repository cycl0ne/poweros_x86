#include "regionbase.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

ClipRegion *reg_AllocRegion(RegionBase *RegionBase)
{
//DPrintF("allocregion()\n");
	ClipRegion *rgn;

	if ((rgn = AllocVec(sizeof(ClipRegion), MEMF_FAST|MEMF_CLEAR )))
	{
		if ((rgn->rects = AllocVec(sizeof(Rect), MEMF_FAST|MEMF_CLEAR)))
		{
			rgn->size = 1;
			EMPTY_REGION(rgn);
			return rgn;
		}
		FreeVec(rgn);
	}
	return NULL;
}

ClipRegion *reg_AllocRectRegion(RegionBase *RegionBase, INT32 left, INT32 top, INT32 right, INT32 bottom)
{
	ClipRegion *rgn;
	rgn = AllocRegion();
	if (rgn) SetRectRegion(rgn, left, top, right, bottom);
	return rgn;
}

ClipRegion *reg_AllocRectRegionIndirect(RegionBase *RegionBase, Rect *prc)
{
	return AllocRectRegion(prc->left, prc->top, prc->right, prc->bottom);
}

//GdRealloc(addr,oldsize,newsize)
#undef SysBase
extern APTR g_SysBase; // Lousy HACK !!!

APTR Realloc(APTR oldAddr, UINT32 oldSize, UINT32 newSize)
{
	APTR SysBase = g_SysBase;
	//DPrintF("Realloc Called %x, %x, %x\n", oldAddr, oldSize, newSize);
	APTR ret = AllocVec(newSize, MEMF_FAST|MEMF_CLEAR);
	if (ret)
	{
		memcpy(ret, oldAddr, oldSize);
		FreeVec(oldAddr);
	}
	return ret;
}

