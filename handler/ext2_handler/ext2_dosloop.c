/**
 * @file ext2handler.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */

#include "bcache_interface.h"
#include "ext2_handler.h"

//----- Lock function
// ARG1 = Lock on directory ARG2 is relative to
// ARG2 = Name of object
// ARG3 = LONG mode: SHARED_LOCK, EXCLUSIVE_LOCK
// RES1 = Lock on requested object (0=failure)
// RES2 = failurecode

#if 0
void ext2_Lock(pGD gd, pDosPacket dp)
{
	KPrintF("Lock Debug: \n");
	KPrintF("Arg1: %x, ", dp->dp_Arg1);
	KPrintF("Arg2: %s, ", dp->dp_Arg1);
	KPrintF("Arg3: %x\n", dp->dp_Arg1);

	dp->dp_Res1 = DOSIO_FALSE;
	dp->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
}
#endif

INT32 ext2_UnknownCmd(pGD gd, pDosPacket dp)
{
	KPrintF("Err Action: %d\n", dp->dp_Action);
	gd->gd_FileErr = ERROR_ACTION_NOT_KNOWN;
	return DOSIO_FALSE;
}

INT32 ext2_SameLock(pGD gd, pDosPacket dp);
INT32 ext2_FreeLock(pGD gd, pFSLock lock);
pFSLock ext2_Locate(pGD gd, pFSLock lock, STRPTR str, INT32 mode);
//pFSLock ext2_Locate(pGD gd, pDosPacket dp);
INT32 ext2_ExNext(pGD gd, pDosPacket dp, INT32 x);
INT32 ext2_Parentfh(pGD gd, pDosPacket dp);
DOSIO ext2_Close(pGD gd, pFSLock);
DOSIO ext2_Open(pGD gd, pDosPacket pkt, INT32 action);
DOSIO ext2_Read(pGD, pFSLock, INT8*, INT32);
DOSIO ext2_AddBuffers(pGD gd, pDosPacket dp);
BOOL BackupSbAndBgdt(GD *gd);

#if 0
pFSLock ext2_Locate(pGD gd, pDosPacket dp)
{
	return  ext2_UnknownCmd(gd, dp);
}
INT32 ext2_FreeLock(pGD gd, pDosPacket dp)
{
	return  ext2_UnknownCmd(gd, dp);
}
INT32 ext2_ExNext(pGD gd, pDosPacket dp, INT32 x)
{
	return  ext2_UnknownCmd(gd, dp);
}
INT32 ext2_Parentfh(pGD gd, pDosPacket dp)
{
	return  ext2_UnknownCmd(gd, dp);
}
#endif

void ext2_InitTimerFlush(pGD gd, UINT32 secs)
{
	gd->gd_timesec = secs;
	gd->gd_trequest->tr_node.io_Command = TR_ADDREQUEST;
	gd->gd_trequest->tr_time.tv_micro   = 0;//micro;
	gd->gd_trequest->tr_time.tv_secs    = gd->gd_timesec;//secs;
	SendIO((IOStdReq*)gd->gd_trequest);
}

void ext2_SetTimeDirty(pGD gd)
{
	gd->gd_timedirty = TRUE;
}

void ext2_TimerFlush(pGD gd)
{
	if (gd->gd_timedirty)
	{
		gd->gd_timedirty = FALSE;
		BackupSbAndBgdt(gd);
		FlushCache(gd->gd_Cache);
		KPrintF("EXT2 disk cache flushed by timer\n");
	}
	gd->gd_trequest->tr_node.io_Command = TR_ADDREQUEST;
	gd->gd_trequest->tr_time.tv_micro   = 0;//micro;
	gd->gd_trequest->tr_time.tv_secs    = gd->gd_timesec;//secs;
	SendIO((IOStdReq*)gd->gd_trequest);
}

void ext2_DOSLoop(pGD gd)
{
	INT32 res1 = 0;
	pDosPacket dp = NULL;
	UINT32 sig = 0;

	while (TRUE)
	{
		while ( (dp = (pDosPacket) GetMsg(gd->gd_msgport)) != NULL )
		{
			// Reset our statusflags
			res1 = gd->gd_FileErr = DOSIO_FALSE;
			switch (dp->dp_Action)
			{
				case ACTION_LOCATE_OBJECT:
					res1 = (DOSIO)ext2_Locate(gd, (pFSLock) dp->dp_Arg1, (STRPTR)dp->dp_Arg2, dp->dp_Arg3);
					break;
				case ACTION_FREE_LOCK:
					res1 = ext2_FreeLock(gd, (pFSLock)dp->dp_Arg1);
					break;
				case ACTION_EXAMINE_OBJECT:
				case ACTION_EXAMINE_FH:
					res1 = ext2_ExNext(gd, dp, 0);
					break;
				case ACTION_EXAMINE_NEXT:
					res1 = ext2_ExNext(gd, dp, 1);
					break;
				case ACTION_EXAMINE_ALL:
					res1 = ext2_ExNext(gd, dp, 2);
					break;
				case ACTION_PARENT_FH:
				case ACTION_COPY_DIR_FH:
				case ACTION_PARENT:
				case ACTION_COPY_DIR:
					res1 = (DOSIO) ext2_Parentfh(gd, dp);
					break;
				case MODE_READWRITE:
					res1 = ext2_Open( gd, dp, MODE_READWRITE);
					break;
				case MODE_OLDFILE:
					res1 = ext2_Open( gd, dp, MODE_OLDFILE);
					break;
				case MODE_NEWFILE:
					res1 = ext2_Open( gd, dp, MODE_NEWFILE);
					break;
				case ACTION_END:
					res1 = ext2_Close(gd, (pFSLock)dp->dp_Arg1);
					break;
				case ACTION_READ:
					res1 = ext2_Read(gd, (pFSLock) dp->dp_Arg1, (INT8*) dp->dp_Arg2, dp->dp_Arg3);
				break;
				case ACTION_SEEK:
					res1 = ext2_Seek(gd, (FSLock*)dp->dp_Arg1,
                           dp->dp_Arg2, dp->dp_Arg3, FALSE);
					break;
				case ACTION_WRITE:
					res1 = ext2_Write(gd, (FSLock*)dp->dp_Arg1, (void*)dp->dp_Arg2, dp->dp_Arg3);
				break;
				case ACTION_SET_FILE_SIZE:
					res1 = ext2_Seek(gd, (FSLock*)dp->dp_Arg1,
                           dp->dp_Arg2, dp->dp_Arg3, TRUE);
				break;
				case ACTION_IS_FILESYSTEM:
					res1 = DOSIO_TRUE;
				break;
				case ACTION_SAME_LOCK:
					res1 = ext2_SameLock(gd, dp);
					break;
				case ACTION_MORE_CACHE:
					res1 = ext2_AddBuffers(gd, dp);
					KPrintF("res1 = %d\n", res1);
					break;
				case ACTION_DIE:
					BackupSbAndBgdt(gd);
					FlushCache(gd->gd_Cache);
					KPrintF("EXT2 disk cache flushed before shutdown\n");
					break;
				case ACTION_CREATE_DIR:
					res1 = (DOSIO)ext2_CreateObject(
					    gd, (FSLock*)dp->dp_Arg1, (char*)dp->dp_Arg2,
					    EXT2_S_IFDIR | 0775, 0, EXCLUSIVE_LOCK);
					break;
				case ACTION_DELETE_OBJECT:
					res1 = ext2_DeleteObject(
					    gd, (FSLock*)dp->dp_Arg1, (char*)dp->dp_Arg2) ?
					    DOSIO_TRUE : DOSIO_FALSE;
					break;
				case ACTION_INHIBIT:
				case ACTION_NIL:
				default:
					res1 = ext2_UnknownCmd(gd, dp);
					break;
			}
			ReplyPkt(dp, res1, gd->gd_FileErr);
		}

		if (sig & (1 << gd->gd_tport->mp_SigBit))
		{
			GetMsg(gd->gd_tport);
			ext2_TimerFlush(gd);
		}

		sig = WaitSignal( (1 << gd->gd_msgport->mp_SigBit) | (1 << gd->gd_tport->mp_SigBit));
	}
}

/*
#define ACTION_NIL		0
#define ACTION_STARTUP		0
#define ACTION_GET_BLOCK	2
#define ACTION_SET_MAP		4
#define ACTION_DIE		5
#define ACTION_EVENT		6
#define ACTION_CURRENT_VOLUME	7
#define ACTION_LOCATE_OBJECT	8
#define ACTION_RENAME_DISK	9
#define ACTION_WRITE		'W'
#define ACTION_READ		'R'
#define ACTION_FREE_LOCK	15
#define ACTION_DELETE_OBJECT	16
#define ACTION_RENAME_OBJECT	17
#define ACTION_MORE_CACHE	18
#define ACTION_COPY_DIR		19
#define ACTION_WAIT_CHAR	20
#define ACTION_SET_PROTECT	21
#define ACTION_CREATE_DIR	22
#define ACTION_EXAMINE_OBJECT	23
#define ACTION_EXAMINE_NEXT	24
#define ACTION_DISK_INFO	25
#define ACTION_INFO		26
#define ACTION_FLUSH		27
#define ACTION_SET_COMMENT	28
#define ACTION_PARENT		29
#define ACTION_TIMER		30
#define ACTION_INHIBIT		31
#define ACTION_DISK_TYPE	32
#define ACTION_DISK_CHANGE	33
#define ACTION_SET_DATE		34
#define ACTION_SCREEN_MODE	994
#define ACTION_READ_RETURN	1001
#define ACTION_WRITE_RETURN	1002
#define ACTION_SEEK		1008
#define ACTION_FINDUPDATE	1004
#define ACTION_FINDINPUT	1005
#define ACTION_FINDOUTPUT	1006
#define ACTION_END		1007
#define ACTION_SET_FILE_SIZE	1022
#define ACTION_WRITE_PROTECT	1023
#define ACTION_SAME_LOCK	40
#define ACTION_CHANGE_SIGNAL	995
#define ACTION_FORMAT		1020
#define ACTION_MAKE_LINK	1021
#define ACTION_READ_LINK	1024
#define ACTION_FH_FROM_LOCK	1026
#define ACTION_IS_FILESYSTEM	1027
#define ACTION_CHANGE_MODE	1028
#define ACTION_COPY_DIR_FH	1030
#define ACTION_PARENT_FH	1031
#define ACTION_EXAMINE_ALL	1033
#define ACTION_EXAMINE_FH	1034
#define ACTION_LOCK_RECORD	2008
#define ACTION_FREE_RECORD	2009
#define ACTION_ADD_NOTIFY	4097
#define ACTION_REMOVE_NOTIFY	4098
#define ACTION_EXAMINE_ALL_END	1035
#define ACTION_SET_OWNER	1036
#define	ACTION_SERIALIZE_DISK	4200

*/

