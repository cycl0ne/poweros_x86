/**
* File: /shell．c
* User: cycl0ne
* Date: 2014-11-01
* Time: 09:42 PM
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

#define SysBase gd->gd_SysBase
#define	DOSBase gd->gd_DOSBase
#define UtilBase gd->gd_UtilBase

#define DBG 0
#if DBG
#else
#endif

/*
 * 
typedef struct GlobalData
{
	APTR		gd_SysBase;
	pDOSBase	gd_DOSBase;
	pUtilBase	gd_UtilBase;
	pProcess	gd_ShellProc;
	pComLinInt	gd_Cli;
	pShellSM	gd_SStartup;

	UINT32		clinum;
	
	STRPTR		prompt;
	STRPTR		commandname;
	STRPTR		commandfile;
	STRPTR		setname;
	
	pFileHandle	StdOut;

}GD, *pGD;
*/
static void setDecVar(pGD gd, const char *name, INT32 val);
static void updateEnv(pGD gd);
static UINT32 testflags(pGD gd, INT32 flag);
static void _GenPrompt(pGD gd);
static void dbg_Printf(pGD gd, char *str, ...);
static INT32 LoadModule(pGD gd, STRPTR commandname, pSegment *seg, pDosPacket dp);
static INT32 Run(pGD gd, pSegment seg);

#define NAMEMAX 256
//\033[1mBootshell %N>\033[m 

void Shell(pGD gd)
{
	pProcess this	= gd->gd_ShellProc;
	pComLinInt cli	= Cli(); //gd->gd_Cli;
	INT32 rc		= 0;
	INT32 ch, item, i;
	BOOL error		= FALSE;

	dbg_Printf(gd, "cli_Background %x, cli_CurrentInput %x, cli_StandardInput %x", cli->cli_Background, cli->cli_CurrentInput, cli->cli_StandardInput);
	dbg_Printf(gd, "prompt: %s", cli->cli_Prompt);
	struct LocalVar *current_alias;
	INT32 seg;
	STRPTR str;
	APTR	module;
	BOOL pipe_expand;
	INT8	alias[256];
	gd->gd_ShellProc->pr_HomeDir= NULL;
	gd->gd_StdOut				= cli->cli_StandardOutput;
	
//	gd->runSeg	= FindSegment("Run", NULL, 0);
	setDecVar(gd, "process", gd->gd_CliNum);
	updateEnv(gd);

	STRPTR		prompt		= gd->gd_Prompt = cli->cli_Prompt;
	STRPTR		commandname	= cli->cli_CommandName;
	STRPTR		commandfile = cli->cli_CommandFile;
	STRPTR		setname		= gd->gd_SetName = cli->cli_SetName;


	do
	{
		do
		{
			//setname[0] = '\0';
			//prompt[0] = '\0';
//			commandname[0] = '\0';
			dbg_Printf(gd, "cli_Background %x, cli_CurrentInput %x, cli_StandardInput %x", cli->cli_Background, cli->cli_CurrentInput, cli->cli_StandardInput);
			if (!cli->cli_Background && (cli->cli_CurrentInput == cli->cli_StandardInput))
			{
				dbg_Printf(gd, "interactive");				
				cli->cli_Interactive = TRUE;
			} else
			{
				dbg_Printf(gd, "execute a script");
				cli->cli_Interactive = FALSE;
			}

			SelectInput(cli->cli_CurrentInput);
			SelectOutput(gd->gd_StdOut);

			if (cli->cli_Interactive)
			{
				if(!cli->cli_Background) 
				{
					//FPutC(Output(), '!');
					_GenPrompt(gd);			
				}
			} else
			{
				if (testflags(gd, SIGBREAKF_CTRL_D)) 
				{
					error = TRUE;
					PrintFault(304,"SHELL");
//					Printf("SIGBREAKF_CTRL_D\n");
//					for(;;);
				}
				if (error) 
				{
					dbg_Printf(gd, "Error?!");
					break;
				}
			}
			error = FALSE;
			gd->gd_Echo = FALSE;

			if (GetVar("echo", gd->buffer, 256, LV_VAR|GVF_LOCAL_ONLY) > 0)
			{
				if (!Strnicmp("on", (STRPTR)gd->buffer, 2)) gd->gd_Echo = TRUE;
			}
			
			item = ReadItem(commandname, 256, FALSE);
			ch = 0;
			if (item)
			{
				dbg_Printf(gd, "item: %d", item);
				error = TRUE;
				if (item > 0)
				{
					rc = 0;
					i  = -1;
					
					dbg_Printf(gd, "%s", commandname);

/* FIX THIS!
					// Check for an aliased name
					if (item == ITEM_UNQUOTED)
					{
						dbg_Printf(gd, "Checking Alias");
						i = GetVar(commandname,gd->cbuffer,255,LV_ALIAS);
						if (i >= 0) current_alias = FindVar(commandname, LV_ALIAS);
					}
					if (i < 0) Strcpy((STRPTR)gd->cbuffer, (STRPTR)commandname);
					
					if (item == ITEM_UNQUOTED)
					{
						dbg_Printf(gd, "for()");
						for (str = (STRPTR)gd->cbuffer; *str; str++)
						{
							if (*str == ' ')
							{
								str++;
								Strcpy((STRPTR)alias, str);
								Strcat((STRPTR)alias, " ");
								gd->cpos = Strlen((STRPTR)alias);
								break;
							}
						}
					}
					for (i = 0, str = (STRPTR)gd->cbuffer; i < NAMEMAX && ((item == ITEM_QUOTED) || (*str != ' '));)
					{
						commandname[i++] = *str++;
					}
					commandname[i] = '\0';
*/
					cli->cli_Module = NULL;
					pSegment seg;
					dbg_Printf(gd, "LoadModule commandname=%s", commandname);

					INT32 found = LoadModule(gd, commandname, &seg, NULL);
					// Reset CTRL-C

					testflags(gd, SIGBREAKF_CTRL_C);
					
					if (gd->gd_Echo)
					{
						Printf("%s ", commandname);
						Flush(Output());
					}
					
					if (!found)
					{
						dbg_Printf(gd, "Starting command");
						SetProgramName(commandname);
						rc = Run(gd, seg);
					}
					
					SelectOutput(gd->gd_StdOut);
					if (found == -1)
					{
						rc = RETURN_ERROR;
						if (!gd->gd_Res2 || (gd->gd_Res2 == ERROR_OBJECT_NOT_FOUND))
						{
							PrintFault(STR_UNKNOWN_COMMAND, commandname);						
						} else
						{
							PrintFault(gd->gd_Res2, commandname);
						}
					}

					if ((rc == 0) || (gd->gd_Res2 < 0)) gd->gd_Res2 = 0;
					if ((rc>=0) && (rc < cli->cli_FailLevel)) error = FALSE;

					dbg_Printf(gd, "error: %d, rc: %d, gd->gd_Res2: %d", error, rc, gd->gd_Res2);

					if (error && !(cli->cli_Interactive))
					{
						Printf(GetString(STR_FAILED_RETURNCODE), commandname, rc);
					}

					cli->cli_ReturnCode = rc;						
					cli->cli_Result2	= gd->gd_Res2;
					dbg_Printf(gd, "cli_rc: %d, cli_res2: %d", cli->cli_ReturnCode, cli->cli_Result2);
					updateEnv(gd);
					SelectInput(cli->cli_CurrentInput);

					ch = UnGetC(Input(), -1);
					dbg_Printf(gd, "ch: %x %c", ch, ch);
					if (ch) FGetC(Input());
					//else ch = '\n';
				} else
				{
					PrintFault(STR_ERROR_COMMAND, 0);
					dbg_Printf(gd, "ERROR");
				}
			}
			//Eat all up, if it is a comment or a failed command!
			while (( ch != '\n') && (ch != ENDSTREAMCH))  ch = FGetC(Input());
		} while (ch != ENDSTREAMCH);
		
		dbg_Printf(gd, "leaving ch!=EOF loop");
		if (cli->cli_CurrentInput == cli->cli_StandardInput)
		{
			if (( ch == ENDSTREAMCH) && (!cli->cli_Background))
			{
			}
			dbg_Printf(gd, "Leaving Shell----");
			for(;;);
			return;
		} else if (FALSE) // CHECK SYSTEM CALL TODO
		{
				
		} else
		{
			dbg_Printf(gd, "Reseting to interactive");
			Close(Input());
			cli->cli_CurrentInput	= cli->cli_StandardInput;
			cli->cli_FailLevel		= CLI_DEFAULT_FAIL_LEVEL;
		}
		dbg_Printf(gd, "Start again loop (TRUE)");
	} while (TRUE);	
	dbg_Printf(gd, "Leaving Shell");
	return;
}

static void dbg_Printf(pGD gd, char *str, ...)
{
#if 0
	va_list vl;
	va_start(vl, str);
	
	Printf("[SHELLDBG]");
	VPrintf(str, (INT32*)vl);
	Printf("\n");
	va_end(vl);
#endif
	return;
}

static void _prbuf(INT32 ch, INT8 **str)
{
	*(*str)++ = (char)ch;
}

static int vsprintf(pGD gd, char* str, char *fmt,...)
{
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) str);
	va_end(pvar);
	return Strlen(str);
}

static void setDecVar(pGD gd, const char *name, INT32 val)
{
	char buffer[12];

	vsprintf(gd, buffer, "%d", val);
	//KPrintF("Buffer: %s\n", buffer);
	SetVar((const STRPTR)name, (UINT8*)buffer, -1, LV_VAR|GVF_LOCAL_ONLY);
}

static void updateEnv(pGD gd)
{
	setDecVar(gd, "RC", gd->gd_Cli->cli_ReturnCode);
	setDecVar(gd, "Result2", gd->gd_Cli->cli_Result2);
}

static void _GenPrompt(pGD gd)
{
	STRPTR	alias = (STRPTR)gd->gd_Alias;
	STRPTR	prompt = Cli()->cli_Prompt;//gd->gd_Prompt;
	INT32	j = 0;
	INT32	opts[8];
	
	dbg_Printf(gd, "GenPrompt - Start (%s)", Cli()->cli_Prompt);
	
	Strcpy(alias, prompt);
	for (int i = 0; prompt[i]; i++)
	{
		if (prompt[i] == '%')
		{
			switch (ToUpper(prompt[i+1]))
			{
				case 'S':
					opts[j++] = (INT32)gd->gd_SetName;
					break;
				case 'N':
					opts[j++] = gd->gd_CliNum;
					break;
				case 'R':
					opts[j++] = gd->gd_Cli->cli_ReturnCode;
					alias[i] = 'n';
					break;
				case '%':
					i++;
					break;
				case '\0':
					break;
				default:
					alias[i] = ' ';
					break;
			}
		}
	}
	opts[j] = gd->gd_CliNum;
	//Expand(gd, alias, 256);
	//Ticks(gd, alias, 256);
	VFWritef(Output(), (STRPTR)alias, (INT32 *)&opts);
	Flush(Output());
}

static UINT32 testflags(pGD gd, INT32 flag)
{
	return((SetSignal(0,flag) & flag));
}

static INT32 LoadModuleFromPath(pGD gd, STRPTR commandname, pSegment *seg)
{
	pComLinInt	cli = gd->gd_Cli;
	
	if (FilePart(commandname) == commandname)
	{
		pPathNode	pnode;
		pMinList	plist = &cli->cli_CommandDir;
		pFileLock	odir= CurrentDir(NULL);
		pFileLock	dir	= NULL;

		ForeachNode(plist, pnode)
		{
			CurrentDir(pnode->pl_Lock);
			*seg = LoadSegment(commandname);
			if (*seg)
			{
				if (!gd->gd_ShellProc->pr_HomeDir)
				{
					dir = Lock(commandname, SHARED_LOCK);
					if (dir)
					{
						gd->gd_ShellProc->pr_HomeDir = ParentDir(dir);
						UnLock(dir);
					}
				}
				CurrentDir(odir);
				return 0;
			}
		}
		CurrentDir(odir);
	}
	return -1;
}

static INT32 LoadModule(pGD gd, STRPTR commandname, pSegment *seg, pDosPacket dp)
{
	//pComLinInt cli = gd->gd_Cli;
	pSegment	fseg = NULL;
	
	fseg = FindSegment(commandname, NULL, TRUE);
	if (!fseg)
	{
		dbg_Printf(gd,"Not Internal, now searching disk");
		// Searching Disk
		fseg = LoadSegment(commandname);
		if (!fseg)
		{
			dbg_Printf(gd,"Not in currentdir, now searching path");
			if (LoadModuleFromPath(gd, commandname, seg) == -1)
			{
				dbg_Printf(gd,"Not in path, now searching C:");
				// Now check c:
				pFileLock lock = CurrentDir(Lock("C:", SHARED_LOCK));
				if (lock)
				{
					fseg = LoadSegment(commandname);
					UnLock(CurrentDir(lock));
				}
				if (!fseg)
				{
					if (IoErr()) gd->gd_Res2 = IoErr();
					else gd->gd_Res2 = ERROR_OBJECT_NOT_FOUND;
					return -1;
				}
			}
		}
	}
	if (fseg) *seg = fseg;
	dbg_Printf(gd,"Found Module: %s, %d, %d", fseg->seg_Node.ln_Name, fseg->seg_Count ,fseg->seg_Flags);
	if (dp)
	{
		ReplyPkt(dp, dp->dp_Res1, dp->dp_Res2);
	}
	return 0;
}

static INT32 Error(pGD gd, INT32 error, pFileHandle in, pFileHandle out)
{
	if (in) Close(in);
	if (out) Close(out);
	gd->gd_Res2 = error;
	return -1;
}

#define STATE_QUOTE		-1
#define STATE_COMMENT	0
#define STATE_NORMAL	1

static INT32 Run(pGD gd, pSegment seg)
{
	INT32 rc = 0;
	pComLinInt cli = gd->gd_Cli;
	pFileHandle	stdin = cli->cli_StandardInput;
	pFileHandle stdout = Output();
	pFileHandle redin= NULL, redout= NULL;
	
	UINT8	buffer[128];
	INT32	ch = FGetC(Input());
	INT32	c;
	INT32	pos = 0;
	BOOL	loop = TRUE;
	BOOL	past_begin = FALSE;
	INT32	status = STATE_NORMAL;
	char	lastChar = ' ';
//	INT32	comment = 0;

	while (loop)
	{
		c = ch;
		switch (ch)
		{
			case ';':
				if (status > STATE_COMMENT) status = STATE_COMMENT;
				goto g_default;

			case '"':
				if (lastChar != '*') status = STATE_QUOTE;
				goto g_default;

			case '+':
				if (FGetC(Input()) == '\n') c= '\n';
				else UnGetC(Input(), -1);
				goto g_default;
			
			case '>':
				if ((status != STATE_NORMAL) || (redout != NULL) || (lastChar != ' ')) goto g_default;
				if (past_begin) 
				{
					Printf("past_begin\n");
					goto g_default;
				}
				ch = FGetC(Input()); 
				if (ch != '>') UnGetC(Input(), -1);
				if (ReadItem((STRPTR)buffer, 128, NULL) <= 0)
				{
					ch = ' ';
					goto g_default;
				}
				if (redout == NULL)
				{
					dbg_Printf(gd, "Redirecting to: %s\n",buffer);
					redout = Open(buffer, ACTION_FINDOUTPUT);
					if (!redout) Printf("Error opening file\n");
				}
				ch = ' ';
				break;		
			g_default:
			default:
				if (status == STATE_COMMENT) break;
				if ((ch != ' ') && (ch != '\t')) past_begin = TRUE;

			case '\n':
			case ENDSTREAMCH:
				if (pos > 512) return Error(gd, ERROR_COMMAND_LONG, redin, redout);
				gd->cbuffer[pos++] = c;
				if ((ch == '\n') || (ch == ENDSTREAMCH)) loop = FALSE;
		}
		if (loop) 
		{
			if ((lastChar == '*') && (ch == '*')) lastChar = '\0';
			else lastChar = ch;
			ch = FGetC(Input());
		}
	}
	
	if (pos) gd->cbuffer[pos-1] = '\n';
	gd->cbuffer[pos]			= '\0';
	
	if (gd->gd_Echo)
	{
		Printf("%s", gd->cbuffer);
		Flush(Output());
	}
		
	LockSegment(seg);
	seg->seg_Count++;
	UnLockSegment(seg);

	cli->cli_Module = seg;
	if (redin == NULL) redin = stdin;
	if (redout == NULL) redout = stdout;
	
	SelectInput(redin);
	SelectOutput(redout);
	SetIoErr(0);
	SetSignal(0, SIGBREAKF_CTRL_C);

	dbg_Printf(gd, "Runcommand: %x, %d, %s, %d", seg, cli->cli_DefaultStack, gd->cbuffer, pos);
	rc = RunCommand(seg, cli->cli_DefaultStack, gd->cbuffer, pos);
	dbg_Printf(gd, "Return: %d", rc);

	gd->gd_Res1 = rc;
	gd->gd_Res2	= IoErr();


	if (gd->gd_ShellProc->pr_HomeDir!= NULL)
	{
		UnLock(gd->gd_ShellProc->pr_HomeDir);
		gd->gd_ShellProc->pr_HomeDir = NULL;
	}


	updateEnv(gd);
	SetProgramName("");

	LockSegment(seg);
	seg->seg_Count--;
	dbg_Printf(gd,"SegCount: %d\n", seg->seg_Count);
	UnLockSegment(seg);

	UnloadSegment(seg);
	cli->cli_Module = NULL;	

	// clean up the channels.
	SelectOutput(stdout);
	SelectInput(stdin);
	if (stdout != redout) {Close(redout);}
	if (stdin != redin)	{Close(redin);}
	return rc;
}




