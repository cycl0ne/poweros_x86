/**
 * @file cd.c
 *
 * CLI Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define TEMPLATE    "DIR"
#define OPT_DIR		0
#define OPT_COUNT   1

DOSCALL cmd_cd(APTR SysBase)
{
	pDOSBase	DOSBase = OpenLibrary("dos.library", 0);
	UINT32 		opts[OPT_COUNT];
	struct RDargs *rdargs;

	INT32 			rc = RETURN_FAIL, rc2 = 0;
	char			buffer[256];
	pProcess		process = FindProcess(NULL);
	pFileLock		lock;
	FileInfoBlock	fib;
	
	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);
	if (rdargs == NULL) PrintFault(IoErr(), NULL);
	else
	{
		rc = RETURN_OK;

		if (opts[OPT_DIR])
		{
			if ( (lock=Lock((char *)opts[OPT_DIR], SHARED_LOCK)) ) 
			{
				if(!Examine(lock, &fib) || fib.fib_DirEntryType < 0) 
				{
					rc= RETURN_FAIL;
					if (!(rc2 = IoErr()))rc2=ERROR_OBJECT_WRONG_TYPE;
					UnLock(lock);
					goto ERRORSPOT;
				}
			} else 
			{
				rc2=IoErr();
				if((rc2 == ERROR_NO_DISK) || (rc2==ERROR_DEVICE_NOT_MOUNTED)) 
				{
					rc = RETURN_FAIL;
                    goto ERRORSPOT;
				}
			}
			if(rc || !lock) 
			{
				rc = RETURN_FAIL;
				UnLock(lock);
				goto ERRORSPOT;
			}
			if( (lock = CurrentDir(lock)) != NULL) UnLock(lock);
		}
		if(!NameFromLock(process->pr_CurrentDir, buffer, 255))
		{
			if((rc2=IoErr()) == ERROR_LINE_TOO_LONG)
			{
				rc2 = 0;
			} else
			{
				rc = RETURN_ERROR;
				goto ERRORSPOT;
			}
		}

		if(opts[OPT_DIR])
		{
			//KPrintF("SCDN: buffer [%s]", buffer);
			if(!SetCurrentDirName(buffer))
			{
				rc2 = IoErr();
				rc = RETURN_FAIL;
			}
		} else
		{
			opts[0] = (INT32)buffer;
			VPrintf("%s\n", (INT32*)opts);
		}
	ERRORSPOT:
		FreeArgs(rdargs);
		if(rc && rc2) 
		{
				PrintFault(rc2, NULL);
				if(rc2 == ERROR_LINE_TOO_LONG)rc = rc2 = 0;
		}
		SetIoErr(rc2);
	}
	CloseLibrary(DOSBase);
	return rc;
}
