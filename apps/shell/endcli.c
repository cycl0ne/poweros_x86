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

DOSCALL cmd_endcli(APTR SysBase)
{
	pDOSBase	DOSBase;
	INT32 		rc = RETURN_ERROR;
	UINT32 		opts[OPT_COUNT];
	struct RDargs *rdargs;
	
	DOSBase = OpenLibrary("dos.library", 0);
	
	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);
	if (rdargs == NULL) 
	{
		Printf("Err Endcli\n");
		PrintFault(IoErr(), NULL);
	}else
	{
		Printf("Endcli\n");
		rc = RETURN_OK;
		char	buffer[44];
		pProcess	this = FindProcess(NULL);
		pComLinInt	cli = Cli();
		pFileHandle fh	= cli->cli_StandardInput;
		fh->fh_bufend = fh->fh_bufidx = 0;

		#define _PDCLIB_EOFFLAG     2048u

		fh->fh_status |= _PDCLIB_EOFFLAG;
		Printf("Endcli\n");
		if (cli->cli_CurrentInput != cli->cli_StandardInput) 
		{
			Printf("Endcli Close\n");
			Close(cli->cli_CurrentInput);
			cli->cli_CurrentInput = cli->cli_StandardInput;
		}
		Printf("Endcli-BckGrnd\n");

		cli->cli_Background = DOSIO_TRUE;
		if (cli->cli_Interactive)
			if (Fault(STR_PROCESS_ENDING, NULL, buffer, 44))
			{
				VFWritef(Output(), buffer, &this->pr_CLINum);
				Flush(Output());
			}
		Printf("Endcli->Segment\n");
		pSegment seg = FindSegment("SHELL", NULL, TRUE);
		Printf("Segment: %x\n", seg);
		LockSegment(seg);
		seg->seg_Count--;
		UnLockSegment(seg);
		Printf("Endcli<-Segment\n");
		FreeArgs(rdargs);
	}
	CloseLibrary(DOSBase);
	return rc;
}


