/**
 * @file process.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

INT32 dos_CheckSignal(pDOSBase DOSBase, INT32 mask)
{
    return (SetSignal(0, mask) &mask);
}

pProcess dos_FindProcess(pDOSBase DOSBase, STRPTR name)
{
	pProcess ret = (pProcess)FindTask(name);
	if (ret->pr_Task.tcb_Node.ln_Type != NT_PROCESS)  return NULL;
	//SetIoErr(0);
	return ret;
}

void dos_Exit(pDOSBase DOSBase,INT32 rc)
{
//TODO!!!!
	return;
}

static void _Process(pSysBase sysBase, pDOSBase DOSBase)
{
	pProcess	this = FindProcess(NULL);
	if (this == NULL) return; // Bail out, we are no Process, how did this come?
	
	STRPTR args = this->pr_Arguments;
	DOSCALL	rc = -1;
	//if (args) rc = RunCommand(this->pr_ProcessCode, args, Strlen(args));
	if (this->pr_ProcessCode) rc = this->pr_ProcessCode(sysBase, args, Strlen(args));

//KPrintF("this->pr_ProcessCode: %x",this->pr_ProcessCode);
	
	if ( (rc != 0) || (!IsListEmpty(&this->pr_OpenFiles)) )
	{
		KPrintF("Returned with Errorcode: %d and/or OpenFiles: %d\n", rc, (!IsListEmpty(&this->pr_OpenFiles)));
	}
	if (this->pr_ExitCode) this->pr_ExitCode(sysBase, rc, this->pr_ExitData);
}

static pProcess _CreateProc(pDOSBase DOSBase, UINT32 stackSize, UINT32 strLen)
{
	pProcess	newProc = AllocVec(sizeof(Process)+strLen, MEMF_FAST|MEMF_CLEAR);
//	KPrintF("NewProc created (AllocVec)\n");
	
	if (newProc != NULL) 
	{
		newProc->pr_Task.tcb_Stack = AllocVec(stackSize, MEMF_FAST|MEMF_CLEAR);
//		KPrintF("Stack for newProc\n");
		if (newProc->pr_Task.tcb_Stack != NULL)
		{
			newProc->pr_Task.tcb_Node.ln_Type	= NT_PROCESS;
			newProc->pr_Task.tcb_Switch			= NULL;
			newProc->pr_Task.tcb_Launch			= NULL;
			newProc->pr_Task.tcb_StackSize		= stackSize;
			newProc->pr_Task.tcb_SPLower		= (UINT32) newProc->pr_Task.tcb_Stack;
			newProc->pr_Task.tcb_SPUpper		= (UINT32) newProc->pr_Task.tcb_Stack + stackSize;
		
			// Process Specialities and Handcrafted port (Powerexec is strict in this!):
			newProc->pr_Task.tcb_SigWait		= SIGF_DOS;
			newProc->pr_MsgPort.mp_SigTask		= &newProc->pr_Task;
			newProc->pr_MsgPort.mp_SigBit		= SIGB_DOS;
			newProc->pr_MsgPort.mp_Node.ln_Type	= NT_MSGPORT;
			NewListType(&newProc->pr_MsgPort.mp_MsgList, NT_MSGPORT);
			NewListType((List*)&newProc->pr_LocalVars, NT_VARS);
			return newProc;
		}
		FreeVec(newProc);
	} 
	KPrintF("CreateProc Error\n");
	return NULL;	
}
#define MAXPROMPT		128
#define MAXCOMMANDNAME	128
#define MAXCOMMANDFILE	128
#define MAXSETNAME		128
#define DEFAULTPROMPT	"%N> "

static pComLinInt _CreateCLI(pDOSBase DOSBase, pTagItem tags)
{
	pComLinInt cli = AllocVec(sizeof(ComLinInt), MEMF_FAST|MEMF_CLEAR);
//	KPrintF("CreateCLI AllocVec\n");
	if (cli)
	{
		cli->cli_Prompt = AllocVec(MAXPROMPT, MEMF_FAST|MEMF_CLEAR);
//		KPrintF("Create Prompt AllocVec\n");
		if (cli->cli_Prompt)
		{
			cli->cli_CommandName = AllocVec(MAXCOMMANDNAME, MEMF_FAST|MEMF_CLEAR);
//			KPrintF("Create CommandName AllocVec %x\n", cli->cli_CommandName);
			if (cli->cli_CommandName)
			{
				cli->cli_CommandFile = AllocVec(MAXCOMMANDFILE, MEMF_FAST|MEMF_CLEAR);
//				KPrintF("Create CommandFile AllocVec\n");
				if (cli->cli_CommandFile)
				{
					cli->cli_SetName = AllocVec(MAXSETNAME, MEMF_FAST|MEMF_CLEAR);
//					KPrintF("Create SetName AllocVec\n");
					if (cli->cli_SetName)
					{
						cli->cli_FailLevel	= RETURN_ERROR;
						cli->cli_Background = DOSIO_TRUE;
						return cli;
					}
					FreeVec(cli->cli_CommandFile);
				}
				FreeVec(cli->cli_CommandName);
			}
			FreeVec(cli->cli_Prompt);
		}
		FreeVec(cli);
	}
	KPrintF("CreateCLI failed\n");
	return NULL;
}

#if 0
static APTR _CopyPath(pDOSBase DOSBase, APTR path)
{
    struct PathNode {
		struct PathNode*	next;
		pFileLock			lock;
    };

    APTR list = 0;
    APTR *p = &list;

    while (path)
    {
		struct PathNode *q = (struct PathNode *)path;
		struct PathNode *t = AllocVec(sizeof(struct PathNode), MEMF_PUBLIC);
		if (!t)  break;

		t->lock = DupLock(q->lock);
		t->next = 0;
		*p = t;
		p = (APTR)&t->next;
		path = q->next;
    }
    return list;
}
#endif

static pComLinInt _InitCLI(pDOSBase DOSBase, pProcess curr, pTagItem tags)
{
	pComLinInt	retCLI	= _CreateCLI(DOSBase, tags);
	pComLinInt	prCLI;
	if (curr == NULL) 	prCLI = NULL;
	else				prCLI = curr->pr_CLI;

	UINT8		prompt[] = DEFAULTPROMPT;
	UINT8*		tmp;
	TagItem*	tag;
//	KPrintF("Process: [%s] CLI: %x\n", curr->pr_Task.tcb_Node.ln_Name, prCLI);
	if (retCLI)
	{
		// No Parent to inherit.
		if (!prCLI)
		{
//			KPrintF("Creating prompt: [%s]\n", prompt);
			CopyMem(prompt, retCLI->cli_Prompt, Strlen(DEFAULTPROMPT));
			
			tag = FindTagItem(NP_CommandName, tags);
			if (tag) 
			{
				tmp = (UINT8*)tag->ti_Data;
				CopyMem((UINT8*)tmp, retCLI->cli_CommandName, Strlen((STRPTR)tmp));
			}
			tag = FindTagItem(NP_CurrentDir, tags);
			if (tag) 
			{
				NameFromLock((pFileLock)tag->ti_Data, retCLI->cli_SetName, MAXSETNAME);
			}
			tag = FindTagItem(NP_Path, tags);
			if (tag) 
			{
				//TODO: Or should we use single linked list?
				//retCLI->cli_CommandDir = (APTR)tag->ti_Data;
			}
		} else
		{
			// We inherit all from Parent
			CopyMem(prCLI->cli_Prompt, retCLI->cli_Prompt, Strlen(prCLI->cli_Prompt));
			CopyMem(prCLI->cli_CommandName, retCLI->cli_CommandName, Strlen(prCLI->cli_CommandName));
			CopyMem(prCLI->cli_SetName, retCLI->cli_SetName, Strlen(prCLI->cli_SetName));
//			retCLI->cli_CommandDir	= _CopyPath(DOSBase, prCLI->cli_CommandDir);
			retCLI->cli_FailLevel	= prCLI->cli_FailLevel;			
		}
		return retCLI;
	}
	KPrintF("InitCLI failed\n");
	return NULL;
}

pProcess dos_CreateProcessTags(pDOSBase DOSBase, Tag tags,...)
{
	pProcess ret = CreateProcess((pTagItem) &tags);
	return ret;
}

pProcess dos_CreateProcess(pDOSBase DOSBase, pTagItem tags)
{
	pProcess	curr	= FindProcess(NULL);
	STRPTR		args	= NULL;
	STRPTR		name	= (STRPTR) GetTagData(NP_Name, (Tag) "Unnamed Process", tags);
	pProcess	newProc = _CreateProc(DOSBase, (UINT32) GetTagData(NP_StackSize, 0x1000, tags), Strlen(name));
//KPrintF("NewProc created\n");
	if (newProc != NULL) 
	{
		UINT8 *pname = ((UINT8*)newProc) + sizeof(Process);
		//KPrintF("pname: %x  //  %x\n", pname, newProc);
		Strcpy((STRPTR)pname, name);
		pSegment seg = (pSegment) GetTagData(NP_Seglist,(Tag)NULL, tags);
		APTR	 entry = (APTR) GetTagData(NP_Entry, 	(Tag)NULL, tags);
		
		
		if (seg)
		{
			newProc->pr_ProcessCode				= (Process_Function) seg->seg_Entry;
			//KPrintF("Segment Entry: %x\n",seg->seg_Entry);
			//for(;;);
		}
		else
			newProc->pr_ProcessCode				= (Process_Function) entry;
		
		newProc->pr_Task.tcb_Node.ln_Pri	= (INT8) GetTagData(NP_Priority, 0,tags);
		newProc->pr_Task.tcb_Node.ln_Name	= (STRPTR)pname;
		newProc->pr_Arguments = args;
		newProc->pr_Task.tcb_Parent = &curr->pr_Task;

		newProc->pr_CurrentDir = NULL;
		newProc->pr_CLI = _InitCLI(DOSBase, curr, tags);

		if (AddTask(&newProc->pr_Task, (Task_Function)_Process, DOSBase) != SYSERR)
		{
			KPrintF("Added Process Name [%s] Prio [%d]\n", newProc->pr_Task.tcb_Node.ln_Name, newProc->pr_Task.tcb_Node.ln_Pri);
			ReadyTask(&newProc->pr_Task, FALSE);
			return newProc;
		}
		FreeVec(newProc->pr_Task.tcb_Stack);
		FreeVec(newProc);		
	}
	return NULL;
}

