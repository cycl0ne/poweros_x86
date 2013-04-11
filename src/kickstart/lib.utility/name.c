#include "utility.h"
#include "semaphores.h"
#include "utility_funcs.h"

#define SysBase UtilBase->SysBase

struct NameSpace
{
	struct MinList			ns_Entries;
	struct SignalSemaphore	ns_Semaphore;
	UINT32					ns_Flags;
};

struct NamedObj_Node
{
	struct Node	non_Node;
	UINT16		non_UseCount;
};

struct NamedObj_Sub
{
	APTR				nos_Object;
	struct NameSpace	*nos_NameSpace;
	APTR				nos_RemoveMsg;	
};

struct NamedObj
{
	struct NamedObj_Node	no_Non;
	struct NamedObj_Sub		no_Nos;
};


#define _OBJ( o )		((struct NamedObj *)(o))
#define  NOS( _obj )	( (struct NamedObj_Sub *) (_OBJ(_obj)+1) )
#define  NON( o )		( (struct NamedObj_Node *)(_OBJ(o)   -1) )

STRPTR util_NamedObjectName(pUtility UtilBase, struct NamedObject *nos)
{
	if (nos)
	{
		struct NamedObj_Node *tmp = NON(nos);
		return tmp->non_Node.ln_Name;
	}
}

static struct NameSpace *getNameSpace(pUtility UtilBase, struct NamedObject *nameSpace)
{
	struct NameSpace *ns = NOS(UtilBase->MasterSpace)->nos_NameSpace;
	if (nameSpace)
	{
		ns = NOS(nameSpace)->nos_NameSpace;
	}
	return ns;
}

static struct NamedObj *searchns(pUtility UtilBase, struct NamedObj *object, struct NameSpace *ns, STRPTR search)
{
	struct NamedObj *ret;
	if (!search)
	{
		if (object->no_Non.non_Node.ln_Succ) return object;
		return NULL;
	}

	if (ns->ns_Flags & NSB_CASE != 0)
	{
		 ret = (struct NamedObj *)FindName((struct List *)object->no_Non.non_Node.ln_Pred, search);
		 return ret;
	}
	
	ForeachNode(object, ret)
	{
		if (Stricmp(search, ret->no_Non.non_Node.ln_Name) == 0)
		{
			if (ret->no_Non.non_Node.ln_Succ) return ret;
			return NULL;
		}
	}
	return NULL;
}

struct NamedObject *util_FindNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject)
{
	struct NamedObj *tmp;
	struct NamedObject *ret = NULL;
	struct NameSpace *ns = getNameSpace(UtilBase, nameSpace);
	if (ns == NULL) return NULL;

	ObtainSemaphoreShared(&ns->ns_Semaphore);
	if (lastObject == NULL) 
		tmp = (struct NamedObj *)ns->ns_Entries.mlh_Head;
	else
		tmp = (struct NamedObj *)NON(lastObject)->non_Node.ln_Succ;

	tmp = searchns(UtilBase, tmp, ns, name);
	if (tmp)
	{
		tmp->no_Non.non_UseCount++;
		ret = (struct NamedObject *) NOS(tmp);
	}
	ReleaseSemaphore(&ns->ns_Semaphore);
	return ret;
}

BOOL util_AddNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, struct NamedObject *object)
{
	struct NameSpace *ns = getNameSpace(UtilBase, nameSpace);
	if (ns == NULL) return FALSE;
	if (object == NULL) return FALSE;
	
	struct NamedObj *tmp = (struct NamedObj *)NON(object);
	ObtainSemaphore(&ns->ns_Semaphore);
	if ((ns->ns_Flags & NSF_NODUPS)==1)
	{
		if (searchns(UtilBase, (struct NamedObj *)ns->ns_Entries.mlh_Head, ns, tmp->no_Non.non_Node.ln_Name)) 
		{
			ReleaseSemaphore(&ns->ns_Semaphore);
			return FALSE;
		}
	}
	Enqueue((struct List *)&ns->ns_Entries.mlh_Head, &tmp->no_Non.non_Node);
	tmp->no_Nos.nos_NameSpace = ns;
	ReleaseSemaphore(&ns->ns_Semaphore);
	return TRUE;
}

BOOL util_AttemptRemNamedObject(pUtility UtilBase, struct NamedObject *nos)
{
	return RemNamedObject(nos, NULL);
}

BOOL util_RemNamedObject(pUtility UtilBase, struct NamedObject *object, struct Message *message)
{
	Forbid();
	struct NameSpace *ns = NOS(object)->nos_NameSpace;
	if (ns == NULL)
	{
		if (message != NULL) 
		{
			message->mn_Node.ln_Name = NULL;
			ReplyMsg(message);
		}
		Permit();
		return TRUE;
	}
	if (message == NULL) 
	{
		if (NON(object)->non_UseCount != 1)
		{
			Permit();
			return TRUE;
		}
	}

	NOS(object)->nos_NameSpace = NULL;
	ObtainSemaphore(&ns->ns_Semaphore);
	Remove(&NON(object)->non_Node);
	if (message != NULL)
	{
		NOS(object)->nos_RemoveMsg = message;
		message->mn_Node.ln_Name = (STRPTR)object;
	}
	ReleaseSemaphore(&ns->ns_Semaphore);
	Permit();
	return ReleaseNamedObject(object);
}

BOOL util_ReleaseNamedObject(pUtility UtilBase, struct NamedObject *object)
{	
	if (!object) return TRUE;

	Forbid();
	NON(object)->non_UseCount--;
	if (NON(object)->non_UseCount == 0) 
	{ 
		if (NOS(object)->nos_RemoveMsg != NULL) ReplyMsg(NOS(object)->nos_RemoveMsg);
	}
	Permit();
	return TRUE;
}

struct NamedObject *util_AllocNamedObjectA(pUtility UtilBase, STRPTR name, struct TagItem *tagList)
{
	if (name == NULL) return NULL;
	UINT32 ns = GetTagData(ANO_NameSpace, FALSE, tagList);
	
	if (ns == TRUE) ns = sizeof(struct NameSpace);
	
	UINT32 us = GetTagData(ANO_UserSpace, 0, tagList);
	
	if (us != 0) us +=3;
	
	UINT32 nameSize = Strlen(name);
	UINT32 allocSize = ns + us + sizeof(struct NamedObj) + nameSize + 1;

	UINT8 *mem = AllocVec(allocSize, MEMF_PUBLIC|MEMF_CLEAR);
	if (mem == NULL) return NULL;

	struct NamedObj *object = (struct NamedObj *) mem;
	mem += sizeof(struct NamedObj);

	if (ns) {
		object->no_Nos.nos_NameSpace = (struct NameSpace *)mem;
		mem += ns;
	}
	object->no_Non.non_Node.ln_Name = mem;
	Strcpy(mem, name);
	mem += nameSize+1;
	struct NamedObject *ret = (struct NamedObject *)NOS(object);

	if (us)
	{
		UINT32 align = (UINT32)mem;
		align = (align+3) & ~0x03;
		//mem =  ((UINT32)mem+3) & ~0x03; // Align
		NOS(object)->nos_Object = (UINT8*)align;
	}
	
	UINT32 flags = GetTagData(ANO_Flags, 0, tagList);
	struct NameSpace *namSp = NOS(ret)->nos_NameSpace;
	if (namSp != NULL)
	{
		namSp->ns_Flags = flags;
		NewList((struct List *)&namSp->ns_Entries);
		InitSemaphore(&namSp->ns_Semaphore);
	} 
	INT8 prio = GetTagData(ANO_Priority, 0, tagList);
	NON(object)->non_Node.ln_Pri = prio;
	NON(object)->non_UseCount = 1;
	return ret;
}

void util_FreeNamedObject(pUtility UtilBase, struct NamedObject *object)
{
	if (object) FreeVec(NON(object));
}
