#include "utility.h"
#include "semaphores.h"
#include "utility_funcs.h"

#if 0
static void hexdump(APTR SysBase, unsigned char *buf,int len)
{
	int cnt3,cnt4;
	int cnt=0;
	int cnt2=0;
	UINT8 *temp = buf;
	do
	{
		DPrintF("%08X | ", temp); //cnt);
		for (cnt3=0;cnt3<16;cnt3++)
		{
			if (cnt<len)
			{
				DPrintF("%02X ",buf[cnt++]);
				temp++;
			}
			else
				DPrintF("   ");
		}
		DPrintF("| ");
		for (cnt4=0;cnt4<cnt3;cnt4++)
		{
			if (cnt2==len)
				break;
			if (buf[cnt2]<0x20)
				DPrintF(".");
			else
				if (buf[cnt2]>0x7F && buf[cnt2]<0xC0)
					DPrintF(".");
				else
					DPrintF("%c",buf[cnt2]);
			cnt2++;
		}
		DPrintF("\n");
	}
	while (cnt!=len);
}
#endif

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
	UINT16		non_Pad;
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

#define _OBJ( o )			((struct NamedObj *)(o))
#define BASEOBJECT( _obj )	( (struct NamedObject *) (_OBJ(_obj)+1) )
#define SYSTEM( o )			(_OBJ(o) - 1)

STRPTR util_NamedObjectName(pUtility UtilBase, struct NamedObject *nos)
{
	if (nos)
	{
		struct NamedObj *tmp = SYSTEM(nos);
		return tmp->no_Non.non_Node.ln_Name;
	}
	return NULL;
}

static struct NameSpace *getNameSpace(pUtility UtilBase, struct NamedObject *nameSpace)
{
	struct NameSpace *ns = SYSTEM(UtilBase->RootSpace)->no_Nos.nos_NameSpace;
	if (nameSpace)
	{
		ns = SYSTEM(nameSpace)->no_Nos.nos_NameSpace;
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

	if ((ns->ns_Flags & NSB_CASE) != 0)
	{
		 ret = (struct NamedObj *)FindName((struct List *)object->no_Non.non_Node.ln_Pred, search);
		 return ret;
	}
	
	ForeachNode(object, ret)
	{
		if (Stricmp((const char*)search, (const char*)ret->no_Non.non_Node.ln_Name) == 0)
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
		tmp = (struct NamedObj *)SYSTEM(lastObject)->no_Non.non_Node.ln_Succ;

	tmp = searchns(UtilBase, tmp, ns, name);
	if (tmp)
	{
		tmp->no_Non.non_UseCount++;
		ret = (struct NamedObject *) BASEOBJECT(tmp);
	}
	ReleaseSemaphore(&ns->ns_Semaphore);
	return ret;
}

BOOL util_AddNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, struct NamedObject *object)
{
	struct NameSpace *ns = getNameSpace(UtilBase, nameSpace);
	if (ns == NULL) return FALSE;
	if (object == NULL) return FALSE;
	
	struct NamedObj *tmp = (struct NamedObj *)SYSTEM(object);
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
	struct NameSpace *ns = SYSTEM(object)->no_Nos.nos_NameSpace;
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
		if (SYSTEM(object)->no_Non.non_UseCount != 1)
		{
			Permit();
			return TRUE;
		}
	}

	SYSTEM(object)->no_Nos.nos_NameSpace = NULL;
	ObtainSemaphore(&ns->ns_Semaphore);
	Remove(&SYSTEM(object)->no_Non.non_Node);
	if (message != NULL)
	{
		SYSTEM(object)->no_Nos.nos_RemoveMsg = message;
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
	SYSTEM(object)->no_Non.non_UseCount--;
	if (SYSTEM(object)->no_Non.non_UseCount == 0) 
	{ 
		if (SYSTEM(object)->no_Nos.nos_RemoveMsg != NULL) ReplyMsg(SYSTEM(object)->no_Nos.nos_RemoveMsg);
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
	
	UINT32 nameSize = Strlen((const char*)name);
	UINT32 allocSize = ns + us + sizeof(struct NamedObj) + nameSize + 1;

	UINT8 *mem = AllocVec(allocSize, MEMF_PUBLIC|MEMF_CLEAR);
//	UINT8 *test = mem;

	if (mem == NULL) return NULL;	
	struct NamedObj *object = (struct NamedObj *) mem;
	mem += sizeof(struct NamedObj);

	if (ns) {
		object->no_Nos.nos_NameSpace = (struct NameSpace *)mem;
		mem += ns;
	}
	object->no_Non.non_Node.ln_Name = (STRPTR)mem;
	Strcpy((char *)mem, (const char *)name);
	mem += nameSize+1;
	struct NamedObject *ret = (struct NamedObject *)BASEOBJECT(object); //&object->no_Nos;

	if (us)
	{
		UINT32 align = (UINT32)mem;
		align = (align+3) & ~0x03;
		SYSTEM(ret)->no_Nos.nos_Object = (UINT8*)align;
	}
	
	UINT32 flags = GetTagData(ANO_Flags, 0, tagList);
	struct NameSpace *namSp = SYSTEM(ret)->no_Nos.nos_NameSpace;
	if (namSp != NULL)
	{
		namSp->ns_Flags = flags;
		NewList((struct List *)&namSp->ns_Entries);
		InitSemaphore(&namSp->ns_Semaphore);
	} 
	INT8 prio = GetTagData(ANO_Priority, 0, tagList);
	SYSTEM(ret)->no_Non.non_Node.ln_Pri = prio;
	SYSTEM(ret)->no_Non.non_UseCount = 1;
	return ret;
}

void util_FreeNamedObject(pUtility UtilBase, struct NamedObject *object)
{
	if (object) FreeVec(SYSTEM(object));
}
