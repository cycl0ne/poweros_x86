#ifndef REGIONBASE_h
#define REGIONBASE_h

#include "types.h"
#include "libraries.h"

#include "regions.h"
//#include "region_funcs.h"
//#include "region_funcs.h"

#include "exec_interface.h"
#include "region_interface.h"

#define AN_REGIONMEMORY (1<<31)

typedef struct RegionBase {
	struct Library	Library;
	APTR			SysBase;
} RegionBase, *pRegionBase;

#define REGION_NOT_EMPTY(pReg) pReg->numRects

#define EMPTY_REGION(pReg) { \
    (pReg)->numRects = 0; \
    (pReg)->extents.left = (pReg)->extents.top = 0; \
    (pReg)->extents.right = (pReg)->extents.bottom = 0; \
    (pReg)->type = REGION_NULL; \
 }

#define INRECT(r, x, y) \
      ( ( ((r).right >  x)) && \
        ( ((r).left <= x)) && \
        ( ((r).bottom >  y)) && \
        ( ((r).top <= y)) )

#define EXTENTCHECK(r1, r2) \
	((r1)->right > (r2)->left && \
	 (r1)->left < (r2)->right && \
	 (r1)->bottom > (r2)->top && \
	 (r1)->top < (r2)->bottom)
#endif

static inline void memcpy(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsb" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

APTR Realloc(pRegionBase, APTR, UINT32, UINT32);

