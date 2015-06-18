/**
 * @file makedir.c
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

#define ERROR_NONAME "No name given\n"
#define ERROR_EXISTS "%s already exists\n"
#define ERROR_CANTCR "Can't create directory %s\n"

#define VERSION		0
#define REVISION	1
#define REVSTRING "\0$VER: makedir 0.1 ("__DATE__")\r\n"

#define TEMPLATE    "NAME/M"

#define OPT_NAME  0
#define OPT_COUNT 1


#define OPENERROR	FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;

DOSCALL cmd_makedir(APTR SysBase)
{
	pDOSBase DOSBase;

	INT32 rc = RETURN_ERROR, rc2=0;
	UINT32 opts[OPT_COUNT];
	struct RDargs *rdargs;
	char *name;
	char **argptr;
	pFileLock	lock;
	
	if ( (DOSBase = OpenLibrary("dos.library", 0)) )
	{
		
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL) 
		{
			PrintFault(IoErr(), NULL);
		}
		else
		{
			rc2 = RETURN_OK;

			argptr = (char **)opts[OPT_NAME];
			if(argptr == NULL) 
			{
				rc = RETURN_FAIL;
				VPrintf(ERROR_NONAME, NULL);
			} else 
			{
				rc = RETURN_OK;
				for(name=*argptr; name; name=*(++argptr)) 
				{
					if(lock=Lock(name, SHARED_LOCK)) 
					{
						rc = RETURN_ERROR;
						rc2 = 0;
						VPrintf(ERROR_EXISTS, (INT32 *)&name);
					} else if(!(lock=CreateDir(name))) 
					{
						if(rc != RETURN_ERROR) 
						{
							rc = RETURN_ERROR;
							rc2 = IoErr();
						}
						VPrintf(ERROR_CANTCR, (INT32 *)&name);
					}
					INT32 test;
					if (lock) test = UnLock(lock);
				}
			}
			FreeArgs(rdargs);
			SetIoErr(rc2);
		}
		if(rc2)PrintFault(rc2, NULL);
		CloseLibrary(DOSBase);
	} else 
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

