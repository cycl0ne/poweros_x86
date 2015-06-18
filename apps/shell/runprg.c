/**
* File: /runprg．c
* User: cycl0ne
* Date: 2014-10-22
* Time: 09:38 PM
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

#define TEMPLATE    "FILE"
#define OPT_FILE	0
#define OPT_COUNT   1


static void hexdump(pDOSBase DOSBase, unsigned char *buf,int len)
{
	int cnt3,cnt4;
	int cnt=0;
	int cnt2=0;

	do
	{
		Printf("%08X | ",cnt);
		for (cnt3=0;cnt3<16;cnt3++)
		{
			if (cnt<len)
			{
				Printf("%02X ",buf[cnt++]);
			}
			else
				Printf("   ");
		}
		Printf("| ");
		for (cnt4=0;cnt4<cnt3;cnt4++)
		{
			if (cnt2==len)
				break;
			if (buf[cnt2]<0x20)
				Printf(".");
			else
				if (buf[cnt2]>0x7F && buf[cnt2]<0xC0)
					Printf(".");
				else
					Printf("%c", buf[cnt2]);
			cnt2++;
		}
		Printf("\n");
	}
	while (cnt!=len);
}

DOSCALL cmd_runprg(APTR SysBase)
{
	pDOSBase DOSBase;
	UINT32 opts[OPT_COUNT];
	struct RDargs *rdargs;
	INT32 			rc = RETURN_FAIL, rc2 = 0;

	DOSBase = OpenLibrary("dos.library", 0);
	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);

	if (rdargs == NULL) 
	{
		PrintFault(IoErr(), NULL);
	} else 
	{
		rc = RETURN_OK;
		if (opts[OPT_FILE])
		{
			Printf("Loading file: %s \n", (char *)opts[OPT_FILE]);
			pSegment seg = LoadSegment((char *)opts[OPT_FILE]);
			if (seg)
			{
				Printf("- Success\n");
				DOSCALL rc = RunCommand(seg, 8192, (UINT8*)NULL, 0);
				UnloadSegment(seg);
			} else
				Printf("- Error\n");
			
		} else
		{
			Printf("No File given.\n");
			rc = RETURN_FAIL;
		}
		
		FreeArgs(rdargs);
		if (rc && rc2)
		{
			PrintFault(rc2, NULL);
		}
	}
	CloseLibrary(DOSBase);
	return rc;
}


