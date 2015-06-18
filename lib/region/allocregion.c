#include "regionbase.h"

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

ClipRegion *reg_AllocRectRegionIndirect(RegionBase *RegionBase, pRect prc)
{
	return AllocRectRegion(prc->left, prc->top, prc->right, prc->bottom);
}

APTR Realloc(pRegionBase RegionBase, APTR oldAddr, UINT32 oldSize, UINT32 newSize)
{
	KPrintF("Realloc Called %x, %x, %x\n", oldAddr, oldSize, newSize);
	APTR ret = AllocVec(newSize, MEMF_FAST);
	if (ret)
	{
		memcpy(ret, oldAddr, oldSize);
		FreeVec(oldAddr);
	}
	return ret;
}

