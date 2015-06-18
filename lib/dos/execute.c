/**
* File: /execute．c
* User: cycl0ne
* Date: 2014-11-01
* Time: 06:38 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "dosbase_private.h"

static DOSCALL int_run(pDOSBase DOSBase, STRPTR command, STRPTR shell, INT32 type, pFileHandle input, pFileHandle output, struct TagItem *tags)
{
	pProcess this	= FindProcess(NULL);
	INT32	rc 		= DOSCMD_FAIL;
	INT8	prio	= this->pr_Task.tcb_Node.ln_Pri;
	pSegment seg	= NULL;
	INT32	len		= Strlen(command);
	pFileLock	ncurdir	= NULL;
	pFileHandle finput	= NULL;
	pTagItem 	filterTags = NULL;
	pTagItem	tag;
	DosPacket dp;	// we allocate it on stack (11 * 4bytes + Header ~ 60bytes = OK!

	Tag NotAllowedTags[] = {
		NP_Seglist,
		NP_FreeSeglist,
		NP_Entry,
		NP_Input,
		NP_Output,
		NP_CloseInput,
		NP_CloseOutput,
		NP_CurrentDir,		/* handled via packet */
		NP_HomeDir,		/* shell stuffs this  */
		NP_Cli,
		TAG_END,
	};

	struct TagItem systags[] = {
		{ NP_Seglist,NULL },	/* code depends on this being first */
		{ NP_WindowPtr, 0 },	/* and this being second */
		{ NP_Priority,0 },	/* and this third */
		{ NP_StackSize,800*4 },	/* and this fourth */
		{ NP_Path, NULL },
		{ NP_Name,(INT32) "Background CLI" },
		{ NP_FreeSeglist,FALSE },
		{ NP_Input,NULL },	/* set up by cli_init_run */
		{ NP_Output,NULL },
		{ NP_CloseInput,FALSE },
		{ NP_CloseOutput,FALSE },
/*		{ NP_Error,NULL },
		{ NP_CloseError,FALSE }, */
		{ NP_CurrentDir,NULL },
		{ NP_HomeDir,NULL },	/* shell sets cdir/homedir */
		{ NP_Cli,TRUE },
		{ TAG_MORE,NULL},	/* pass through tags after filter */
		{ TAG_END,NULL},	/* not really needed? */
	};

	if (command)
	{
		finput = AllocVec(sizeof(FileHandle), MEMF_PUBLIC);
		if (finput)
		{
			finput->fh_buffer = AllocVec(len+1, MEMF_PUBLIC|MEMF_CLEAR);
			if (finput->fh_buffer)
			{
				CopyMem(command, finput->fh_buffer, len);
				finput->fh_bufend = len+1;
				finput->fh_bufidx = 0;
			} else
			{
				FreeVec(finput);
				PutStr(GetString(STR_CANT_CREATE_PROC));
				return rc;
			}
		} else
		{
			PutStr(GetString(STR_CANT_CREATE_PROC));
			return rc;
		}
	} else
	{
		finput = input;
	}
	
	seg = FindSegment(shell, NULL, TRUE);
	if (!seg)
	{
		FreeVec(finput->fh_buffer);
		FreeVec(finput);
		PutStr(GetString(STR_CANT_CREATE_PROC));
		return rc;		
	}

	systags[0].ti_Data = (INT32)seg;
	if (type != RUN_SYSTEM_ASYNCH) 				systags[1].ti_Data = (INT32)this->pr_WindowPtr;
	if ((tag = FindTagItem(NP_WindowPtr,tags)))	systags[1].ti_Tag = TAG_IGNORE;

	systags[2].ti_Data = (INT32)prio;
	if ((tag = FindTagItem(NP_Priority,tags)))	systags[2].ti_Tag = TAG_IGNORE;

	if (!(filterTags = CloneTagItems(tags)))
	{
		FreeVec(finput->fh_buffer);
		FreeVec(finput);
		PutStr(GetString(STR_CANT_CREATE_PROC));
		return rc;				
	}
	
	if ((tag = FindTagItem(NP_CurrentDir,tags))) ncurdir = (pFileLock)tag->ti_Data;
	
	systags[sizeof(systags)/sizeof(systags[0]) - 2].ti_Data = (INT32) filterTags;
	FilterTagItems(filterTags, NotAllowedTags, TAGFILTER_NOT);
	
	pProcess newProc = CreateProcess(systags);
	if (!newProc)
	{
		FreeTagItems(filterTags);
		FreeVec(finput->fh_buffer);
		FreeVec(finput);
		PutStr(GetString(STR_CANT_CREATE_PROC));
		return rc;		
	}
	
	INT32	tnum	= newProc->pr_CLINum;
	pMsgPort pPort	= &newProc->pr_MsgPort;
	
	dp.dp_Action	= type;
	dp.dp_Arg1		= (INT32) newProc->pr_CLI;
	dp.dp_Arg2		= (INT32) input;
	dp.dp_Arg3		= (INT32) output;
	dp.dp_Arg4		= (INT32) finput;

	if (ncurdir)
		dp.dp_Arg5	= (INT32) ncurdir;
	else
		dp.dp_Arg5	= (INT32) DupLock(this->pr_CurrentDir);
	
	dp.dp_Arg6		= (INT32) (command != NULL);
	dp.dp_Arg7		= (INT32) filterTags;
	dp.dp_Arg8		= 0;
	dp.dp_Res1		= 0;
	dp.dp_Res2		= 0;
	
	SendPkt(&dp, pPort, &this->pr_MsgPort);
	WaitPkt();
	
	if (!command) Printf("[CLI %d]\n", tnum);

    if (command && (type != RUN_SYSTEM_ASYNCH) && input)
    {
		pFileHandle fh = input;
		if (fh->fh_Type) DoPkt(fh->fh_Type, ACTION_CHANGE_SIGNAL, (INT32)fh->fh_Arg1, (INT32)&this->pr_MsgPort, 0, 0, 0);
    }

    rc	= ((type == RUN_EXECUTE && command) ? 0 : dp.dp_Res1);

    if (dp.dp_Arg4)
    {
		Close((pFileHandle)dp.dp_Arg4);
		rc = -1;
    }

    UnLock((pFileLock)dp.dp_Arg5);
    SetIoErr(dp.dp_Res2);

	FreeTagItems(filterTags);
	FreeVec(finput->fh_buffer);
	FreeVec(finput);
	if (!command && rc) PutStr(GetString(STR_CANT_CREATE_PROC));
	return rc;		
}

DOSCALL dos_Execute(pDOSBase DOSBase, STRPTR command, pFileHandle input, pFileHandle output)
{
	DOSCALL	ret = int_run(DOSBase, command, "shell", RUN_EXECUTE, input, output, NULL);
	if (ret == 0) return DOSCMD_SUCCESS;
	return DOSCMD_FAIL;
}

DOSCALL dos_System(pDOSBase DOSBase, STRPTR command, struct TagItem *tags)
{
	DOSCALL ret = 0;
	pFileHandle input, output;
	BOOL		asynch;
	STRPTR		shell;
	INT32		type = 0;
	
	input	= (pFileHandle) GetTagData(SYS_Input , (UINT32)Input() , tags);
	output	= (pFileHandle) GetTagData(SYS_Output, (UINT32)Output(), tags);
	asynch	= (BOOL) GetTagData(SYS_Asynch, FALSE, tags);
	
	if (GetTagData(SYS_UserShell, FALSE, tags))
	{
		shell = "shell";
	} else
	{
		shell = (STRPTR) GetTagData(SYS_CustomShell, (INT32) "shell", tags);
	}

	if (command)
	{
		if (asynch)
			type = RUN_SYSTEM_ASYNCH;
		else
			type = RUN_SYSTEM;
	} else
		type = RUN_EXECUTE;
//static DOSCALL int_run(pDOSBase DOSBase, STRPTR command, STRPTR shell, INT32 type, pFileHandle input, pFileHandle output, struct TagItem *tags)

	ret = int_run(DOSBase, command, shell, type, input, output, tags);
	return ret;	
}



