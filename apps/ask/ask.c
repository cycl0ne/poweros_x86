/**
* File: /stack．c
* User: cycl0ne
* Date: 2014-10-31
* Time: 06:29 PM
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

#define	VSTRING	"Ask 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "PROMPT/M/A" CMDREV
#define OPT_PROMPT  0
#define OPT_COUNT   1

#define ASK       "YES/S,NO/S"
#define ASK_YES   0
#define ASK_NO    1
#define ASK_COUNT 2

//DOSCALL cmd_ask(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;
	struct RDargs *qrdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];
	INT32 	query[ASK_COUNT];
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else 
		{
			do
			{
				char **word = (char **)opts[OPT_PROMPT];
				STRPTR msg;
				while ((msg = *word++))
				{
					VPrintf("%s ", (INT32*)&msg);
				}
				Flush(Output());
				
				query[ASK_YES] = query[ASK_NO] = 0;
				qrdargs = ReadArgs(ASK, (UINT32*)query);

			} while(qrdargs == NULL);
			if (query[ASK_YES]) rc = RETURN_WARN;
			rc = RETURN_OK;

			FreeArgs(qrdargs);
			FreeArgs(rdargs);
		}			
		CloseLibrary(DOSBase);
	}
	return rc;
}

