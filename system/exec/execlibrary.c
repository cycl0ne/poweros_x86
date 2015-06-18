/**
 * @file execlibrary.c
 *
 * Stub Calls for the Library Header
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

SysBase *g_SysBase; // this is our savepoint for SysBase

APTR lib_OpenLib(struct SysBase *SysBase)
{
	if (SysBase == NULL)
	{
		//g_SysBase = 
		//Initcode here :)
		
		
	}
	SysBase->LibNode.lib_OpenCnt++;
	return SysBase;
}

APTR lib_CloseLib(struct SysBase *SysBase)
{
	SysBase->LibNode.lib_OpenCnt--;
	return NULL;
}

APTR lib_ExpungeLib(struct SysBase *SysBase)
{
	return NULL;
}

APTR lib_ExtFuncLib(void)
{
	return NULL;
}
