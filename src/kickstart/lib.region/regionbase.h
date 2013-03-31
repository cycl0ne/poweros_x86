#ifndef REGIONBASE_h
#define REGIONBASE_h

#include "types.h"
#include "sysbase.h"
#include "resident.h"

#include "regions.h"
#include "region_funcs.h"

#define AN_REGIONMEMORY (1<<31)

typedef struct RegionBase {
	struct Library	Library;
	SysBase			*SysBase;
} RegionBase;

#endif
