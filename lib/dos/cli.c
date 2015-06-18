/**
 * @file cli.c
 *
 * CLI Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

INT32	dos_MaxCli(pDOSBase DOSBase)
{
	return DOSBase->dos_CliNum;
	
#if 0
	INT32			value = 0;
	pCliProcList	tmp;

	ForeachNode(&DOSBase->dos_CliList, tmp)
	{
		if (value <= tmp->cpl_Number) value = tmp->cpl_Number;
	}
	return value;
#endif
/*
		if (tmp->cpl_Number == value) value++;
		else break;
	}
	return value;
*/
}

#include "tasks.h"
SysCall lib_StackSwapRun(APTR sysBase, UINT32 stackSize, APTR entry);

DOSCALL dos_RunCommand(pDOSBase DOSBase, pSegment seg, INT32 stacksize, UINT8* args, INT32 length)
{
//	pTask this	= FindTask(NULL);

	pFileHandle	fh = Input();
	STRPTR		old_buffer;
	INT32		old_pos;
	INT32		old_end;
	DOSIO		rc, rc2;
	STRPTR		buffer;

	buffer = AllocVec(fh->fh_bufsize, MEMF_PUBLIC|MEMF_CLEAR);

	if (!buffer) return DOSCMD_FAIL;

	CopyMem(args, buffer, length);
	old_buffer	= (STRPTR)fh->fh_buffer;
	old_end		= fh->fh_bufend;
	old_pos		= fh->fh_bufidx;
	fh->fh_buffer= (INT8*)buffer;
	fh->fh_bufend= length;
	fh->fh_bufidx= 0;
	rc = lib_StackSwapRun(SysBase, stacksize, seg->seg_Entry);	
//	rc = seg->seg_Entry(SysBase);

	rc2 = IoErr();

	fh->fh_buffer= 	(INT8*)old_buffer;
	fh->fh_bufend=	old_end;
	fh->fh_bufidx=	old_pos;
	FreeVec(buffer);
	SetIoErr(rc2);
	return rc;
}

