/**
* File: /startup_shell．c
* User: cycl0ne
* Date: 2014-11-04
* Time: 11:47 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "devices.h"
#include "ports.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "residents.h"

#include "exec_interface.h"
#include "utility_interface.h"
#include "dos_interface.h"

#include "shell.h"
#include "shell_cmds.h"

void Shell(pGD gd);
void startup_shell(APTR SysBase);

#define HANDLER_VERSION_STRING "\0$VER: shell 0.1 ("__DATE__")\r\n";
#define HANDLER_VERSION 0

static const char Name[] = "shell";
static const char Version[] = HANDLER_VERSION_STRING

static APTR EndResident;

static volatile RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	0, // RTF_AUTOINIT | RTF_COLDSTART,
	HANDLER_VERSION,
	NT_HANDLER,
	-120,
	(APTR)Name,
	(APTR)Version,
	(APTR)startup_shell
};

const ICmd cmdTab[] = {
	{"Avail",		cmd_avail},
	{"CD",			cmd_cd},
	{"Echo",		cmd_echo},
	{"EndCLI",		cmd_endcli},
	{"EndShell",	cmd_endcli},
	{"Why",			cmd_why},
	{"MakeDir",		cmd_makedir},
//	{"Set",			cmd_setenv},
//	{"MakeLeanFS",	cmd_makeleanfs},
	{"Dir",			cmd_dir},
//	{"pfs",			cmd_pfs},
//	{"MakeSFS",		cmd_makesfs},
//	{"SanityCheck",	cmd_sanitycheck},
	{"TestConsole",	cmd_testconsole},
//	{"NyanCat",		cmd_nyancat},
	{"RunPrg",		cmd_runprg},
//	{"AddBuffers",	cmd_addbuffers},
//	{"Run", 		cmd_run},
	{"Quit",		cmd_quit},
	{"Debug",	cmd_debug},
	{"Resident",	cmd_resident}

/*
	{"Echo",		cmd_echo},
	{"EndCLI",		cmd_endcli},
	{"EndShell",	cmd_endcli}
*/
};

static void CreateShellCMDs(pGD gd);

void startup_shell(APTR SysBase)
{
	pDosPacket	dp = NULL;
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	pProcess	proc;
	//INT32		res2 = ERROR_ACTION_NOT_KNOWN;
	INT32		res1 = DOSIO_FALSE;
	pGD			gd;
	
	DOSBase = OpenLibrary("dos.library",0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			proc = FindProcess(NULL);
			if (proc)
			{
				gd = AllocVec(sizeof(GD), MEMF_CLEAR|MEMF_PUBLIC);
				if (gd)
				{
					gd->gd_SysBase	= SysBase;
					gd->gd_DOSBase	= DOSBase;
					gd->gd_UtilBase	= UtilBase;
					dp = WaitPkt();
					if (dp->dp_Action == ACTION_STARTUP)
					{
						res1 = CliInit(dp);
						if (res1 == DOSIO_TRUE)
						{
							pShellSM 	sstartup= (pShellSM) dp->dp_Arg2;
							//pTagItem	stags	= (pTagItem) dp->dp_Arg2;
							pComLinInt	cli		= Cli(); //(pComLinInt) dp->dp_Arg3;

							if (sstartup->sm_ShellType == SHELL_BOOT)
							{
								CreateShellCMDs(gd);
								ReplyPkt(dp, 0, 0);
							}
							if (sstartup->sm_ShellType == SHELL_SYSTEM_ASYNC) ReplyPkt(dp, dp->dp_Res1, dp->dp_Res2);

							SetProgramDir(DOSIO_ZERO);
							gd->gd_ShellProc= proc; // FIX!!
							gd->gd_Cli		= cli;
							gd->gd_SStartup = sstartup;
							gd->gd_CliNum	= proc->pr_CLINum;

KPrintF("Starting Shell\n");
							Shell(gd);
KPrintF("Ending Shell\n");
for(;;);
							
							if (cli->cli_StandardInput != NULL) 
							{
								Close(cli->cli_StandardInput);
							}

							if (cli->cli_StandardOutput != NULL)
							{
								Flush(cli->cli_StandardOutput);
								Close(cli->cli_StandardOutput);
							}
							
							if (sstartup->sm_ShellType == SHELL_SYSTEM_SYNC)
							{
								ReplyPkt(dp, cli->cli_ReturnCode, cli->cli_Result2);
							}

							pFileLock cd = CurrentDir(DOSIO_ZERO);
							if (cd != DOSIO_ZERO) UnLock(cd);
							
						} else
						{
							// CliInit failed
							ReplyPkt(dp, dp->dp_Res1, dp->dp_Res2);
						}
						
					}
					FreeVec(gd);
					
				} else
				{
					ReplyPkt(dp, DOSIO_FALSE, ERROR_NO_FREE_STORE);
				}
			}
			CloseLibrary(UtilBase);
		}	
		CloseLibrary(DOSBase);
	} else
		KPrintF("SHELL: Couldnt load DOS.library (There could be now a Msg in Nirvana for us!)\n");
	return;
}

#define SysBase gd->gd_SysBase
#define	DOSBase gd->gd_DOSBase
#define UtilBase gd->gd_UtilBase

static void CreateShellCMDs(pGD gd)
{
	pSegment segment = NULL;
	INT32		ncmd = sizeof(cmdTab) / sizeof(ICmd);
	STRPTR name;
	Forbid();
	while (ncmd)
	{
		ncmd--;
		segment = AllocVec(sizeof(struct Segment), MEMF_CLEAR|MEMF_FAST);
		if (segment)
		{
			//KPrintF("adding %s to seglist %d,%d\n", name, name, segment);
			segment->seg_Node.ln_Name	= (STRPTR)&segment->seg_Name;
			segment->seg_Node.ln_Pri	= CMD_INTERNAL; //(flag<0)? -10:10; //Internal Commands are allways higher prio as other
			segment->seg_Node.ln_Type	= NT_SEGMENT;
			segment->seg_Flags	= CMD_INTERNAL;
			segment->seg_Entry	= (Command)cmdTab[ncmd].function;
			segment->seg_Memory	= NULL;
			segment->seg_Count	= 0;
			InitSemaphore(&segment->seg_Lock);
			AddSegment(cmdTab[ncmd].name, segment, CMD_INTERNAL);
		}
	}
	Permit();
}


/*

New Idea, after about some thinking:

1) WE have the startup message for drives. Why not for Shells?
2) Reconfig the parameters in the start packet to accomplish this behavior
3) We should also reconfig the startupmessage for drives, so that a handler can act apropiate
   (Shell gets a disk structure and rejects it)





------------------
 
Ok what am i planing:

A clean way to open up a shell. We have the following three situations:
a) Bootup
b) Run/Execute/System on an existing shell
c) NewShell/NewCli

For this we implemented 3 Packets (2 new)
ACTION_STARTUP = Bootup
ACTION_EXECUTE = Run/Execute/System
ACTION_NEWSHELL = NewShell/NewCli

Depending the package call to DOS? Or should we just create one entry on dos to simplify CLI?
Option1: 3 Calls:
A_S -> CliInit(dp)
A_R -> CliInitRun(dp)
A_N -> CliInitNewcli(dp)

Option2: 1 Call:
ALL -> CliInit(dp)

dos decides depending on package what to do and calls internally the apropiate functions.

Returncodes from Option1 or Option2:

From understanding, we can have the following things:
SHELL_FAILURE - As name implies -> You need to exit


*/