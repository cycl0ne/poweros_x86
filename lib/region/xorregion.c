#include "regionbase.h"

void reg_XorRegion(RegionBase *RegionBase, ClipRegion *dr, ClipRegion *sra, ClipRegion *srb)
{
    ClipRegion *tra, *trb;

    if ((! (tra = AllocRegion())) || (! (trb = AllocRegion()))) return;
    SubtractRegion(tra,sra,srb);
    SubtractRegion(trb,srb,sra);
    UnionRegion(dr,tra,trb);
    DestroyRegion(tra);
    DestroyRegion(trb);
}

