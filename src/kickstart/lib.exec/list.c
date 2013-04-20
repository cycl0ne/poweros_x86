/*
*	list.c
*
*	All functions for list manipulation.
*
*/

#include "list.h"
#include "sysbase.h"
#include "assert.h"

#include "exec_funcs.h"

void lib_hexstrings(UINT32);
void lib_Print_uart0(const char *s);

void lib_AddHead(SysBase *SysBase, List *list, Node *node);
void lib_AddTail(SysBase *SysBase, List *list, Node *node);
void lib_Enqueue(SysBase *SysBase, List *list, Node *node);
void lib_Insert(SysBase *SysBase, List *list, Node *node, Node *pred);
void lib_NewList(SysBase *SysBase, List *list);
void lib_NewListType(SysBase *SysBase, List *list, UINT8 type);
void lib_Remove(SysBase *SysBase, Node *node);
Node *lib_RemTail(SysBase *SysBase, List *list);
struct Node *lib_RemHead(SysBase *SysBase, struct List *list);
struct Node *lib_FindName(SysBase *SysBase, struct List *liste, STRPTR name);

void lib_AddHead(SysBase *SysBase, List *list, Node *node)
{
	ASSERT(list);
	ASSERT(node);

   // list o-o newnode o-o node
   node->ln_Succ = list->lh_Head;
   node->ln_Pred = (Node *)&list->lh_Head;
   list->lh_Head->ln_Pred = node;
   list->lh_Head = node;
}

void lib_AddTail(SysBase *SysBase, List *list, Node *node)
{
	ASSERT(list);
	ASSERT(node);

   //  list o-o node o-o node o-o node o-o newnode
   node->ln_Succ = (Node *)&list->lh_Tail;
   node->ln_Pred = list->lh_TailPred;
   list->lh_TailPred->ln_Succ = node;
   list->lh_TailPred          = node;
}

void lib_Enqueue(SysBase *SysBase, List *list, Node *node)
{
	ASSERT(list);
	ASSERT(node);

	Node *next;
	ForeachNode(list, next)
	{
		if (node->ln_Pri > next->ln_Pri) break;
	}
	node->ln_Pred = next->ln_Pred;
	node->ln_Succ = next;
	next->ln_Pred->ln_Succ = node;
	next->ln_Pred = node;
}

void lib_Insert(SysBase *SysBase, List *list, Node *node, Node *pred)
{
	ASSERT(list);
	ASSERT(node);

	if (!pred)
	{
		//AddHead(list,node);
		node->ln_Succ = list->lh_Head;
		node->ln_Pred = (struct Node *)&list->lh_Head;
		list->lh_Head->ln_Pred = node;
		list->lh_Head = node;
		return;
	}
	if (!pred->ln_Succ)
	{
		node->ln_Succ = (struct Node *)&list->lh_Tail;
		node->ln_Pred = list->lh_TailPred;
		list->lh_TailPred->ln_Succ = node;
		list->lh_TailPred = node;
	} else
	{
		node->ln_Succ = pred->ln_Succ;
		node->ln_Pred = pred;
		pred->ln_Succ->ln_Pred = node;
		pred->ln_Succ = node;
	}
}

void lib_NewList(SysBase *SysBase, List *list)
{
	ASSERT(list);

    list->lh_Tail = NULL;
    list->lh_Head = (Node*)&list->lh_Tail;
    list->lh_TailPred = (Node*)&list->lh_Head;
}

void lib_NewListType(SysBase *SysBase, List *list, UINT8 type)
{
	ASSERT(list);

    list->lh_Tail = NULL;
    list->lh_Head = (Node*)&list->lh_Tail;
    list->lh_TailPred = (Node*)&list->lh_Head;
    list->lh_Type = type;
}

void lib_Remove(SysBase *SysBase, Node *node)
{
	ASSERT(node);

   node->ln_Succ->ln_Pred = node->ln_Pred;
   node->ln_Pred->ln_Succ = node->ln_Succ;
}

Node *lib_RemTail(SysBase *SysBase, List *list)
{
	ASSERT(list);

	if (IsListEmpty(list)) return NULL;

	Node *node;
	node = list->lh_TailPred;

	node->ln_Succ->ln_Pred = node->ln_Pred;
	node->ln_Pred->ln_Succ = node->ln_Succ;

	return node;
}

Node *lib_RemHead(SysBase *SysBase, struct List *list)
{
	ASSERT(list);

	if (IsListEmpty(list)) return NULL;

	Node *node;
	node = list->lh_Head;

	node->ln_Succ->ln_Pred = node->ln_Pred;
	node->ln_Pred->ln_Succ = node->ln_Succ;

    return node;
}

/*
Node *lib_RemTail(SysBase *SysBase, List *list)
{
	Node *node = list->lh_TailPred->ln_Pred;
	if (!node) return NULL;

	node = list->lh_TailPred;
	list->lh_TailPred = node->ln_Pred;
	list->lh_TailPred->ln_Succ = node->ln_Succ;
	return node;
}

struct Node *lib_RemHead(SysBase *SysBase, struct List *list)
{
	//if(IsListEmpty(list)) return NULL;
	struct Node *node;
	node = list->lh_Head->ln_Succ;
	if (node)
	{
		node->ln_Pred = (struct Node *)list;
		node = list->lh_Head;
		list->lh_Head = node->ln_Succ;
	}
   return node;
}
*/

inline static int strcmp(char *s, char *t)
{
	for (; *s == *t;t++,s++) if (*s == '\0') return 0;
	return *s - *t;
}

struct Node *lib_FindName(SysBase *SysBase, struct List *list, STRPTR name)
{
	ASSERT(list);

   struct Node *tmp=NULL;
   ForeachNode(list, tmp)
   {
      if (!strcmp(tmp->ln_Name, name)) return tmp;
   }
   return NULL;
}


