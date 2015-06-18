/**
 * @file locks.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

#define DoPkt4(a,b,c,d,e,f)	DoPkt(a, b, (INT32)c, (INT32)d, (INT32)e,(INT32)f, 0)
#define DoPkt3(a,b,c,d,e)	DoPkt(a, b, (INT32)c, (INT32)d,	(INT32)e,0,0)
#define DoPkt2(a,b,c,d)		DoPkt(a, b, (INT32)c, (INT32)d,0,0,0)

pFileLock dos_Lock(pDOSBase DOSBase, STRPTR name, INT32 mode)
{
	pFileLock res	= 0;
	DOSIO	error	= 0;
	DOSIO	level	= 15;
	pHandlerProc	hp;

//	hp = ObtainHandler((STRPTR)name);
	
	while(TRUE)
	{
		hp = ObtainHandler((STRPTR)name);
		if (!hp) 
		{
			res = (pFileLock)DOSIO_FALSE;
			break;
		}
		
		res = (pFileLock) DoPkt3(hp->hp_Port, ACTION_LOCATE_OBJECT, hp->hp_Lock, name, mode);
		if (res) break;
		
		error = IoErr();

		if (error == ERROR_IS_SOFT_LINK)
		{
#if 0
// Take Softlinks into charge
			if (--level < 0)
			{
				error = ERROR_TO_MANY_LEVELS;
				SetIoErr(error);
				break;
			} else {
			
			}
#endif
		} else if (error == ERROR_OBJECT_NOT_FOUND)
		{
			if (!(hp->hp_Flags & DVPF_ASSIGN)) break;
		} else
		{
			//ErrorReport here!
			res = 0;
			break;
		}
	}
	
	ReleaseHandler(hp);
	return res;
}


DOSIO dos_UnLock(pDOSBase DOSBase, pFileLock lock)
{
	if (lock)
	{
		if (lock->fl_Handler)
		{
			INT32 err = IoErr();
			INT32 ret = DoPkt(lock->fl_Handler, ACTION_FREE_LOCK, (INT32)lock, 0, 0, 0, 0);
			SetIoErr(err);
			return ret;
		}
	}
	SetIoErr(ERROR_INVALID_LOCK);
	return DOSIO_FALSE;
}

DOSIO dos_SetProtection (pDOSBase DOSBase, const char *name, INT32 protect)
{
	DOSIO			res = DOSIO_FALSE;
	pHandlerProc	hp;

	hp = ObtainHandler((STRPTR)name);
	if (hp)
	{
		res = DoPkt4(hp->hp_Port, ACTION_SET_PROTECT, 0, hp->hp_Lock, name, protect);
		ReleaseHandler(hp);
	}
	return res;
}

DOSIO dos_SetOwner(pDOSBase DOSBase, const char *name, UINT32 owner)
{
	DOSIO			res = DOSIO_FALSE;
	pHandlerProc	hp;

	hp = ObtainHandler((STRPTR)name);
	if (hp)
	{
		res = DoPkt4(hp->hp_Port, ACTION_SET_OWNER, 0, hp->hp_Lock, name, owner);
		ReleaseHandler(hp);
	}
	return res;
}

DOSIO dos_SetComment(pDOSBase DOSBase, const char *name, const char *comment)
{
	DOSIO			res = DOSIO_FALSE;
	pHandlerProc	hp;

	hp = ObtainHandler((STRPTR)name);
	if (hp)
	{
		res = DoPkt4(hp->hp_Port, ACTION_SET_COMMENT, 0, hp->hp_Lock, name, comment);
		ReleaseHandler(hp);
	}
	return res;
}

DOSIO dos_DeleteFile(pDOSBase DOSBase, const char *name)
{
	DOSIO			res = DOSIO_FALSE;
	pHandlerProc	hp;

	hp = ObtainHandler((STRPTR)name);
	if (hp)
	{
		res = DoPkt2(hp->hp_Port, ACTION_DELETE_OBJECT, hp->hp_Lock, name);
		ReleaseHandler(hp);
	}
	return res;
}

pFileLock dos_CreateDir(pDOSBase DOSBase, const char *name)
{
	pFileLock 		res = DOSIO_FALSE;
	pHandlerProc	hp;

	hp = ObtainHandler((STRPTR)name);
	if (hp)
	{
		res = (pFileLock)DoPkt2(hp->hp_Port, ACTION_CREATE_DIR, hp->hp_Lock, name);
		ReleaseHandler(hp);
	}
	return res;
}



static DOSIO _ObjAction(pDOSBase DOSBase, STRPTR name, INT32 action, INT32 *arg)
{
	UINT32	len = Strlen(name);
	if (len < 256)
	{
		pHandlerProc dp = NULL;
		INT32	ret;
		while(1)
		{
			dp = ObtainHandler(name);
			if (!dp) return 0;
			
			switch (action)
			{
				case ACTION_SET_COMMENT:
				case ACTION_SET_OWNER:
				case ACTION_SET_PROTECT:
				case ACTION_SET_DATE:
					ret = DoPkt(dp->hp_Port, action, 0, (INT32)dp->hp_Lock, (INT32)name, arg[0], 0);
					break;
				default:
					KPrintF("_ObjAction: Port: %x, action: %d, lock: %x, name: %s\n", dp->hp_Port, action, dp->hp_Lock, name);
					ret = DoPkt(dp->hp_Port, action, (INT32)dp->hp_Lock, (INT32)name, arg[0], arg[1],0);
					break;
			}
			if (!ret)
			{
				switch(IoErr())
				{
					case ERROR_OBJECT_NOT_FOUND:
						if (dp->hp_Flags & DVPF_ASSIGN) continue;
						goto exit;
					default:
						break;						
				}
			}
exit:
			ReleaseHandler(dp);
			return ret;
#if 0			
			if (!ret) // FIX! should be "ret", but we have no ErrorReport()
			{
exit:		
				ReleaseHandler(dp);
				return ret;
			}
			ReleaseHandler(dp);
			dp = NULL;
#endif 
		}
	} else
		SetIoErr(ERROR_LINE_TOO_LONG);
	return DOSIO_FALSE;
}

#if 0
pFileLock dos_Lock(pDOSBase DOSBase, STRPTR name, INT32 mode)
{
	INT32	args[2];
	args[0] = mode;
	return (pFileLock)_ObjAction(DOSBase, name, ACTION_LOCATE_OBJECT, args);
}
#endif

DOSIO dos_MakeLink(pDOSBase DOSBase, const char *name, INT32 dest, INT32 soft)
{
	INT32 args[2];
	args[0] = dest;
	args[1] = soft;
	return _ObjAction(DOSBase, (STRPTR)name, ACTION_MAKE_LINK, args);
}

DOSIO dos_ReadLink(pDOSBase DOSBase, struct MsgPort *port, struct FileLock *lock, UINT8* path, UINT8 *buffer, UINT32 size)
{
	return DoPkt(port,ACTION_READ_LINK, (INT32)lock, (INT32)path, (INT32)buffer, (INT32)size, 0);
}


DOSIO dos_SetFileDate (pDOSBase DOSBase,const char *name, struct DateStamp *date)
{
	INT32 args[2];
	args[0] = (INT32)date;

	return _ObjAction(DOSBase,(STRPTR)name,ACTION_SET_DATE,args);
}

static DOSIO do_lock(pDOSBase DOSBase, struct FileLock *rlock, INT32 *args, INT32 action)
{
	INT32 rc;
	struct MsgPort *task;
	//struct FileInfoBlock *fib = (void *) args[0];
	INT32 arg1 = (INT32)rlock;

	do {
		/* NULL lock handling code here */
		/* bizarre condition code, careful */
		if (action == ACTION_EXAMINE_FH)
		{
			task = ((struct FileHandle *) rlock)->fh_Type;
			arg1 = ((struct FileHandle *) rlock)->fh_Arg1;
		} else {
			struct Process *tmp = FindProcess(NULL);
			task = (rlock ? rlock->fl_Handler : tmp->pr_FileSystemTask);
		}
//		KPrintF("DoPkt: Action: %d, arg1: %x\n", action, arg1);
		rc = DoPkt(task,action,arg1,args[0],args[1],args[2],0);
		if (rc == 0)
		{
//			KPrintF("AfterDoPkt\n");
			if (action != ACTION_EXAMINE_FH)
			{
				//if (ErrorReport(IoErr(), REPORT_LOCK, (UINT32)rlock, task)) return 0;
				return 0;
			} else {
				//if (ErrorReport(IoErr(), REPORT_STREAM, (UINT32)rlock,0)) return 0;
				return 0;
			}
		}
	} while (rc == 0);
	return rc;
}

DOSIO dos_Examine(pDOSBase DOSBase, struct FileLock *lock, struct FileInfoBlock *fib)
{
	INT32 args = (INT32) fib;
	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;
	return do_lock(DOSBase,lock,&args,ACTION_EXAMINE_OBJECT); 
}

DOSIO dos_ExNext(pDOSBase DOSBase, struct FileLock *lock, struct FileInfoBlock *fib)
{
	INT32 args = (INT32) fib;
	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;
	return do_lock(DOSBase,lock,&args,ACTION_EXAMINE_NEXT); 
}

DOSIO dos_ExamineFH(pDOSBase DOSBase, struct FileHandle *fh, struct FileInfoBlock *fib)
{
	INT32 args = (INT32) fib;
	fib->fib_OwnerUID = fib->fib_OwnerGID = 0;
	return do_lock(DOSBase,(struct FileLock *)fh,&args,ACTION_EXAMINE_FH);
}

DOSIO dos_Info(pDOSBase DOSBase, struct FileLock *lock, struct FileInfoBlock *fib)
{
	INT32 args = (INT32) fib;
	return do_lock(DOSBase,lock,&args,ACTION_INFO);
}

struct FileLock *dos_ParentDir(pDOSBase DOSBase, struct FileLock *lock)
{
	INT32 args = 0;
	//KPrintF("ParentDir: %x\n", lock);
	return (struct FileLock *)do_lock(DOSBase,lock,&args,ACTION_PARENT);
}

struct FileLock *dos_DupLock(pDOSBase DOSBase, struct FileLock *lock)
{
	if (lock == NULL) return 0;
	INT32 args = (INT32) lock;
	return (struct FileLock *)do_lock(DOSBase,lock,&args,ACTION_COPY_DIR);
}

struct FileHandle *dos_OpenFromLock(pDOSBase DOSBase, struct FileLock *lock)
{
	struct FileHandle *fh;
	struct MsgPort *task;
	INT32 rc;

	if (!lock)
	{
		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		return NULL;
	}

	fh = AllocVec(sizeof(FileHandle), MEMF_FAST|MEMF_CLEAR);
	if (!fh) return NULL;

	while (1) {
		task = lock->fl_Handler;
		fh->fh_Type = task;	

		rc = DoPkt(task,ACTION_FH_FROM_LOCK, (INT32)fh, (INT32)lock,0,0,0);

		if (rc )//|| ErrorReport(IoErr(), REPORT_LOCK,(UINT32)lock,task))
		{
			if (!rc)
			{
				FreeVec(fh);
				return 0;
			}
			return fh;
		}
	}
}

INT32 dos_SameLock(pDOSBase DOSBase, pFileLock l1, pFileLock l2)
{
	pFileLock lock1,lock2;

	if (l1 == l2) return LOCK_SAME;
	if (!(lock1 = l1) || !(lock2 = l2)) return LOCK_DIFFERENT;
	if (lock1->fl_Volume != lock2->fl_Volume) return LOCK_DIFFERENT;
	if (DoPkt2(lock1->fl_Handler,ACTION_SAME_LOCK,l1,l2)) return LOCK_SAME;

	if (IoErr() == ERROR_ACTION_NOT_KNOWN)
	{
		if (lock1->fl_Key == lock2->fl_Key) return LOCK_SAME;
	}
	return LOCK_SAME_VOLUME;
}

#if 0


#endif
