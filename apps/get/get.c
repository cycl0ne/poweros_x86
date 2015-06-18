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

#define	VSTRING	"Get 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME" CMDREV
#define OPT_NAME    0
#define OPT_COUNT   1

//DOSCALL cmd_get(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];

	INT32	mode = GVF_LOCAL_ONLY|LV_VAR;
	INT32	len;
	UINT8 	envbuf[257];
	
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
//				Printf("Found %s\n", opts[OPT_NAME]);
				len = GetVar((STRPTR)opts[OPT_NAME], envbuf, 256, mode);
				if (len == -1)
				{
					PrintFault(IoErr(),NULL);
					rc = RETURN_WARN;
				} else 
				{
					rc = RETURN_OK;
					len = Strlen((STRPTR)envbuf);
					if (len == 256)
					{
						rc = RETURN_WARN;
					}
					if (envbuf[len-1] != '\n') envbuf[len++] = '\n';
					WriteChars((STRPTR)envbuf, len);
				}
				FreeArgs(rdargs);
			}
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}

