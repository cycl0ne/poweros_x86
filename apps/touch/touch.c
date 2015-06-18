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

#define	VSTRING	"Touch 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME" CMDREV
#define OPT_NAME    0
#define OPT_COUNT   1

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(), NULL);
		} else
		{
			STRPTR file = (STRPTR)opts[OPT_NAME];
			if (file)
			{
				rc = RETURN_OK;
				pFileHandle fh = Open(file, MODE_NEWFILE);
				Close(fh);
			} else
			{
				Printf("No filename given\n");
			}
		}
		FreeArgs(rdargs);
		CloseLibrary(DOSBase);
	}
	return rc;
}

