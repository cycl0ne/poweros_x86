/**
* File: /addbuffers．c
* User: cycl0ne
* Date: 2014-10-24
* Time: 08:47 AM
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

#define	VSTRING	"addbuffers 0.1 (24.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "DRIVE/A,BUFFERS/N" CMDREV
#define OPT_DRIVE    0
#define OPT_BUFS     1
#define OPT_COUNT    2

DOSCALL cmd_addbuffers(APTR SysBase)
{
	pDOSBase DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;


	INT32 rc = RETURN_FAIL;
	INT32 bufs; 
	STRPTR dev;
	INT32 opts[OPT_COUNT];

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
				dev = (STRPTR)opts[OPT_DRIVE];

				if (*dev && (dev[Strlen(dev)-1] != ':'))
				{
					rc = RETURN_FAIL;
					VPrintf("Invalid device or volume name '%s'\n", opts);
				} else if (opts[OPT_BUFS] && (AddBuffers(dev, *((INT32*)opts[OPT_BUFS])) == 0))
				{
					PrintFault( IoErr(), NULL);
				} else
				{
					bufs = AddBuffers(dev, 0);
					if (bufs <= 0)
					{
						if (bufs == 0) PrintFault( IoErr(), NULL);					
					} else
					{
						Printf("%s has %ld buffers\n", opts[OPT_DRIVE], bufs);
					}
				}
				FreeArgs(rdargs);
			}
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}

