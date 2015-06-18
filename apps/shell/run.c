/**
* File: /run．c
* User: cycl0ne
* Date: 2014-11-01
* Time: 03:12 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "dos_tags.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"Run 0.1 (01.11.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "COMMAND" CMDREV
#define OPT_COM  	0
#define OPT_COUNT   1

DOSCALL cmd_run(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];
//	INT32	len = 0;
//	STRPTR	command;
	struct TagItem runtags[] = {
		{SYS_Input, 0},
		{SYS_Output, 0},
		{SYS_UserShell, TRUE},
		{TAG_DONE,}
	};
	
//	pFileHandle	fh;
	
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
//				command = (STRPTR)opts[OPT_COM];
//				if (command) len = Strlen(command);
//				fh = Input();
//				Input() is empty due to readargs :-PrintFault
//				we should generate a new Input then?? 
//				lets test -> write now system!
				runtags[0].ti_Data = (UINT32)Input();
				runtags[1].ti_Data = (UINT32)Output();
				rc = System(NULL, runtags);
			
				if (rc == -1) rc = RETURN_ERROR;
				
				FreeArgs(rdargs);
			}			
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}


