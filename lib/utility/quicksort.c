#include "utility.h"
#include "utility_funcs.h"

#define SysBase UtilBase->SysBase

static void qs(pUtility UtilBase, List *a, Node* lb, Node* ub, pNodeCmpLe p_le);
static Node* partition(pUtility UtilBase, List *a, Node* lb, Node* ub, pNodeCmpLe p_le, UINT32* l_count, UINT32* u_count);
static void swap(pUtility UtilBase, List *a, Node** l, Node** u);

void util_QuickSort(pUtility UtilBase, pList a, pNodeCmpLe p_le)
{
	//if no list, return
	if(a == NULL)
		return;

	//if no compare function, return
	if(p_le == NULL)
		return;

	qs(UtilBase, a, (Node*)&(a->lh_Head), (Node*)&(a->lh_Tail), p_le);
}

static void qs(pUtility UtilBase, List *a, Node* lb, Node* ub, pNodeCmpLe p_le)
{
	Node* e;
	UINT32 l_count, u_count;

	while(1)
	{
		//if empty list, return
		if(lb->ln_Succ == ub || ub->ln_Pred == lb)
			return;
		//single item in list, return
		if(lb->ln_Succ == ub->ln_Pred)
			return;

		e = partition(UtilBase, a, lb, ub, p_le, &l_count, &u_count);
		if(l_count < u_count)
		{
			qs(UtilBase, a, lb, e, p_le);
			lb = e;
		}
		else
		{
			qs(UtilBase, a, e, ub, p_le);
			ub = e;
		}
	}
}

static Node* partition(pUtility UtilBase, List* a, Node* lb, Node* ub, pNodeCmpLe p_le, UINT32* l_count, UINT32* u_count)
{
	Node* l;
	Node* u;
	l = lb->ln_Succ;
	*l_count = 0;
	u = ub->ln_Pred;
	*u_count = 0;

	Node* v;
	v = l; //pivot is at lb

	while(!(u == l->ln_Pred || u == l))
	{
		while(TRUE == (*p_le)(l, v) && !(l == ub->ln_Pred))
		{
			l = l->ln_Succ;
			(*l_count)++;
		}
		while(FALSE == (*p_le)(u, v))
		{
			u = u->ln_Pred;
			(*u_count)++;
		}
		if(!(u == l->ln_Pred || u == l))
		{
			swap(UtilBase, a, &l, &u);
		}
	}
	if(u == l)
	{
		//nothing to do
	}
	else //u == l->ln_Pred
	{
		(*l_count)--;
	}

	Node* p;
	p = lb->ln_Succ;
	swap(UtilBase, a, &p, &u); //at this moment, u stops at an element which is <= pivot, so it can be swapped with pivot

	return u;
}

static void swap(pUtility UtilBase, List* a, Node** l, Node** u)
{
	//if same element, return
	if(*l == *u)
		return;

	Node* lb = (*l)->ln_Pred;

	Remove(*l);
	Insert(a, *l, *u);
	Remove(*u);
	Insert(a, *u, lb);

	Node* t;
	t=*l;
	*l=*u;
	*u=t;
}

