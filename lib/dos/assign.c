/**
* File: /assign．c
* User: cycl0ne
* Date: 2014-10-28
* Time: 10:19 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "dosbase_private.h"

static pDosEntry dos_AddDE(pDOSBase DOSBase, STRPTR name)
{
	pDosEntry dosEntry = MakeDosEntry(name, DLT_DEVICE );	
	if (dosEntry)
	{
		if (!AddDosEntry(dosEntry))
		{
			FreeDosEntry(dosEntry);
			return NULL;
		}
	}
	return dosEntry;
}


DOSCALL dos_AssignAdd(pDOSBase DOSBase, STRPTR name, pFileLock lock)
{
	DOSCALL		rc = DOSCMD_FAIL;
	pDosEntry	dosEntry;
	pAssign		al;
	
	dosEntry = FindDosEntry(NULL, name, LDF_ASSIGNS);
	if (dosEntry)
	{
		// this is not an assign
		if (dosEntry->de_Type != DLT_DIRECTORY) return rc;
		
		al = AllocVec(sizeof(Assign), MEMF_PUBLIC|MEMF_CLEAR);
		if (!al) return rc;
		AddTail((pList)&dosEntry->de_Misc.assignNode.de_AssignList, (pNode)&al->a_Node);
		al->a_Lock = lock;
		rc = DOSCMD_SUCCESS;
	}
	return rc;
}

DOSCALL dos_AssignRem(pDOSBase DOSBase, STRPTR name, pFileLock lock)
{
	DOSCALL		rc = DOSCMD_FAIL;
	pDosEntry	dosEntry;
	pAssign		al;
	
	dosEntry = FindDosEntry(NULL, name, LDF_ASSIGNS);
	if (dosEntry)
	{
		if (dosEntry->de_Type != DLT_DIRECTORY) return rc;
		if (SameLock(dosEntry->de_Lock, lock) == LOCK_SAME)
		{
			UnLock(dosEntry->de_Lock);
			al = (pAssign) RemHead((pList)&dosEntry->de_Misc.assignNode.de_AssignList);
			if (IsListEmpty((pList)&dosEntry->de_Misc.assignNode.de_AssignList))
			{
				if (RemDosEntry(dosEntry))
				{
					FreeDosEntry(dosEntry);
					rc = DOSCMD_SUCCESS;
				}
			} else
			{
				dosEntry->de_Lock = al->a_Lock;
				FreeVec(al);
				rc = DOSCMD_SUCCESS;
			}
			return rc;
		}
		
		al = GetHead((pList)&dosEntry->de_Misc.assignNode.de_AssignList);
		if (!al) return rc;
		
		if (SameLock(al->a_Lock, lock) == LOCK_SAME)
		{
			Remove((pNode)al);
			UnLock(al->a_Lock);
			FreeVec(al);
			return DOSCMD_SUCCESS;
		} else 
		{
			pAssign temp;
			ForeachNode((pList)&dosEntry->de_Misc.assignNode.de_AssignList, temp)
			{
				if (SameLock(temp->a_Lock, lock) == LOCK_SAME)
				{
					Remove((pNode)temp);
					UnLock(temp->a_Lock);
					FreeVec(temp);
					return DOSCMD_SUCCESS;
				}
			}
			return DOSCMD_FAIL;
		}
	}
	return rc;
}
	
static DOSCALL int_AssignCommon(pDOSBase DOSBase, const STRPTR name, pFileLock lock, INT32 type)
{
	DOSCALL		 rc = DOSCMD_FAIL;
	
	if (Strlen(name) > MAX_DEVICENAME)
	{
		SetIoErr(ERROR_INVALID_COMPONENT_NAME);
		return rc;
	}
	
	pDosEntry dosEntry = FindDosEntry(NULL, name, LDF_ASSIGNS);
	if (dosEntry)
	{
		//Its an assign, undo the previous assign
		if (dosEntry->de_Type != DLT_DIRECTORY) FreeVec(dosEntry->de_Misc.assignNode.de_AssignName);
		FreeDosEntry(dosEntry);
		//
		//Clear AssignList!
	} else {
		if (lock == NULL)
		{
			rc = DOSCMD_SUCCESS;
			return rc;
		}

		dosEntry = MakeDosEntry(name, DLT_DEVICE );
		if (!dosEntry) 
		{
			KPrintF("AssignCommon: MakeDosEntry failed\n");
			return rc;
		}
		if (!AddDosEntry(dosEntry))
		{
			KPrintF("AssignCommon: AddDosEntry failed\n");			
			FreeDosEntry(dosEntry);
			return rc;
		}
	}
	
	// name found, lock = NULL == remove Assign
	if (lock == NULL)
	{
		if (RemDosEntry(dosEntry))
		{
			FreeDosEntry(dosEntry);
			rc = DOSCMD_SUCCESS;
		}
		return rc;
	}

	dosEntry->de_Type = type;
	if (type == DLT_DIRECTORY)
	{
		dosEntry->de_Lock	= lock;
		dosEntry->de_Handler= lock->fl_Handler;
//		KPrintF("Added Assign: %s, %x, %x\n", name, lock, lock->fl_Handler, lock->fl_Handler);
	} else
	{
		dosEntry->de_Lock		= NULL;
		dosEntry->de_Handler	= NULL;
		dosEntry->de_Misc.assignNode.de_AssignLate	= (STRPTR) lock;
	}
	rc = DOSCMD_SUCCESS;
	return rc;
}

DOSCALL dos_AssignLock(pDOSBase DOSBase, const STRPTR name, struct FileLock *lock)
{
//	KPrintF("AssignLock got: %s, %x\n", name, lock);
	return int_AssignCommon(DOSBase, name, lock, DLT_DIRECTORY);
}

DOSCALL dos_AssignLate(pDOSBase DOSBase, const STRPTR name, STRPTR string)
{
	STRPTR str = AllocVec(Strlen(string)+1, MEMF_PUBLIC|MEMF_CLEAR);
	if (!str) return DOSCMD_FAIL;
	Strcpy(str, string);
	return int_AssignCommon(DOSBase, name, (pFileLock) string, DLT_LATE);
}

DOSCALL dos_AssignPath(pDOSBase DOSBase, const STRPTR name, STRPTR string)
{
	STRPTR str = AllocVec(Strlen(string)+1, MEMF_PUBLIC|MEMF_CLEAR);
	if (!str) return DOSCMD_FAIL;
	Strcpy(str, string);
	return int_AssignCommon(DOSBase, name, (pFileLock) string, DLT_NONBINDING);
}




