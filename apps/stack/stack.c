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

#define	VSTRING	"Stack 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "SIZE/N" CMDREV
#define OPT_SIZE    0
#define OPT_COUNT   1
#define MINSTACK	8*1024

//DOSCALL cmd_stack(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32	rc2	= 0;
	INT32 	opts[OPT_COUNT];

	pComLinInt	cli;
	INT32		stackSize = 0;
	INT8		*stackMemTest = NULL;
	UINT8		buffer[50];
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else 
		{
			cli = Cli();
			rc2	= 0;

			if (opts[OPT_SIZE])
			{
				stackSize = *((INT32*)opts[OPT_SIZE]);
				rc2		= STR_STACK_SMALL;
				if (stackSize >= MINSTACK)
				{
					rc2	= STR_STACK_LARGE;
					stackMemTest = AllocVec(stackSize, MEMF_PUBLIC);
					if (stackMemTest)
					{
						FreeVec(stackMemTest);
						rc2 = 0;
						rc = RETURN_OK;
						if (cli) cli->cli_DefaultStack = stackSize;
					}
				}

			} else
			{
				if (cli) stackSize = cli->cli_DefaultStack;
				if (Fault(STR_CURRENT_STACK, NULL, (STRPTR)buffer, sizeof(buffer)))
				{
					rc = RETURN_OK;
					VPrintf((CONST_STRPTR)buffer, &stackSize);
				}
			}
			if (rc) PrintFault(rc2, NULL);
			FreeArgs(rdargs);
			SetIoErr(rc2);
		}			
		CloseLibrary(DOSBase);
	}
	return rc;
}

