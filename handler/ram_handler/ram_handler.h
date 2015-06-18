/**
 * @file ram_handler.h
 *
 * This file describes a standard DOS handler for use with the ram (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "lists.h"
#include "timer.h"

#include "dos.h"
#include "dos_errors.h"
#include "dos_packets.h"
#include "dos_io.h"
#include "dos_notify.h"
#include "dos_exall.h"
#include "dos_records.h"

#include "exec_interface.h"
#include "utility_interface.h"
#include "dos_interface.h"

#define BLKSHIFT		10
#define MIN_BLKSIZE		(1L << BLKSHIFT)
#define FIRSTBUFFPOS	8
#define DATA_EXTRA		(FIRSTBUFFPOS >> 2)
#define MAX_WRITE		(FIRSTBUFFPOS+MIN_BLKSIZE-1)
#define DATA_BLOCK_SIZE ((MIN_BLKSIZE >> 2) + DATA_EXTRA)
#define MAX_FILENAME	30
#define MAX_COMMENT		79
#define FIRSTBUFFPOS	8
#define LOCK_MODIFY		1
#define SAME 0

#define offsetof(st, m) __builtin_offsetof(st, m)
#define xfer(a,b,c)	CopyMem(a,b,c)

typedef struct DataBlock {
	struct DataBlock*	db_Node;
	UINT32				db_Size;
	INT8				db_Data[1024];
}DataBlock, *pDataBlock;

typedef struct ObjNode {
	struct ObjNode		*on_Next;
	struct ObjNode		*on_Parent;
	struct ObjNode		*on_List;

	struct ObjNode		*on_FirstLink;
	UINT32				 on_Size;
	UINT32				 on_Protection;
	DateStamp			 on_Date;
	STRPTR				 on_Comment;
	MinList				 on_NotifyList;
	MinList				 on_RecordList;
	MinList				 on_WaitList;
	INT32				 on_Type;
	UINT32				 on_DelCount;
	char				 on_FileName[MAX_FILENAME+1];
}ObjNode, *pObjNode;

typedef struct RamLock {
	FileLock	 		rl_Lock;
	pDataBlock			rl_Block;
	UINT32		 		rl_Offset;
	UINT32				rl_CPos;
	UINT32				rl_Flags;
	UINT32				rl_DelCount;	
}RamLock, *pRamLock;

typedef struct GlobalData {
	APTR				gd_sysBase;
	APTR				gd_dosBase;
	APTR				gd_utilBase;
	
	pObjNode			gd_Root;
	pObjNode			gd_CurNode;
	pRamLock			gd_LockList;
	INT32				gd_PathPos;
	INT32				gd_SpaceUsed;
	INT32				gd_FileErr;

	pDosEntry			gd_VolumeNode;
	INT32				gd_DiskType;
	pProcess			gd_MyProc;
	pMsgPort			gd_MyPort;
	pMsgPort			gd_MyReplyPort;
	pMsgPort			gd_MyTimerPort;
	MinList				gd_Orphaned;
	MinList				gd_FreeMessages;
}GlobalData, *pGlobalData;

struct notify {
	struct notify *Succ;
	struct notify *Pred;
	STRPTR			Name;		   /* pointer to last portion of name */
	struct NotifyRequest *Req; /* we return it when removed */
	pObjNode 		node;	   /* NULL if not in a chain */
	struct NotifyMessage *msg; /* for WAIT_REPLY */
};

struct rlock {
	struct rlock *next;
	struct rlock *prev;
	pRamLock	lock;	/* the filehandle that holds this */
	INT32 offset;
	INT32 length;
	INT32 mode;
};

struct rlock_wait {
	struct rlock_wait *next;
	struct rlock_wait *prev;
	struct TimeRequest iob;  /* the timer ior for this lock attempt */
	struct DosPacket *dp;	  /* the packet sent to me */
};

DOSIO rh_Open(pGlobalData gd, pFileHandle fh, pRamLock dlock, STRPTR string, INT32 action);
DOSIO rh_OpenFromLock(pGlobalData gd, pFileHandle fh, pRamLock lock);
DOSIO rh_Close(pGlobalData gd, pRamLock lock);
pRamLock rh_CreateObject(pGlobalData gd, pRamLock lock, STRPTR string, INT32 mode, BOOL is_dir);
DOSIO rh_DeleteObject(pGlobalData gd, pObjNode dptr, STRPTR string, BOOL notify);
void rh_RemoveNode(pGlobalData gd, pObjNode node);
DOSIO rh_Read(pGlobalData gd, pRamLock lock, INT8 *buf, INT32 size);
DOSIO rh_Write(pGlobalData gd, pRamLock lock, INT8 *buf, INT32 size);
pObjNode rh_FindNode(pGlobalData gd, pObjNode dptr, STRPTR string, BOOL follow_links);
pObjNode rh_LocateNode(pGlobalData gd, pRamLock lock,STRPTR str, BOOL follow_links);
pRamLock rh_Locate(pGlobalData gd, pRamLock lock, STRPTR str, INT32 mode);
pObjNode rh_CheckLock(pGlobalData gd, pRamLock lock);
pRamLock rh_GetLock(pGlobalData gd, pObjNode ptr, INT32 access);
DOSIO rh_FreeLock (pGlobalData gd, pRamLock lock);
pRamLock rh_FindLock(pGlobalData gd, pRamLock p, pObjNode node);

DOSIO rh_ChangeLock(pGlobalData gd, pRamLock lock, INT32 newmode);
BOOL rh_CheckName(pGlobalData gd, STRPTR name);
DOSIO rh_Comment(pGlobalData gd, pDosPacket pkt, STRPTR objname);
void rh_DeleteDir(pGlobalData gd, pObjNode node);
pObjNode rh_FindDir(pGlobalData gd, pObjNode dptr, STRPTR string, STRPTR name);
pRamLock rh_Parentfh(pGlobalData gd, pRamLock lock, INT32 action);
DOSIO rh_Rename(pGlobalData gd, pRamLock from_dir, STRPTR from_str, pRamLock to_dir, STRPTR to_str);
DOSIO rh_RenameDisk(pGlobalData gd, pDosPacket pkt);
BOOL rh_FindEntry(pGlobalData gd, pObjNode dptr, STRPTR name, BOOL follow_links);
DOSIO rh_DiskInfo(pGlobalData gd, pDosPacket pkt);
INT32 rh_Seek(pGlobalData gd, pRamLock lock, INT32 dest, INT32 frompos, BOOL truncate);
DOSIO rh_ExNext(pGlobalData gd, pDosPacket pkt, INT32 object);
DOSIO rh_ExAll(pGlobalData gd, pObjNode node, pDosPacket dp);
BOOL rh_FillData(pGlobalData gd, struct ExAllData *ed, UINT32 data, pObjNode node, INT32 *size);
DOSIO rh_SetInfo(pGlobalData  gd, struct FileInfoBlock *ivec, pObjNode ptr);
DOSIO rh_LockRecord (pGlobalData  gd, pRamLock lock, pDosPacket dp);
DOSIO rh_FindRLock(pGlobalData gd, pRamLock lock, UINT32 offset, UINT32 length, UINT32 mode);
DOSIO rh_RLockWaitAdd(pGlobalData gd, pRamLock lock, pDosPacket dp);
DOSIO rh_Unlockrecord(pGlobalData gd, pRamLock lock, UINT32 offset, UINT32 length);
void rh_Wakeup(pGlobalData gd, pObjNode node, UINT32 offset, UINT32 top);
void rh_KillRWait(pGlobalData gd, struct TimeRequest *iob);
DOSIO rh_Makelink(pGlobalData gd, pRamLock lock, STRPTR string, INT32 arg, BOOL soft);
DOSIO rh_ReadLink(pGlobalData gd, pRamLock lock, STRPTR string, INT8 *buffer, UINT32 size);
DOSIO rh_Addnotify(pGlobalData gd, struct NotifyRequest *req);
DOSIO rh_RemNotify(pGlobalData gd, struct NotifyRequest *req);
pObjNode rh_Exists(pGlobalData gd, STRPTR file);
void rh_Notify(pGlobalData gd, pRamLock lock);
void rh_NotifyNode(pGlobalData gd, pObjNode node);
void rh_DoNotify (pGlobalData gd, struct notify *notify);
void rh_FindNotifies(pGlobalData gd, pObjNode node, INT32 flag);
void rh_RemNotifies(pGlobalData gd, pObjNode node, INT32 flag);
void rh_AddFreeMsg (pGlobalData gd, struct NotifyMessage *msg);
struct NotifyMessage *rh_FindMsg(pGlobalData gd);

