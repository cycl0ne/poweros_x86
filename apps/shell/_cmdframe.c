/**
* File: /_cmdframe．c
* User: cycl0ne
* Date: 2014-10-25
* Time: 04:50 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"resident 0.1 (24.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME,FILE,REMOVE/S,ADD/S,REPLACE/S,PURE=FORCE/S,SYSTEM/S" CMDREV
#define OPT_NAME    	0
#define OPT_FILE  	1
#define OPT_REMOVE	2
#define OPT_ADD		3
#define OPT_REPLACE	4
#define OPT_PURE	5
#define OPT_SYSTEM	6
#define OPT_COUNT	7

DOSCALL cmd_resident(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32	res	= 0;
	INT32 	opts[OPT_COUNT];

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else 
			{
				rc = RETURN_OK;				
/*
 * Your console code goes here!
 */				
				
				FreeArgs(rdargs);
			}			
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}
