/**
 * @file why.c
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

#define TEMPLATE    "NOPARAMETER"
#define OPT_COUNT   1

DOSCALL cmd_why(APTR SysBase)
{
	pDOSBase DOSBase;
	
	INT32 rc = RETURN_ERROR, faultcode;
	UINT32 opts[OPT_COUNT];
	struct RDargs *rdargs;
	pComLinInt	cli;
	char	buffer[80];
	
	DOSBase = OpenLibrary("dos.library", 0);
	
	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);
	if (rdargs == NULL) 
	{
		PrintFault(IoErr(), NULL);
		Printf("rdargs == NULL\n");
	}
	else
	{
		cli = Cli();
		faultcode = cli->cli_Result2;
		
		if (faultcode) 
		{
			Fault(STR_LAST_MSG_FAILED,NULL,buffer,44);
			PrintFault(faultcode, buffer);
		} else {
			PrintFault(STR_NO_RETURN_CODE,NULL);
		}
		FreeArgs(rdargs);
		SetIoErr(faultcode);
		rc = RETURN_WARN ;
	}
	CloseLibrary(DOSBase);
	return rc;
}

