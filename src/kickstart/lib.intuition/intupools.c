#include "intuitionbase.h"

#include "types.h"
#include "list.h"
#include "exec_funcs.h"

#include "intupools.h"

#define SysBase IBase->ib_SysBase

void InitPool(IntuitionBase *IBase, struct PoolList *pl, INT32 size, UINT32 initNum)
{
	struct PoolNode *pn;
	NewList((struct List *)pl);
	pl->pl_Type		= NT_UNKNOWN;
	pl->pl_NodeSize =  size;
	
	pn = AllocVec(size * initNum, MEMF_FAST|MEMF_CLEAR);
	if (pn == NULL) 
	{
		DPrintF("Cant alloc Pool\n");
		return;
	}
	for (int i = 0; i< initNum; i++)
	{
		pn[i].pn_Delete = FALSE;
		pn[i].pn_FreeList = pl;
		AddTail((struct List*)pl, (struct Node *)&pn[i]);
	}
}

static struct PoolNode *CreatePNode(IntuitionBase *IBase, struct PoolList *pl)
{
	struct PoolNode *pn = AllocVec(pl->pl_NodeSize, MEMF_FAST|MEMF_CLEAR);
	if (pn) 
	{
		pn->pn_Delete	= TRUE;
		pn->pn_FreeList	= pl;
	}
	return pn;
}

struct PoolNode *GetPool(IntuitionBase *IBase, struct PoolList *pl)
{
	struct PoolNode *pn =(struct PoolNode *)RemHead((struct List *)pl);
	if (!pn)
	{
		pn = CreatePNode(IBase, pl);
	}
	return pn;
}

void ReturnPool(IntuitionBase *IBase, struct PoolNode *pn)
{
	if (pn)
	{
		struct PoolList *pl = pn->pn_FreeList;
		if (pn->pn_Delete) FreeVec(pn);
		else AddTail((struct List *)pl, (struct Node *)pn);
	}
}

