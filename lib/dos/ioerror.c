/**
 * @file ioerror.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

DOSCALL dos_IoErr(pDOSBase DOSBase)
{
	pProcess	proc = FindProcess(NULL);
	if (proc) return (DOSCALL)proc->pr_Result2;
	return DOSCMD_FAIL;
}

DOSCALL dos_SetIoErr(pDOSBase DOSBase, INT32 code)
{
	pProcess	proc = FindProcess(NULL);
	if (proc) 
	{
		 proc->pr_Result2 = code;
		 return DOSCMD_SUCCESS;
	}
	return DOSCMD_FAIL;
}



