/**
 * @file doslist.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"
#include "semaphores.h"

DOSCALL	dos_AddDosEntry(pDOSBase DOSBase, pDosEntry item)
{
	if (item == NULL) 
	{
		SetIoErr(ERROR_REQUIRED_ARG_MISSING);
		return DOSCMD_FAIL;
	}
	
	pDosEntry	next;
	STRPTR		name = item->de_Node.ln_Name;

	LockDosList(LDF_ALL|LDF_WRITE);
	
	ForeachNode(&DOSBase->dos_DosList, next)
	{
		if (Stricmp(next->de_Node.ln_Name, name) == SAME)
		{
			if (next->de_Type != DLT_VOLUME)
			{
				SetIoErr(ERROR_OBJECT_EXISTS);
				UnLockDosList(LDF_ALL|LDF_WRITE);
				return DOSCMD_FAIL;
			}
		}
	}
	
	AddHead((pList)&DOSBase->dos_DosList, &item->de_Node);
	UnLockDosList(LDF_ALL|LDF_WRITE);
	return DOSCMD_SUCCESS;
}

DOSCALL	dos_RemDosEntry(pDOSBase DOSBase, pDosEntry item)
{
	if (item == NULL) 
	{
		SetIoErr(ERROR_REQUIRED_ARG_MISSING);
		return DOSCMD_FAIL;
	}

	LockDosList(LDF_ALL|LDF_WRITE);
	Remove(&item->de_Node);
	UnLockDosList(LDF_ALL|LDF_WRITE);
	return DOSCMD_SUCCESS;
}

DOSCALL	dos_FreeDosEntry(pDOSBase DOSBase, pDosEntry item)
{
	if (item == NULL) 
	{
		SetIoErr(ERROR_REQUIRED_ARG_MISSING);
		return DOSCMD_FAIL;
	}
	FreeVec(item->de_Node.ln_Name);
	FreeVec(item);
	return DOSCMD_SUCCESS;
}

pDosEntry dos_NextDosEntry(pDOSBase DOSBase, pDosEntry item, UINT32 flags)
{
	pDosEntry next = (pDosEntry)item->de_Node.ln_Succ;
	return FindDosEntry(next, NULL, flags);
}

pDosEntry dos_FindDosEntry(pDOSBase DOSBase, pDosEntry item, STRPTR name, UINT32 flags)
{
	static const UINT8 dlt_to_ldf[] = {
		0,				/* DLT_PRIVATE (-1) */
		LDF_DEVICES,	/* DLT_DEVICE       */
		LDF_ASSIGNS,	/* DLT_DIRECTORY    */
		LDF_VOLUMES,	/* DLT_VOLUME       */
		LDF_ASSIGNS,	/* DLT_LATE         */
		LDF_ASSIGNS,	/* DLT_NONBINDING   */
	};

	if (item == NULL) item = (pDosEntry)GetHead(&DOSBase->dos_DosList);
	while(item)
	{
		//KPrintF("FindDosEntry: %s [%x][%x]\n", item->de_Node.ln_Name, item->de_Type, flags);
		if (dlt_to_ldf[item->de_Type + 1] & flags)
		{
			if (!name || (Stricmp(item->de_Node.ln_Name, name) == SAME)) break;
		}
		item = (pDosEntry) item->de_Node.ln_Succ;
	}
	return item;
}

pDosEntry dos_MakeDosEntry(pDOSBase DOSBase, STRPTR name, INT32 type)
{
	if (name != NULL && type < 5 && type >= 0)
	{
		UINT32	len = Strlen(name);
		pDosEntry ret = DOSCMD_NULL;
		STRPTR	dname = AllocVec(len+1, MEMF_FAST);
		if (dname != NULL)
		{
			Strcpy(dname, name);
			ret = (pDosEntry) AllocVec(sizeof(DosEntry), MEMF_FAST|MEMF_CLEAR);
			if (ret != NULL)
			{
				ret->de_Node.ln_Name = dname;
				ret->de_Type = type;
				return ret;
			}
			FreeVec(dname);
		}
		SetIoErr(ERROR_NO_FREE_STORE);
		return DOSCMD_NULL;
	}
	SetIoErr(ERROR_REQUIRED_ARG_MISSING);
	return DOSCMD_NULL;
}

static DOSCALL _Lock(pDOSBase DOSBase, pSignalSemaphore sem, UINT32 flags, BOOL attempt)
{
	if (!attempt) {
		if (flags & LDF_WRITE) ObtainSemaphore(sem);
		else ObtainSemaphoreShared(sem);
	} else
		if (!AttemptSemaphore(sem)) return DOSCMD_FAIL;
	return DOSCMD_SUCCESS;
}

static DOSCALL _LockDL(pDOSBase DOSBase, UINT32 flags, BOOL attempt)
{
	INT32	rw;
	DOSCALL	ret = DOSCMD_FAIL;
	
	rw = flags & (LDF_READ|LDF_WRITE);
	if (!rw || rw == (LDF_READ|LDF_WRITE)) return ret;
	if (flags & ~(LDF_DELETE|LDF_ENTRY|LDF_ALL|LDF_READ|LDF_WRITE)) return ret;
	if (flags & LDF_ALL) 	ret = _Lock(DOSBase, &DOSBase->dos_DosListLock, flags, attempt);
	if (flags & LDF_ENTRY)	ret = _Lock(DOSBase, &DOSBase->dos_EntryLock, flags, attempt);
	if (flags & LDF_DELETE)	ret = _Lock(DOSBase, &DOSBase->dos_DeleteLock, flags, attempt);
	
	return ret;
}

DOSCALL dos_LockDosList(pDOSBase DOSBase, UINT32 flags)
{
	return _LockDL(DOSBase, flags, FALSE);
}

DOSCALL dos_AttemptLockDosList(pDOSBase DOSBase, UINT32 flags)
{
	return _LockDL(DOSBase, flags, TRUE);
}

DOSCALL dos_UnLockDosList(pDOSBase DOSBase, UINT32 flags)
{
	if (flags & LDF_ALL)	ReleaseSemaphore(&DOSBase->dos_DosListLock);
	if (flags & LDF_ENTRY)	ReleaseSemaphore(&DOSBase->dos_EntryLock);
	if (flags & LDF_DELETE)	ReleaseSemaphore(&DOSBase->dos_DeleteLock);
	return DOSCMD_SUCCESS;
}



