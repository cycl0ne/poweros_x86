/**
 * @file io.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

pFileLock dos_CurrentDir(pDOSBase DOSBase, pFileLock lock)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		pFileLock old = me->pr_CurrentDir;
		if (lock) me->pr_CurrentDir = lock;
		//KPrintF("old: %x\n", old);
		return old;
	}
	return NULL;
}

pFileHandle dos_Input(pDOSBase DOSBase)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_CIS;
	}
	return NULL;		
}

pFileHandle dos_SelectInput(pDOSBase DOSBase,struct FileHandle *fh)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		struct FileHandle *old = me->pr_CIS;
		me->pr_CIS = fh;
		return old;
	}
	return NULL;
}

pFileHandle dos_Output(pDOSBase DOSBase)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_COS;
	}
	return NULL;		
}

pFileHandle dos_SelectOutput(pDOSBase DOSBase, struct FileHandle *fh)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		struct FileHandle *old = me->pr_COS;
		me->pr_COS = fh;
		return old;
	}
	return NULL;
}

STRPTR dos_GetArgStr(pDOSBase DOSBase)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_Arguments;
	}
	return NULL;
}

STRPTR dos_SetArgStr(pDOSBase DOSBase, STRPTR string)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		STRPTR oldStr;
		oldStr = me->pr_Arguments;
		me->pr_Arguments = string;
		return oldStr;
	}
	return NULL;
}

struct CommandLineInterface *dos_Cli(pDOSBase DOSBase)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_CLI;
	}
	return NULL;
}

struct FileLock *dos_GetProgramDir(pDOSBase DOSBase, pFileLock lock)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_HomeDir;
	}
	return NULL;
}

struct FileLock *dos_SetProgramDir(pDOSBase DOSBase, struct FileLock *lock)
{
	struct FileLock *oldLock;
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		oldLock = me->pr_HomeDir;
		me->pr_HomeDir = lock;
		return oldLock;
	}
	return NULL;
}

struct MsgPort *dos_GetFileSystemTask(pDOSBase DOSBase)
{
	pProcess me = FindProcess(NULL);
	if (me != NULL)
	{
		return me->pr_FileSystemTask;
	}
	return NULL;
}

struct MsgPort *dos_SetFileSystemTask(pDOSBase DOSBase, struct MsgPort *task)
{
	pProcess pr = FindProcess(NULL);
	if (pr != NULL)
	{
		struct MsgPort *old;
		old = pr->pr_FileSystemTask;
		pr->pr_FileSystemTask = task;
		return old;
	}
	return NULL;
}

DOSIO dos_GetCurrentDirName(pDOSBase DOSBase, STRPTR buffer, INT32 len)
{
	pProcess pr = FindProcess(NULL);
	if (pr != NULL)
	{
		struct CommandLineInterface *cli = pr->pr_CLI;
		struct FileLock *tmp = pr->pr_CurrentDir;
	
		if (cli==NULL) return NameFromLock(tmp, buffer, len);
	
		INT32 end;
		INT32 lenCli = Strlen(cli->cli_SetName);
		SetIoErr(0);
		if (lenCli < len-1)
		{
			end = lenCli;
		} else
		{
			end = len -1;
			SetIoErr(ERROR_LINE_TOO_LONG);	
			return DOSIO_FALSE;
		}
		if (len != 0)
		{
			CopyMem(cli->cli_SetName,buffer, end);
			buffer[end] = '\0';
		}
		return DOSIO_TRUE;
	}
	return DOSIO_FALSE;
}

struct MsgPort *dos_GetConsoleTask(pDOSBase DOSBase)
{
	pProcess pr = FindProcess(NULL);
	if (pr != NULL)
	{
		return pr->pr_ConsoleTask;
	}
	return NULL;
}

struct MsgPort *dos_SetConsoleTask(pDOSBase DOSBase, struct MsgPort *handler)
{
    APTR old;
	pProcess pr = FindProcess(NULL);
	if (pr != NULL)
	{
		old = pr->pr_ConsoleTask;
		pr->pr_ConsoleTask = handler;
		return old;
	}
	return NULL;
}

DOSIO dos_GetPrompt(pDOSBase DOSBase, STRPTR buffer, INT32 len)
{
    struct CommandLineInterface *cli = Cli();

	if (cli==NULL)
	{
		*buffer = '\0';
		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		return DOSIO_FALSE;
	}
	
	INT32 end;
	INT32 lenCli = Strlen(cli->cli_Prompt);

	SetIoErr(0);
	if (lenCli < len-1) end = lenCli;
	else {
		end = len -1;
		SetIoErr(ERROR_LINE_TOO_LONG);	
		return DOSIO_FALSE;
	}
	if (len != 0)
	{
		CopyMem(cli->cli_Prompt, buffer, end);
		buffer[end] = '\0';
	}
	return DOSIO_TRUE;
}

DOSIO dos_SetPrompt(pDOSBase DOSBase, STRPTR name)
{
    struct CommandLineInterface *cli = NULL;
    INT32 namelen = Strlen(name);

    if ((cli = Cli()) == NULL) return DOSIO_FALSE;
    if (namelen > 255) return DOSIO_FALSE;

	Strcpy(cli->cli_Prompt, name);
    return DOSIO_TRUE;
}

DOSIO dos_SetProgramName(pDOSBase DOSBase, STRPTR name)
{
    struct CommandLineInterface *cli = NULL;
    INT32 namelen = Strlen(name);

    if ((cli = Cli()) == NULL) return DOSIO_FALSE;
    if (namelen > 255) return DOSIO_FALSE;

	Strcpy(cli->cli_CommandName, name);
    return DOSIO_TRUE;
}

DOSIO dos_SetCurrentDirName(pDOSBase DOSBase, STRPTR name)
{
    struct CommandLineInterface *cli = NULL;
    INT32 namelen = Strlen(name);

    if ((cli = Cli()) == NULL) 
	{
		KPrintF("------------------------>Error in SCDN\n");
		return DOSIO_FALSE;
	}
    if (namelen > 255) 
	{
		KPrintF("------------------------>Error in SCDN\n");
		return DOSIO_FALSE;
	}

	//CopyMem(name, cli->cli_SetName, namelen+1);
	Strcpy(cli->cli_SetName, name);
    return DOSIO_TRUE;
}

DOSIO dos_NameFromLock(pDOSBase DOSBase, struct FileLock *lock, char *buffer, INT32 buflen)
{
	pFileLock		lock_1, lock_2;
	STRPTR			name, volname;
	FileInfoBlock	fib;
	DOSIO			rc = RETURN_OK;
	INT32			len;
	
	if (buflen == 0) 
	{
		SetIoErr(ERROR_LINE_TOO_LONG);
		return DOSIO_FALSE;
	}
//	KPrintF("Lock: %x\n", lock);
	buffer[0] = '\0';	
	if (lock == ZERO_LOCK) return AddPart(buffer, "SYS:", buflen);
	
	lock_1 = DupLock(lock);
	if (!lock_1) return DOSIO_FALSE;
	
	name = &buffer[buflen-1];
	name[0] = '\0';

//	KPrintF("Duplock: %x\n",lock_1);	
	while ( (lock_2 = ParentDir(lock_1)) )
	{
//		KPrintF("Examine\n");
		if ( (Examine(lock_1, &fib)) )
		{
//	KPrintF("name: %s\n", fib.fib_FileName);
			len = Strlen(fib.fib_FileName);
			if (name -(len+4) < buffer)
			{
				SetIoErr(ERROR_LINE_TOO_LONG);
				UnLock(lock_2);
				goto cleanup;
			}
			if (*name) *--name = '/';
			CopyMem(&fib.fib_FileName, name -len, len);
			name -= len;
		} else
			goto cleanup;
		UnLock(lock_1);
		lock_1 = lock_2;
	}
//	KPrintF("out of while\n");
	if (IoErr()) goto cleanup;
	*--name = ':';
	volname = lock_1->fl_Volume->de_Node.ln_Name;
//	KPrintF("Volname: %s", volname);
	len = Strlen(volname);
	if (name - len < buffer)
	{
		SetIoErr(ERROR_LINE_TOO_LONG);
		goto cleanup;
	}
	CopyMem(volname, buffer, len);
	Strcpy(buffer+len, name);
	rc = DOSIO_TRUE;

cleanup:
	UnLock(lock_1);
	return rc;	
}


/*****
******
*****/

#if 0

INT32 dos_int_MyNameFromLock(pDOSBase DOSBase, struct FileLock *lock, char*buffer, INT32 len)
{
	struct Process *me = (struct Process *)FindTask(0);
	APTR temp = me->pr_WindowPtr;
	INT32 rc;
	me->pr_WindowPtr = (APTR)-1;
	rc = NameFromLock(lock, buffer, len);
	me->pr_WindowPtr = temp;
	return rc;
}

INT32 dos_GetProgramName (pDOSBase DOSBase, UINT8 *buffer, INT32 len)
{
    struct CommandLineInterface *cli = Cli();

	if (cli==NULL)
	{
		*buffer = '\0';
		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		return DOSFALSE;
	}
	
	INT32 end;
	INT32 lenCli = Strlen(cli->cli_CommandName);
	SetIoErr(0);
	if (lenCli < len-1)
	{
		end = lenCli;
	} else
	{
		end = len -1;
		SetIoErr(ERROR_LINE_TOO_LONG);	
	}
	if (len != 0)
	{
		CopyMem(cli->cli_CommandName,buffer, end);
		buffer[end] = '\0';
	}
	return DOSTRUE;
}


INT32 dos_NameFromFH (pDOSBase DOSBase, struct FileHandle *fh, char *buffer, INT32 buflen)
{
	struct FileLock *lock;
	struct FileInfoBlock *fib = NULL;
	INT32 rc = FALSE;
	INT32 res2;

	if ((!(lock = ParentOfFH(fh))) ||
	    (!(fib  = AllocDosObject(DOS_FIB,NULL))) ||
	    (!(ExamineFH(fh,fib))) ||
	    (!(NameFromLock(lock,buffer,buflen))))
		goto cleanup;

	rc = AddPart(buffer,fib->fib_FileName,buflen);

cleanup:
	res2 = IoErr();

	if (fib) FreeDosObject(DOS_FIB,fib);
	if (lock) UnLock(lock);

	SetIoErr(res2);

	return rc;
}

void kill_cli_num (pDOSBase DOSBase, INT32 tnum)
{
	struct CliProcList *cl;

	for (cl = (struct CliProcList *) DOSBase->rn_CliList.mlh_Head;
	     cl && cl->cpl_Node.mln_Succ;
	     cl = (struct CliProcList *) cl->cpl_Node.mln_Succ)
	{
		/* is the entry in this small array? */
		if (tnum <= cl->cpl_First + ((INT32) cl->cpl_Array[0]) - 1)
		{
			cl->cpl_Array[tnum + 1 - cl->cpl_First] = NULL;
			return;
		}
	}
}

UINT32 dos_MaxCli(pDOSBase DOSBase)
{
	struct CliProcList *cli;
	UINT32 max = 0,curr;

		for (cli = (struct CliProcList *) DOSBase->rn_CliList.mlh_Head;
			 cli && cli->cpl_Node.mln_Succ;
			 cli = (struct CliProcList *) cli->cpl_Node.mln_Succ)
		{
			for (curr = 0; curr < (INT32) cli->cpl_Array[0]; curr++)
			{
				if (cli->cpl_Array[curr]) max = cli->cpl_First + curr;
			}
		}
	return max;
}

struct Process *dos_FindCli(pDOSBase DOSBase,  UINT32 num)
{
	struct CliProcList *cli;

	if (num)
	{
		for (cli = (struct CliProcList *) DOSBase->rn_CliList.mlh_Head;
			 cli && cli->cpl_Node.mln_Succ;
			 cli = (struct CliProcList *) cli->cpl_Node.mln_Succ)
		{
			if (num < cli->cpl_First + (INT32) cli->cpl_Array[0])
			{
			if (cli->cpl_Array[num+1 - cli->cpl_First]) 
				return (struct Process *)
			          (((INT32) cli->cpl_Array[num+1-cli->cpl_First]) - sizeof(struct Task));
			else
				return NULL;
			}
		}
	}
	return NULL;
}

#endif



