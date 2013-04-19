#ifndef intupools_h
#define intupools_h

#include "types.h"
#include "list.h"

struct PoolList {
	struct MinList	pl_List;
	UINT32			pl_NodeSize;
	UINT8			pl_Type;
};

struct PoolNode {
	struct MinNode	pn_Node;
	struct PoolList	*pn_FreeList;
	BOOL			pn_Delete;
};

#endif
