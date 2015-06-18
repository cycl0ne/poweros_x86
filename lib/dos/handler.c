/**
 * @file handler.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

pMsgPort _LaunchHandler(pDOSBase DOSBase, pDosEntry dosEntry, STRPTR name)
{
	pMsgPort pID;
	if (dosEntry->de_Handler) return dosEntry->de_Handler; // Already started.
	if (dosEntry->de_Misc.handlerNode.de_HandlerSegment->seg_Flags == CMD_DISABLED) return NULL; // Handler disabled
	//KPrintF("Launch Handler\n");
	pProcess handler = CreateProcessTags(
				NP_Name, (Tag) dosEntry->de_Node.ln_Name,
				NP_Entry, (Tag) dosEntry->de_Misc.handlerNode.de_HandlerSegment->seg_Entry,
				NP_Priority, (Tag)dosEntry->de_Misc.handlerNode.de_HandlerPrio,
				NP_StackSize, (Tag)dosEntry->de_Misc.handlerNode.de_HandlerStack,
				TAG_END
			);

	if (handler)
	{
		pID = &handler->pr_MsgPort;
//		KPrintF("[LH]Process [%s] MsgPort: [%x]\n",handler->pr_Task.tcb_Node.ln_Name, pID);
		INT32 StartupMsg = (INT32)dosEntry->de_Misc.handlerNode.de_Startup;
		// CheckPkt();
//		KPrintF("Started Handler, sending Startuppackage\n");
		DOSIO ret = DoPkt(pID, ACTION_STARTUP, (INT32)name, StartupMsg, (INT32)dosEntry, 0, 0);
//		KPrintF("[dos]LaunchHandler-Return\n");
		if (ret == DOSIO_FALSE || dosEntry->de_Handler == NULL)
		{
			KPrintF("Got ERROR from Handler!! PANIC!!!!!!! \n");
			for(;;);
		}
		return dosEntry->de_Handler;
	} else
		KPrintF("Handler failed to start\n");
	return NULL; // Todo...
}

static pHandlerProc _ObtainHandler(pDOSBase DOSBase, pProcess self, STRPTR name, INT8 *volume)
{
	pHandlerProc dp		= NULL;
	pFileLock lock		= NULL;
	pMsgPort port		= NULL;
	INT32 len			= SplitName(name, ':', (STRPTR)volume, 0, 31);
	UINT32 flags		= 0;

	if (len > 1) // a ":" was found and its not ":foooo"
	{
		pDosEntry dosEntry	= FindDosEntry(NULL, (STRPTR)volume, LDF_ALL);
		if (dosEntry)
		{
			switch(dosEntry->de_Type)
			{
				case DLT_LATE:
					// We remove the Latebinding from our List
					Remove(&dosEntry->de_Node);
					lock = Lock(dosEntry->de_Misc.assignNode.de_AssignName, ACCESS_READ);
					if (!lock)
					{
						AddHead((pList)&DOSBase->dos_DosList, &dosEntry->de_Node);
						port = NULL;
					}
					FreeVec(dosEntry->de_Misc.assignNode.de_AssignName);
					dosEntry->de_Type	= DLT_DIRECTORY;
					dosEntry->de_Lock	= lock;
					dosEntry->de_Handler= lock->fl_Handler;
					AddHead((pList)&DOSBase->dos_DosList, &dosEntry->de_Node);
					if (lock) 	port = lock->fl_Handler;
					else		port = dosEntry->de_Handler;
					break;
					
				case DLT_NONBINDING:
					lock = Lock(dosEntry->de_Misc.assignNode.de_AssignName, ACCESS_READ);
					if (!lock) return NULL;
					flags |= DVPF_UNLOCK;
					if (lock) 	port = lock->fl_Handler;
					else		port = dosEntry->de_Handler;
					break;
				
				case DLT_DEVICE:
					lock = dosEntry->de_Lock;
					if (lock) 	port= lock->fl_Handler;
					else		port= dosEntry->de_Handler;
					if (port == NULL)
					{
						port	= _LaunchHandler(DOSBase, dosEntry, name);
						lock	= NULL;
					}
					break;
					
				case DLT_DIRECTORY:
				case DLT_VOLUME:
					//KPrintF("Found DLT_DIRECTORY/DLT_VOLUME\n");
					lock = dosEntry->de_Lock;
					if (lock) 	port = lock->fl_Handler;
					else		port = dosEntry->de_Handler;
					break;
				
				default:
					port = NULL;
					break;
			}
			if (port == NULL) 
			{
				SetIoErr(ERROR_DEVICE_NOT_MOUNTED);
				KPrintF("Obtainhandler: ERROR_DEVICE_NOT_MOUNTED\n");
				return NULL;
			}

			dp = AllocVec(sizeof(HandlerProc), MEMF_CLEAR|MEMF_FAST);
			if (dp) 
			{
				dp->hp_Port		= port;
				dp->hp_Lock		= lock;
				dp->hp_Flags	= flags;
				dp->hp_DevNode	= dosEntry;
				return dp;
			} else
			{
				SetIoErr(ERROR_NO_FREE_STORE);
				KPrintF("Obtainhandler: ERROR_NO_FREE_STORE\n");
			}
		} else
		{
			SetIoErr(ERROR_DEVICE_NOT_MOUNTED);
			KPrintF("Obtainhandler: ERROR_DEVICE_NOT_MOUNTED (%s)\n", volume);
			return NULL;
		}
	} else
	{
		// Relative Path
		lock = self->pr_CurrentDir;
		//KPrintF("CurrentDir reported: %x on (%s)\n",lock, name);
		pDosEntry dosEntry;
		if (lock == NULL) 	port = GetFileSystemTask(); // No CurrentDir = Root Filesystem with a ZERO lock
		else {
			dosEntry = lock->fl_Volume;	// 
			port = dosEntry->de_Handler;
			if (len == 1) lock = NULL;				// ":look/here/for/a/file" colon is the first char, so lock = Root
		}
		dp = AllocVec(sizeof(HandlerProc), MEMF_CLEAR|MEMF_FAST);
		if (dp) 
		{
			dp->hp_Port		= port;
			dp->hp_Lock		= lock;
			dp->hp_Flags	= flags;
			dp->hp_DevNode	= dosEntry;
			//KPrintF("Returning for %s: %x, %x, %d, %x\n", name, port, lock, flags, dosEntry);
			return dp;
		} else
			SetIoErr(ERROR_NO_FREE_STORE);
	} 
	return NULL;	
}

DOSCALL dos_ReleaseHandler(pDOSBase DOSBase, pHandlerProc hp)
{
	if (hp)
	{
		if (hp->hp_Flags & DVPF_UNLOCK) UnLock(hp->hp_Lock);
		FreeVec(hp);
		return DOSCMD_SUCCESS;
	}
	SetIoErr(ERROR_BAD_ARGUMENTS);
	return DOSCMD_FAIL;
}

pHandlerProc dos_ObtainHandler(pDOSBase DOSBase, STRPTR name)
{
	INT8	buffer[64];
	pHandlerProc dp		= NULL;
	pProcess this = FindProcess(NULL);
	if (this == NULL) return dp;
	
	if (name)
	{
		if (!Strnicmp(name, "NIL:", 4))
		{
			SetIoErr(0);	// this is a good NULL return!Because this is NIL :)
			return NULL;
		} else if (!Strnicmp(name, "CONSOLE:", 8))
		{
			dp = AllocVec(sizeof(HandlerProc), MEMF_CLEAR|MEMF_FAST);
			if (dp) 
			{
				dp->hp_Port		= GetConsoleTask();
				dp->hp_Lock		= NULL;
				dp->hp_Flags	= 0;
				dp->hp_DevNode	= NULL;
				return dp;
			} else
				SetIoErr(ERROR_NO_FREE_STORE);
			
		} else if (!Strnicmp(name, "PROGDIR:", 8))
		{			
			if (this->pr_HomeDir != NULL)
			{
				dp = AllocVec(sizeof(HandlerProc), MEMF_CLEAR|MEMF_FAST);
				if (dp) 
				{
					dp->hp_Port		= this->pr_HomeDir->fl_Handler; //lock->fl_Handler;
					dp->hp_Lock		= this->pr_HomeDir;
					dp->hp_Flags	= 0;
					dp->hp_DevNode	= NULL;
				} else
					SetIoErr(ERROR_NO_FREE_STORE);
			} else
				dp = _ObtainHandler(DOSBase, this, "SYS:", buffer); // No Homedir given, take SYS: as Homedir
				
			return dp;
		} else	
		{
			//KPrintF("Calling _ObtainHandler\n");
			dp = _ObtainHandler(DOSBase, this, name, buffer);
			//KPrintF("found %x\n",dp);
		}
	} else
		SetIoErr(ERROR_BAD_ARGUMENTS);
	return dp;
}

