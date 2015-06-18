#ifndef bcache_h
#define bcache_h

#include "types.h"
#include "libraries.h"
#include "exec_interface.h"

typedef struct BCacheBase {
	struct Library	Library;
	APTR			bc_SysBase;
} BCacheBase_t, *pBCacheBase;

typedef void * bcache_t;

#endif
