/**
 * @file echo.c
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

#define TEMPLATE    "ECHO/M,NOLINE/S,FIRST/K/N,LEN/K/N,TO/K"
#define OPT_STRING  0
#define OPT_NOLINE  1
#define OPT_FIRST   2
#define OPT_LEN     3
#define OPT_TO	    4
#define OPT_COUNT   5

DOSCALL cmd_echo(APTR SysBase)
{
	pDOSBase DOSBase;
	pUtilBase UtilBase;
	
	INT32 cnt, rc = RETURN_ERROR, first, len;
	char *msg;
	UINT32 opts[OPT_COUNT];
	struct RDargs *rdargs;
	char **argptr;
	pFileHandle fh;
	BOOL loop=0;

	DOSBase = OpenLibrary("dos.library", 0);
	UtilBase = OpenLibrary("utility.library", 0);
	
	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);

	if (rdargs == NULL) {
		PrintFault(IoErr(), NULL);
	} else {
		if (opts[OPT_TO]) 
		{
			KPrintF("Output to File: %-32s\n", opts[OPT_TO]);
			if (!(fh = Open((char *)opts[OPT_TO], MODE_NEWFILE))) goto ERROR;
			KPrintF("Output to File\n");
		} else
		{
			fh = Output();
		}
				
		rc = RETURN_OK;
		if (opts[OPT_STRING]) 
		{
			argptr = (char **) opts[OPT_STRING];
			while ((msg = *argptr++)) 
			{
				cnt = Strlen(msg);
				if (opts[OPT_FIRST]) 
				{
					first = *((INT32*)opts[OPT_FIRST])-1;
					if (first >= cnt) 
					{
						first = cnt-1;
					}
					if (first >= 0)
					{
						msg += first;
						cnt -= first;
					}
				}

				if (opts[OPT_LEN]) 
				{
					len = *((INT32*)opts[OPT_LEN]);
					if (len < cnt) 
					{
						if (*((INT32*)opts[OPT_FIRST]) == 0) msg += (cnt-len);
						cnt = len;
					}
				}
				if(loop)FPutC(fh,' ');
//				KPrintF("string [msg:%x/cnt:%d] %s\n", msg, cnt, msg);
				FWrite(fh, msg, 1, cnt);
				loop=TRUE;
			}
			if (opts[OPT_NOLINE] != TRUE)FPutC(fh,'\n');
			else Flush(fh);
		}
		if(opts[OPT_TO])
		{
			KPrintF("close\n");
			Close(fh);
			KPrintF("close\n");
		}
ERROR:
		FreeArgs(rdargs);
	}
	CloseLibrary((struct Library *)DOSBase);
	return(rc);
}
