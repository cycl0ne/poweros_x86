/**
 * @file cli_init.c
 *
 * This is the call, that has to be done by the boot shell! Dont call it again!
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

#define GetSysTime(a)		_LIBCALL(TimerBase, 10, VOID, (APTR, struct TimeVal *), (TimerBase,a))

#define MagicCLI 0x434c4900 // CLI0

static DOSIO int_CliInit_Boot(pDOSBase DOSBase, pDosPacket dp);
static DOSIO int_CliInit_Run(pDOSBase DOSBase, pDosPacket dp);
static DOSIO int_CliInit_NewShell(pDOSBase DOSBase, pDosPacket dp);


struct DateStamp *dos_DateStamp(pDOSBase DOSBase, struct DateStamp *ds)
{
	APTR	TimerBase = DOSBase->dos_TimerIO.tr_node.io_Device;
	struct TimeVal tv;
	GetSysTime(&tv);


	ds->ds_Tick		= tv.tv_secs % 60;
	ds->ds_Minute	= tv.tv_secs % (60*24);
	ds->ds_Days		= tv.tv_secs / (60*60*24);
	return ds;
}

uint64_t dos_TimeStamp(pDOSBase DOSBase)
{
	APTR	TimerBase = DOSBase->dos_TimerIO.tr_node.io_Device;
	struct TimeVal tv;
	GetSysTime(&tv);
	return tv.tv_secs;
}

pDosEntry dos_AddHandler(pDOSBase DOSBase, STRPTR name)
{
	pDosEntry de = NULL;

	de = MakeDosEntry(name, DLT_DEVICE);
	if (de)
	{
		DOSCALL error=0;
		if ((error = AddDosEntry(de)) != DOSCMD_SUCCESS)
		{
			KPrintF("AddDosEntry failed, error: %x\n", error);
			FreeDosEntry(de);
			return NULL;
		}
	} else
		KPrintF("dos_AddHandler failed de=%x\n");
	return de;
}

pDosEntry _MakeDosEntry(pDOSBase DOSBase, STRPTR name, INT32 startup, pSegment seg, INT32 stk, INT32 pri)
{

//	if (seg->seg_Flags == CMD_DISABLED) return NULL;
	pDosEntry de = dos_AddHandler(DOSBase, name);
	//KPrintF("_MDE: de: %x\n", de);

	if (de)
	{
		de->de_Misc.handlerNode.de_HandlerSegment= seg;
		de->de_Misc.handlerNode.de_HandlerStack	= stk;
		de->de_Misc.handlerNode.de_HandlerPrio	= pri;
		de->de_Misc.handlerNode.de_Startup		= (pStartupMsg)startup;
	}
	return de;
}

void _CreateAssign(pDOSBase DOSBase, STRPTR name, STRPTR dir)
{
	if (dir == NULL) dir = name;
//	KPrintF("Getting Lock on %s\n", dir);
	pFileLock	fl = Lock(dir, ACCESS_READ);
//	KPrintF("Got back %x\n", fl);
	if (fl == DOSIO_FALSE) fl = Lock(":", ACCESS_READ);
//	KPrintF("Got back %x\n", fl);
	AssignLock(name, fl);
//	KPrintF("ret=%d ", ret);
}

static char _Prompt[] = "%N> ";
#define CLI_DEFAULT_STACK	8192
#define CLI_DEFAULT_FAIL_LEVEL	10
pMsgPort _LaunchHandler(pDOSBase DOSBase, pDosEntry dosEntry, STRPTR name);

DOSIO dos_CLI_Init(pDOSBase DOSBase, pDosPacket dp)
{
	//if (dp->dp_Arg8 != MagicCLI) return DOSIO_FALSE;
	pDosEntry boot =NULL;
	pProcess	proc = FindProcess(NULL);
	pComLinInt		cli = Cli();
	pCliProcList	cpl = AllocVec(sizeof(CliProcList), MEMF_FAST|MEMF_CLEAR);

	cpl->cpl_Number = DOSBase->dos_CliNum = proc->pr_CLINum = 1;
	DateStamp(&DOSBase->dos_Time);

	NewList((pList)&DOSBase->dos_CliList);
	AddHead((pList)&DOSBase->dos_CliList, (pNode)&cpl->cpl_Node);
	//KPrintF("[CLIINIT]Creating DosEntries\n");

	_MakeDosEntry(DOSBase, "AUX", 0, &DOSBase->dos_Console		, 8192, 10);
	_MakeDosEntry(DOSBase, "CON", 0, &DOSBase->dos_Console		, 8192, 10);
	_MakeDosEntry(DOSBase, "RAW", 0, &DOSBase->dos_Console		, 8192, 10);
	_MakeDosEntry(DOSBase, "RAM", 0, &DOSBase->dos_RAMHandler	, 8192, 10);
	_MakeDosEntry(DOSBase, "SER", 0, &DOSBase->dos_ConTTY		, 8192, 10);

	pStartupMsg msg = AllocVec(sizeof(StartupMsg), MEMF_FAST|MEMF_CLEAR);
	if (msg)
	{
		msg->fssm_Environ	= AllocVec(sizeof(DosEnvec), MEMF_FAST|MEMF_CLEAR);
		if (msg->fssm_Environ)
		{
			pDosEnvec env = msg->fssm_Environ;
			env->de_TableSize	= 17;
			env->de_SizeBlock	= 128;
			env->de_Surfaces	= 1;
			env->de_SectorPerBlock	= 1;
			env->de_BlocksPerTrack = 10;
			env->de_Reserved	= 2;
			env->de_LowCyl		= 0;
			env->de_HighCyl		= 10000;
			env->de_NumBuffers	= 300;
			env->de_BufMemType	= MEMF_FAST;
			env->de_MaxTransfer	= 0xfffe00;
			env->de_BootBlocks	= 2;
//			env->de_PreAlloc	= 5;
			env->de_Mask		= 0x7ffffffe;
			msg->fssm_Unit		= 1;
			msg->fssm_Device	= "pata.device";
			msg->fssm_Flags		= 0;
			boot = _MakeDosEntry(DOSBase, "HD0", (INT32)msg, &DOSBase->dos_FileSystem	, 8192, 10);
		} else
		{
			KPrintF("Failed to create StartupMsg\n");
			FreeVec(msg);
		}
	} else
		KPrintF("Failed to create StartupMsg\n");

	//_MakeDosEntry("CON", 0, &DOSBase->dos_Console	, 8192, 10);
	//_MakeDosEntry("RAW", 0, &DOSBase->dos_Console	, 8192, 10);

	// Startup Filesystem
	// Have none ;-) ATM, should we boot ram atm? I DONT think so.
	// Booting our ext2 Handler
	_LaunchHandler(DOSBase, boot, "HD0");
	proc->pr_FileSystemTask = boot->de_Handler;
	// Create Assign
	//	KPrintF("Assign create\n");

	_CreateAssign(DOSBase, "HANDLERS",NULL);
	_CreateAssign(DOSBase, "FONTS",	NULL);
	_CreateAssign(DOSBase, "DEVS",	NULL);
	_CreateAssign(DOSBase, "LIBS",	NULL);
	_CreateAssign(DOSBase, "S",		NULL);
	_CreateAssign(DOSBase, "C",		NULL);
	_CreateAssign(DOSBase, "SYS", ":");
	cli->cli_FailLevel		= CLI_DEFAULT_FAIL_LEVEL;
	cli->cli_DefaultStack	= CLI_DEFAULT_STACK;
	CopyMem(_Prompt, cli->cli_Prompt, Strlen(_Prompt));

	NewList((pList)&cli->cli_CommandDir);

if (cli != NULL)
{
//intF("Prompt: %s\n", cli->cli_Prompt);
}

	InitResidentCode(RTF_AFTERDOS);

	// AUX at the moment, later CON:
	pFileHandle	out = Open("CON:PowerOS", MODE_READWRITE);
	if (!out)
	{
		KPrintF("AUX failed \n");
		for(;;);
	}
	proc->pr_ConsoleTask = out->fh_Type;
	SelectOutput(out);
	//KPrintF("Open CON for read\n");
	SelectInput(Open("CON:", MODE_OLDFILE));
	cli->cli_StandardInput	= Input();
	if (cli->cli_CurrentInput == NULL) cli->cli_CurrentInput = cli->cli_StandardInput;

	cli->cli_StandardOutput	= Output();
	cli->cli_CurrentOutput	= cli->cli_StandardOutput;

#if 0
	cli->cli_StandardInput	= Input();
	cli->cli_CurrentInput	= Open(":S/startup-sequence", MODE_OLDFILE);

	if (cli->cli_CurrentInput == NULL) cli->cli_CurrentInput = cli->cli_StandardInput;
	cli->cli_StandardOutput	= Output();
	cli->cli_CurrentOutput	= cli->cli_StandardOutput;
#endif
	//KPrintF("Returning INIT_CLI\n");
	CurrentDir(Lock("SYS:", ACCESS_READ));
	return DOSIO_TRUE;
}

static pDosEntry creatmsg(pDOSBase DOSBase)
{
	pDosEntry		boot= NULL;
	pStartupMsg msg = AllocVec(sizeof(StartupMsg), MEMF_FAST|MEMF_CLEAR);
	if (msg)
	{
		msg->fssm_Environ	= AllocVec(sizeof(DosEnvec), MEMF_FAST|MEMF_CLEAR);
		if (msg->fssm_Environ)
		{
			pDosEnvec env = msg->fssm_Environ;
			env->de_TableSize	= 17;
			env->de_SizeBlock	= 128;
			env->de_Surfaces	= 1;
			env->de_SectorPerBlock	= 1;
			env->de_BlocksPerTrack = 10;
			env->de_Reserved	= 2;
			env->de_LowCyl		= 0;
			env->de_HighCyl		= 30000;
			env->de_NumBuffers	= 300;
			env->de_BufMemType	= MEMF_FAST;
			env->de_MaxTransfer	= 0xfffe00;
			env->de_BootBlocks	= 2;
//			env->de_PreAlloc	= 5;
			env->de_Mask		= 0x7ffffffe;
			msg->fssm_Unit		= 0;
			msg->fssm_Device	= "pata.device";
//			msg->fssm_Device	= "virtio_blk.device";
			msg->fssm_Flags		= 0;
			boot = _MakeDosEntry(DOSBase, "HD0", (INT32)msg, &DOSBase->dos_FileSystem	, 8192, 10);
		} else
		{
			KPrintF("Failed to create StartupMsg\n");
			FreeVec(msg);
		}
	} else
		KPrintF("Failed to create StartupMsg\n");
	return boot;
}

DOSIO dos_CliInit(pDOSBase DOSBase, pDosPacket dp)
{
	DOSIO	 ret = DOSIO_FALSE;
	if (dp)
	{
		pShellSM ssm = (pShellSM)dp->dp_Arg2;
		KPrintF("CliInit: Type: %d", ssm->sm_ShellType);
		if (ssm)
		{
			switch(ssm->sm_ShellType)
			{
				case SHELL_BOOT:
				ret = int_CliInit_Boot(DOSBase, dp);
				break;

				case SHELL_RUNEX:
				case SHELL_SYSTEM_SYNC:
				case SHELL_SYSTEM_ASYNC:
				ret = int_CliInit_Run(DOSBase, dp);
				break;

				case SHELL_NEWCLI:
				ret = int_CliInit_NewShell(DOSBase, dp);
				break;

				default:
				break;
			}
		}
	}
	return ret;
}

static DOSIO int_CliInit_Boot(pDOSBase DOSBase, pDosPacket dp)
{
	pDosEntry		boot= NULL;
	pShellSM		ssm	= (pShellSM)dp->dp_Arg2;
	pComLinInt		cli	= Cli(); //(pComLinInt)dp->dp_Arg3;
	pProcess		proc= FindProcess(NULL);
	pCliProcList	cpl = AllocVec(sizeof(CliProcList), MEMF_FAST|MEMF_CLEAR);

	KPrintF("CliInit_Boot\n");

	if (cpl)
	{
		cpl->cpl_Number = DOSBase->dos_CliNum = proc->pr_CLINum = 1;
		cpl->cpl_CLI	= cli;
		DateStamp(&DOSBase->dos_Time);

		NewList((pList)&DOSBase->dos_CliList);
		AddHead((pList)&DOSBase->dos_CliList, (pNode)&cpl->cpl_Node); // Fix, we should enqueue it

		_MakeDosEntry(DOSBase, "AUX", 0, &DOSBase->dos_Console		, 8192, 10);
		_MakeDosEntry(DOSBase, "CON", 0, &DOSBase->dos_Console		, 8192, 10);
		_MakeDosEntry(DOSBase, "RAW", 0, &DOSBase->dos_Console		, 8192, 10);
		_MakeDosEntry(DOSBase, "RAM", 0, &DOSBase->dos_RAMHandler	, 8192, 10);
		_MakeDosEntry(DOSBase, "SER", 0, &DOSBase->dos_ConTTY		, 8192, 10);

		boot = creatmsg(DOSBase);
		_LaunchHandler(DOSBase, boot, "HD0");
		if (boot) ssm->sm_FileSysTask = proc->pr_FileSystemTask = boot->de_Handler;

		_CreateAssign(DOSBase, "HANDLERS",NULL);
		_CreateAssign(DOSBase, "FONTS",	NULL);
		_CreateAssign(DOSBase, "DEVS",	NULL);
		_CreateAssign(DOSBase, "LIBS",	NULL);
		_CreateAssign(DOSBase, "S",		NULL);
		_CreateAssign(DOSBase, "C",		NULL);
		_CreateAssign(DOSBase, "SYS", ":");
		cli->cli_FailLevel		= CLI_DEFAULT_FAIL_LEVEL;
		cli->cli_DefaultStack	= CLI_DEFAULT_STACK;
//		CopyMem(_Prompt, cli->cli_Prompt, Strlen(_Prompt));
		NewList((pList)&cli->cli_CommandDir);

		InitResidentCode(RTF_AFTERDOS);

		cli->cli_Background	= FALSE;
		ssm->sm_Output = Open("CON:PowerOS", MODE_READWRITE);
		if (!ssm->sm_Output) KPrintF("CLIINITBOOT: Error Output\n");
		proc->pr_ConsoleTask	= ssm->sm_Output->fh_Type;
		SelectOutput(ssm->sm_Output);
		cli->cli_StandardOutput = Output();
		cli->cli_CurrentOutput = cli->cli_StandardOutput;

		ssm->sm_Input = Open("CONSOLE:", MODE_OLDFILE);
		if (!ssm->sm_Input) KPrintF("CLIINITBOOT: Error Input\n");
		SelectInput(ssm->sm_Input);
		cli->cli_CurrentInput = cli->cli_StandardInput = ssm->sm_Input;

// Fix when shell is running
		cli->cli_CurrentInput = Open(":S/Startup-Sequence", MODE_OLDFILE);

		if (cli->cli_CurrentInput == NULL) cli->cli_CurrentInput = cli->cli_StandardInput;

//KPrintF("cli_Background %x, cli_CurrentInput %x, cli_StandardInput %x\n", cli->cli_Background, cli->cli_CurrentInput, cli->cli_StandardInput);
		SetPrompt("%N> ");
		//SetPrompt("\033[1mBootshell %N>\033[m ");

		ssm->sm_CurDir = Lock(":", ACCESS_READ);
		CurrentDir(ssm->sm_CurDir);
		INT8 buffer[256];
		if (NameFromLock(ssm->sm_CurDir, (STRPTR)buffer, 255)) SetCurrentDirName((STRPTR)buffer);
		else SetCurrentDirName("ERROR_LINE_TOO_LONG");
		return DOSIO_TRUE;
	}
	return DOSIO_FALSE;
}

static DOSIO int_CliInit_Run(pDOSBase DOSBase, pDosPacket dp)
{
	pShellSM		ssm			= (pShellSM)dp->dp_Arg2;
	pComLinInt		cli			= Cli();
	pComLinInt		old_cli		= (pComLinInt)dp->dp_Arg3;
	pProcess		proc		= FindProcess(NULL);
//	pCliProcList	cpl			= AllocVec(sizeof(CliProcList), MEMF_FAST|MEMF_CLEAR);
	BOOL			interactive	= FALSE;
	pFileHandle		cmdstream	= ssm->sm_Command;
	BOOL			is_run		= ssm->sm_Run;
	pFileHandle		output		= NULL;


	cli->cli_CurrentInput	= cmdstream;
	cli->cli_StandardInput	= cmdstream;

	if (!is_run)
	{
		pFileHandle	input = ssm->sm_Input;
		if (input)
		{
			cli->cli_StandardInput	= input;
			interactive				= input->fh_Interactive;
			//Fix: Tell shell to not close STDIN
		} else
		{
			input = Open("NIL:", MODE_OLDFILE);
			if (input) cli->cli_StandardInput = input;
		}
		output = ssm->sm_Output;
	}

	if (!output)
	{
		//Fix: Tell shell to close Output

		if (is_run)
		{
			if (old_cli->cli_StandardOutput == ssm->sm_Output)
			{
				output = Open("CONSOLE:", MODE_READWRITE);
			} else if (ssm->sm_Output->fh_Interactive)
			{
				proc->pr_ConsoleTask = ssm->sm_Output->fh_Type;
				output = Open("CONSOLE:", MODE_READWRITE);
			} else if (ssm->sm_Output->fh_Type)
			{
				output = Open("CONSOLE:", MODE_READWRITE);
			} else
			{
				proc->pr_ConsoleTask = NULL;
			}
		} else
		{
			output = Open("CONSOLE:", MODE_READWRITE);
		}
	}

	if (output == NULL) output = Open("NIL:", MODE_READWRITE);
	if (output == NULL)
	{
		SetIoErr(ERROR_NO_FREE_STORE);
		return DOSIO_FALSE;
	}
	cli->cli_Background = ((interactive && output->fh_Interactive) ? FALSE : TRUE);

	if (ssm->sm_ShellType == SHELL_RUNEX)
	{
		cli->cli_Background = FALSE;
	} else if (ssm->sm_ShellType == SHELL_SYSTEM_ASYNC)
	{
		cli->cli_Background = TRUE;
	}
	CurrentDir(ssm->sm_CurDir);

	//runinitclip()

	SetIoErr(0);
	return DOSIO_TRUE;
}

static DOSIO int_CliInit_NewShell(pDOSBase DOSBase, pDosPacket dp)
{
	pShellSM		ssm			= (pShellSM)dp->dp_Arg2;
	pComLinInt		cli			= Cli();
	pComLinInt		old_cli		= (pComLinInt)dp->dp_Arg3;
	pProcess		proc		= FindProcess(NULL);
	pFileHandle		input		= NULL;
	STRPTR			prompt		= old_cli->cli_Prompt;
	STRPTR			filename	= (STRPTR)dp->dp_Arg1;
	INT8			temp[256];

	CurrentDir(ssm->sm_CurDir);
	proc->pr_FileSystemTask	= ssm->sm_FileSysTask;
	proc->pr_WindowPtr = NULL;

	// IsListEmpty!
	//if (!cli->cli_CommandDir) cli->cli_CommandDir = ssm->sm_CurDir;
	//FreePath?

	input = Open(filename, MODE_OLDFILE);
	if (input && !(input->fh_Interactive))
	{
		Close(input);
		SetIoErr(ERROR_BAD_STREAM_NAME);
		input = NULL;
	}

	if (!input)
	{
		CurrentDir(NULL);
		return DOSIO_FALSE;
	}

	cli->cli_StandardInput	= input;
	SelectInput(input);
	cli->cli_CurrentInput	= input;
	proc->pr_ConsoleTask	= input->fh_Type;

	cli->cli_StandardOutput = Open("CONSOLE:", MODE_READWRITE);
	if (!cli->cli_StandardOutput)
	{
		Close(cli->cli_StandardInput);
		CurrentDir(NULL);
		return DOSIO_FALSE;
	}

	SelectOutput(cli->cli_StandardOutput);
	cli->cli_CurrentOutput	= cli->cli_StandardOutput;
	cli->cli_CurrentInput	= cli->cli_StandardInput;

	input = Open("s:shell-startup", MODE_OLDFILE);
	if (input) cli->cli_CurrentInput = input;

	cli->cli_Background = FALSE;
	cli->cli_ReturnCode	= 0;
	cli->cli_Result2	= 0;
	cli->cli_Module		= NULL;
	cli->cli_FailLevel	= old_cli->cli_FailLevel;
	cli->cli_CommandFile= NULL;

	cli->cli_DefaultStack= old_cli->cli_DefaultStack;

    if (NameFromLock(proc->pr_CurrentDir, (STRPTR)temp, sizeof(temp)))
    {
		SetCurrentDirName((STRPTR)temp);
    } else
	{
		CopyMem(old_cli->cli_SetName, cli->cli_SetName, Strlen(old_cli->cli_SetName));
	}

	CopyMem(prompt, cli->cli_Prompt, Strlen(prompt));
	VPrintf(GetString(STR_NEW_CLI_PROCESS), &proc->pr_CLINum);
	SetIoErr(0);
	return DOSIO_TRUE;
}


