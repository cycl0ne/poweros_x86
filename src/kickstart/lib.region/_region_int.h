
#define OPERATION_OR    0
#define OPERATION_AND	1

void gfx_int_FreeRR(struct RegionBase *RegionBase, struct RegionRectangle *rr);
void gfx_int_RectSplit(struct RegionBase *RegionBase, struct Region *rgn, struct Rectangle r1, struct Rectangle *r2, UINT32 op);
void gfx_int_ReplaceRegion(struct RegionBase *RegionBase, struct Region *from, struct Region *to);
struct RegionRectangle *gfx_int_NewRect(struct RegionBase *RegionBase);
void gfx_int_Intersect(struct Rectangle *a, struct Rectangle *b, struct Rectangle *c);
BOOL gfx_int_Obscured(struct Rectangle *r1, struct Rectangle *r2);
void gfx_int_OffsetRegionRectangle(struct RegionRectangle *r, UINT16 x,UINT16 y);
void gfx_int_AdjustRegionRectangles(struct Region *rgn, UINT16 x, UINT16 y);
BOOL gfx_int_RectXRect(struct Rectangle *r1, struct Rectangle *r2);
void gfx_int_AdjustRegion(struct Region *newrgn, struct Region *rgn ,struct Rectangle *rect);
void gfx_int_AppendRR(struct Region *rgn, struct RegionRectangle *r);
void gfx_int_PrependRR(struct Region *rgn, struct RegionRectangle *r);
void gfx_int_RemoveRR(struct Region *rgn, struct RegionRectangle *r);
