/**
 * @file ram_handler.c
 *
 * This file describes a standard DOS handler for use with the ram (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "ram_handler.h"

#include "residents.h"

struct NotifyMessage NotifyMessage, *pNotifyMessage;
struct NotifyRequest NotifyRequest, *pNotifyRequest;

#define HANDLER_VERSION_STRING "\0$VER: ram.handler 0.1 ("__DATE__")\r\n";
#define HANDLER_VERSION 0

static const char Name[] = "ram.handler";
static const char Version[] = HANDLER_VERSION_STRING

static APTR EndResident;
static void RAM_Handler(APTR data);

volatile static RESIDENT_TAG RomTag =
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
	(APTR)RAM_Handler
};

static void RAM_Handler(APTR data)
{
	pSysBase		SysBase 	= data;
	pDOSBase		DOSBase		= OpenLibrary("dos.library",0);
	pUtilBase		UtilBase	= OpenLibrary("utility.library",0);
	pMsgPort		HandlerPort;
	pProcess		this = FindProcess(NULL);
	pObjNode 		node;
	//KPrintF("RAM: (%s) this->pr_Task.tcb_SigRecvd %x\n", this->pr_Task.tcb_Node.ln_Name, this->pr_Task.tcb_SigRecvd);

	pGlobalData		gd = AllocVec(sizeof(GlobalData), MEMF_FAST|MEMF_CLEAR);
	if (gd)
	{
		gd->gd_sysBase	= SysBase;
		gd->gd_dosBase	= DOSBase;
		gd->gd_utilBase	= UtilBase;

		gd->gd_MyProc	= FindProcess(NULL);
		gd->gd_MyPort	= HandlerPort = &gd->gd_MyProc->pr_MsgPort;
		pDosPacket	dp	= WaitPkt();

		//KPrintF("RAM: (%s) this->pr_Task.tcb_SigRecvd %x\n", this->pr_Task.tcb_Node.ln_Name, this->pr_Task.tcb_SigRecvd);

		if (dp)
		{
			gd->gd_Root	= AllocVec(sizeof(ObjNode), MEMF_FAST|MEMF_CLEAR);

			if (gd->gd_Root)
			{
				gd->gd_MyReplyPort	= CreateMsgPort(NULL);
				gd->gd_MyTimerPort	= CreateMsgPort(NULL);

				NewList((pList) &gd->gd_Root->on_NotifyList);
				NewList((pList) &gd->gd_Root->on_RecordList);
				NewList((pList) &gd->gd_Root->on_WaitList);
				NewList((pList) &gd->gd_Orphaned);
				NewList((pList) &gd->gd_FreeMessages);

				gd->gd_Root->on_Type= ST_USERDIR;
				Strcpy(gd->gd_Root->on_FileName, "Ram Disk");
				gd->gd_LockList		= NULL;
				gd->gd_SpaceUsed	= 1;
				gd->gd_VolumeNode	= MakeDosEntry("Ram Disk", DLT_VOLUME);

				gd->gd_VolumeNode->de_Handler				= gd->gd_MyPort;
				gd->gd_VolumeNode->de_Misc.volumeNode.de_DiskType	= ID_DOS_DISK;
				DateStamp(&gd->gd_VolumeNode->de_Misc.volumeNode.de_VolumeDate);

				if (AddDosEntry(gd->gd_VolumeNode) == DOSCMD_SUCCESS)
				{
					pDosEntry	devnode = (pDosEntry) dp->dp_Arg3;
					devnode->de_Handler	= gd->gd_MyPort;
					devnode->de_Misc.volumeNode.de_LockList = NULL;
					devnode->de_Misc.volumeNode.de_DiskType = ID_DOS_DISK;
					DateStamp(&gd->gd_Root->on_Date);
					ReplyPkt(dp, DOSIO_TRUE, RETURN_OK);

					KPrintF("RAM_Handler loop\n");

					INT32	mrp_sig, time_sig;
					INT32	waitMask = ( (1<< gd->gd_MyPort->mp_SigBit) |
										(mrp_sig	= (1<< gd->gd_MyReplyPort->mp_SigBit)) |
										(time_sig	= (1<< gd->gd_MyTimerPort->mp_SigBit)) );

					INT32	res1;
					UINT32	sigs;

					while(TRUE)
					{
						sigs = WaitSignal(waitMask);
						//KPrintF("Received something: %x on port %x \n", sigs, gd->gd_MyPort);

						while ( (dp = (pDosPacket)GetMsg(HandlerPort)) !=  NULL )
						{
							//KPrintF("action %ld (packet at %lx\n)",dp->dp_Action, dp);
							res1 = gd->gd_FileErr = DOSIO_FALSE;

							switch (dp->dp_Action)
							{
								case ACTION_WRITE:
									res1 = rh_Write(gd, (pRamLock) dp->dp_Arg1, (INT8*)dp->dp_Arg2, dp->dp_Arg3);
									break;
								case ACTION_READ:
									res1 = rh_Read(gd,(pRamLock) dp->dp_Arg1, (INT8*)dp->dp_Arg2, dp->dp_Arg3);
									break;
								case ACTION_EXAMINE_OBJECT:
								case ACTION_EXAMINE_FH:
									res1 = rh_ExNext(gd,dp,0);
									break;
								case ACTION_EXAMINE_NEXT:
									res1 = rh_ExNext(gd,dp,1);
									break;
								case ACTION_EXAMINE_ALL:
									res1 = rh_ExNext(gd,dp,2);
									break;
								case ACTION_LOCATE_OBJECT:
									//KPrintF("Getting Lock [%x] on [%s] with mode: %d\n",dp->dp_Arg1,dp->dp_Arg2,dp->dp_Arg3);
									res1 = (DOSIO) rh_Locate(gd, (pRamLock) dp->dp_Arg1, (STRPTR)dp->dp_Arg2, dp->dp_Arg3);
									break;
								case ACTION_FREE_LOCK:
									//KPrintF("Freeing lock on : [%s]\n", ((pObjNode)((pRamLock)dp->dp_Arg1)->rl_Lock.fl_Key)->on_FileName);
									res1 = rh_FreeLock(gd,(pRamLock) dp->dp_Arg1);
									break;
								case ACTION_PARENT_FH:
								case ACTION_COPY_DIR_FH:
								case ACTION_PARENT:
								case ACTION_COPY_DIR:
									res1 = (DOSIO) rh_Parentfh(gd, (pRamLock) dp->dp_Arg1, dp->dp_Action);
									break;

								case MODE_READWRITE:
									res1 = rh_Open( gd,(pFileHandle) dp->dp_Arg1, (pRamLock)dp->dp_Arg2, (STRPTR)dp->dp_Arg3, 2);
									break;
								case MODE_OLDFILE:
									res1 = rh_Open( gd,(pFileHandle) dp->dp_Arg1, (pRamLock)dp->dp_Arg2, (STRPTR)dp->dp_Arg3, 1);
									break;
								case MODE_NEWFILE:
//									KPrintF("Newfile\n");
									res1 = rh_Open( gd,(pFileHandle) dp->dp_Arg1, (pRamLock)dp->dp_Arg2, (STRPTR)dp->dp_Arg3, 0);
									break;
								case ACTION_END:
									res1 = rh_Close(gd,(pRamLock) dp->dp_Arg1);
									break;
								case ACTION_READ_LINK:
									res1 = rh_ReadLink(gd,(pRamLock) dp->dp_Arg1, (char *) dp->dp_Arg2,(INT8 *) dp->dp_Arg3,(UINT32) dp->dp_Arg4);
									break;
								case ACTION_RENAME_OBJECT:
									res1 = rh_Rename(gd,(pRamLock) dp->dp_Arg1,(STRPTR)dp->dp_Arg2,(pRamLock)dp->dp_Arg3,(STRPTR)dp->dp_Arg4);
									break;
								case ACTION_DELETE_OBJECT:
									node = rh_CheckLock(gd,(pRamLock) dp->dp_Arg1);
									if (node)
									{
										res1 = rh_DeleteObject(gd,node,(STRPTR)dp->dp_Arg2, TRUE);
									}
									break;
								case ACTION_SAME_LOCK:
									node = rh_CheckLock(gd,(pRamLock) dp->dp_Arg1);
									if (node)
									{
										dp->dp_Arg1 = (INT32) rh_CheckLock(gd,(pRamLock)dp->dp_Arg2);
										if (dp->dp_Arg1 == (INT32) node) res1 = TRUE;
									}
									break;
								case ACTION_CURRENT_VOLUME:
									res1 = (DOSIO) gd->gd_VolumeNode;
									break;
								case ACTION_SET_COMMENT:
								case ACTION_SET_PROTECT:
								case ACTION_SET_DATE:
									res1 = rh_Comment(gd, dp, (STRPTR)dp->dp_Arg3);
									break;
								case ACTION_INFO:
									if (!rh_CheckLock(gd,(pRamLock) dp->dp_Arg1))
									{
										gd->gd_FileErr = ERROR_OBJECT_IN_USE;
										break;
									}
								case ACTION_DISK_INFO:
									res1 = rh_DiskInfo(gd,dp);
									break;
								case ACTION_FLUSH:
									res1 = DOSIO_TRUE;
									break;
								case ACTION_RENAME_DISK:
									res1 = rh_RenameDisk(gd,dp);
									break;
								case ACTION_ADD_NOTIFY:
									res1 = rh_Addnotify(gd,(struct NotifyRequest *) dp->dp_Arg1);
									break;
								case ACTION_REMOVE_NOTIFY:
									res1 = rh_RemNotify(gd,(struct NotifyRequest *) dp->dp_Arg1);
									break;
								case ACTION_DIE:
									gd->gd_FileErr = ERROR_OBJECT_IN_USE;
									break;
								case ACTION_CREATE_DIR:
									res1 = (DOSIO) rh_CreateObject(gd,(pRamLock) dp->dp_Arg1, (STRPTR)dp->dp_Arg2,EXCLUSIVE_LOCK,TRUE);
									break;
								case ACTION_MAKE_LINK:
									res1 = rh_Makelink(gd,(pRamLock) dp->dp_Arg1, (STRPTR)dp->dp_Arg2, dp->dp_Arg3, dp->dp_Arg4);
									break;
								case ACTION_IS_FILESYSTEM:
									res1 = DOSIO_TRUE;
									break;
								case ACTION_FH_FROM_LOCK:
									res1 = rh_OpenFromLock(gd,(struct FileHandle *) dp->dp_Arg1, (pRamLock)dp->dp_Arg2);
									break;
								case ACTION_CHANGE_MODE:
									if (dp->dp_Arg1 == CHANGE_LOCK << 2)
									{
										res1 = rh_ChangeLock(gd,(pRamLock)dp->dp_Arg2, dp->dp_Arg3);
									} else if (dp->dp_Arg1 == CHANGE_FH << 2)
									{
										res1 = rh_ChangeLock(gd,(pRamLock)((pFileHandle)dp->dp_Arg2)->fh_Arg1, dp->dp_Arg3 == MODE_NEWFILE ? EXCLUSIVE_LOCK : SHARED_LOCK);
									} else
										gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
									break;
								case ACTION_LOCK_RECORD:
									res1 = rh_LockRecord(gd,(pRamLock) dp->dp_Arg1,dp);
									if (res1 == 1) continue;
									break;
								case ACTION_FREE_RECORD:
									res1 = rh_Unlockrecord(gd,(pRamLock) dp->dp_Arg1, dp->dp_Arg2,dp->dp_Arg3);
									break;
								case ACTION_SET_FILE_SIZE:
									res1 = rh_Seek(gd,(pRamLock) dp->dp_Arg1, dp->dp_Arg2, dp->dp_Arg3, TRUE);
									break;
								case ACTION_SEEK:
									res1 = rh_Seek(gd,(pRamLock) dp->dp_Arg1, dp->dp_Arg2, dp->dp_Arg3, FALSE);
									break;
								default:
									gd->gd_FileErr = ERROR_ACTION_NOT_KNOWN;
									break;
							}
							//KPrintF("returning packet: res1: %d, res2: %d\n", res1, gd->gd_FileErr);
							ReplyPkt(dp, res1, gd->gd_FileErr);
						}
					}

				} else
				{
					FreeDosEntry(gd->gd_VolumeNode);
					DeleteMsgPort(gd->gd_MyReplyPort);
					DeleteMsgPort(gd->gd_MyTimerPort);
					KPrintF("Ram_Handler-->PANIC, COULDNT ADD DOSENTRY\n");
					while(TRUE);
				}
			} else
			{
				KPrintF("Ram_Handler-->PANIC, NO MEMORY\n");
				while(TRUE);
			}
		} else
		{
			KPrintF("Ram_Handler-->PANIC, NO PACKET!\n");
			while(TRUE);
		}
	} else
	{
		KPrintF("Ram_Handler-->PANIC, NO SYSBASE, NO DOS, NO MEMORY!\n");
		while(TRUE);
	}
	return;
}

