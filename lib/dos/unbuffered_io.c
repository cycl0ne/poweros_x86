/**
 * @file unbuffered_io.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

static DOSIO _DoAction(pDOSBase DOSBase, pFileHandle fh, INT32 *args, INT32 action)
{
	pMsgPort	process;
	INT32		res;
	
	if (!fh)
	{
		SetIoErr(ERROR_INVALID_LOCK);
		return DOSIO_FALSE;
	}
	
	while(1)
	{
		process = fh->fh_Type;
		if (!process) 
		{
//			KPrintF("NIL: %d\n", args[1]);
			// is this NIL: then return on WRITE the amount of bytes, on Read/other = 0
			if (action == 'W') return args[1];
			return DOSIO_TRUE;
		}

		res = DoPkt(process, action, fh->fh_Arg1, args[0], args[1], args[2], args[3]);
//KPrintF("res: %d %x %d/%d\n", res, process, action, 'W');
		if (res != DOSIO_FALSE) return res;
		// we failed, do a errorreport requestor TRUE = Retry from user
#if 0
		if (ErrorReport(IoErr(), REPORT_STREAM, (UINT32)fh, NULL) != TRUE)
		{
			return res;
		}
#endif
	return res;
	}
}

DOSIO dos_Write(pDOSBase DOSBase, pFileHandle fh, const UINT8 *buffer, INT32 len)
{
	INT32 args[4];
	args[0] = (INT32)buffer;
	args[1] = len;
	return _DoAction(DOSBase, fh, args, ACTION_WRITE);
}

DOSIO dos_Read(pDOSBase DOSBase, pFileHandle fh, const UINT8 *buffer, INT32 len)
{
	INT32 args[4];
	args[0] = (INT32)buffer;
	args[1] = len;
	return _DoAction(DOSBase, fh, args, ACTION_READ);
}

DOSIO dos_SeekInternal(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 mode)
{
	INT32 args[4];
	args[0] = pos;
	args[1] = mode;
	return _DoAction(DOSBase, fh, args, ACTION_SEEK);
}

/*
DOSIO dos_NoFlushSeek(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 mode)
{
	INT32 args[4];
	args[0] = pos;
	args[1] = mode;
	return _DoAction(DOSBase, fh, args, ACTION_SEEK);
}
*/

DOSIO dos_SetFileSize(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 mode)
{
	INT32 args[4];
	args[0] = pos;
	args[1] = mode;
	return _DoAction(DOSBase, fh, args, ACTION_SET_FILE_SIZE);
}

pFileLock dos_DupLockFromFH(pDOSBase DOSBase, pFileHandle fh)
{
	INT32 args[4];
	return (pFileLock)_DoAction(DOSBase, fh, args, ACTION_COPY_DIR_FH);
}

pFileLock dos_ParentOfFH(pDOSBase DOSBase, pFileHandle fh)
{
	INT32 args[4];
	return (pFileLock)_DoAction(DOSBase, fh, args, ACTION_PARENT_FH);
}

DOSIO dos_UnLockRecord(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 len)
{
	INT32 args[4];
	args[0] = pos;
	args[1] = len;
	return _DoAction(DOSBase, fh, args, ACTION_FREE_RECORD);
}

DOSIO dos_LockRecord(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 len, INT32 mode, INT32 timeout)
{
	INT32 args[4];
	args[0] = pos;
	args[1] = len;
	args[2] = mode;
	args[3] = timeout;
	return _DoAction(DOSBase, fh, args, ACTION_LOCK_RECORD);
}

DOSIO dos_WaitForChar(pDOSBase DOSBase, pFileHandle fh, INT32 timeout)
{
	if (!fh)
	{
		SetIoErr(ERROR_INVALID_LOCK);
		return DOSIO_FALSE;
	}
	return DoPkt(fh->fh_Type, ACTION_WAIT_CHAR, timeout, 0, 0, 0, 0);
}


