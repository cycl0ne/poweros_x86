/**
 * @file functions.c
 *
 * This file describes a standard DOS handler for use with the ram (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "ram_handler.h"

extern struct NotifyMessage NotifyMessage, *pNotifyMessage;
extern struct NotifyRequest NotifyRequest, *pNotifyRequest;

#define DOSBase		gd->gd_dosBase
#define UtilBase	gd->gd_utilBase
#define SysBase		gd->gd_sysBase

INT32 rh_DoMatch(pGlobalData gd, struct ExAllControl *ec, INT32 data, struct ExAllData *ed)
{
//FIX!
	return 1;
}


void rh_CopyDate(pDateStamp sdate,pDateStamp ddate)
{
	ddate->ds_Days	= sdate->ds_Days;
	ddate->ds_Minute= sdate->ds_Minute;
	ddate->ds_Tick	= sdate->ds_Tick;
}

static inline STRPTR rh_Tail (pGlobalData gd, STRPTR str) {
	STRPTR end;
	end = str + Strlen(str);
	while (end > str)
	{
		end--;
		if (*end == '/' || *end == ':')
		{
			end++;
			break;
		}
	}
	return end;
}

INT32 rh_FreeList(pGlobalData gd, pObjNode list)
{
	pObjNode	node = list;
	INT32		ret = 0;
	while(list != NULL)
	{
		node = list->on_Next;
		FreeVec(list);
		ret++;
		list = node;
	}
	return ret;
}

DOSIO rh_Open(pGlobalData gd, pFileHandle fh, pRamLock dlock, STRPTR string, INT32 action) {
	pRamLock	lock = NULL;
	pObjNode	node = NULL;
	pDataBlock	data = NULL;
	BOOL	created = FALSE;

	if (action == 0)
	{
		data = AllocVec(sizeof(DataBlock), MEMF_FAST);
		if (!data) return DOSIO_FALSE;
		lock = rh_CreateObject(gd, dlock, string, EXCLUSIVE_LOCK, FALSE);
		created = TRUE;
	} else
	{
		lock = rh_Locate(gd, dlock, string, SHARED_LOCK);
		if (!lock && action == 2 && gd->gd_FileErr == ERROR_OBJECT_NOT_FOUND)
		{
			data = AllocVec(sizeof(DataBlock), MEMF_FAST);
			if (!data) return DOSIO_FALSE;
			lock = rh_CreateObject(gd, dlock, string, EXCLUSIVE_LOCK, FALSE);
			created = TRUE;
		}
	}

	if (lock)
	{
		node = (pObjNode) lock->rl_Lock.fl_Key;
		if (created)
		{
			node->on_Type = ST_FILE;
			lock->rl_Flags= LOCK_MODIFY;
		} else if (node->on_Type != ST_FILE)
		{
			rh_FreeLock(gd, lock);
			gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
			if (data) FreeVec(data);
			return DOSIO_FALSE;
		}

		if (data)
		{
			node->on_List	= (pObjNode)data;
			data->db_Node	= NULL;
			data->db_Size	= FIRSTBUFFPOS-1;
			lock->rl_Block	= data;
			gd->gd_SpaceUsed++;
		}

		fh->fh_Arg1 = (INT32)lock;
		return DOSIO_TRUE;
	}

	if (data) FreeVec(data);
	return DOSIO_FALSE;
}

DOSIO rh_OpenFromLock(pGlobalData gd, pFileHandle fh, pRamLock lock) {
	pObjNode node;

	node = (pObjNode)lock->rl_Lock.fl_Key;

	if (node->on_Type != ST_FILE)
	{
		gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
		return DOSIO_FALSE;
	}
	fh->fh_Arg1 = (INT32)lock;
	return DOSIO_TRUE;
}

DOSIO rh_Close(pGlobalData gd, pRamLock lock) {
	pObjNode	node;
	node = (pObjNode) lock->rl_Lock.fl_Key;
	if (lock->rl_Flags & LOCK_MODIFY) DateStamp(&node->on_Date);
//KPrintF("Return FreeLock\n");
	return rh_FreeLock(gd, lock);
}

pRamLock rh_CreateObject(pGlobalData gd, pRamLock lock, STRPTR string, INT32 mode, BOOL is_dir) {
	pObjNode	node;
	pObjNode	dptr, save;
	char		name[MAX_FILENAME+1];
	STRPTR		comm = NULL;
	INT32		prot = 0;

	dptr = rh_CheckLock(gd,lock);
	if (dptr == NULL) return NULL;

	dptr = rh_FindDir(gd, dptr, string, name);
	if (dptr == NULL) return NULL;

	if (!rh_CheckName(gd, name)) return NULL;

	if (rh_FindEntry(gd, dptr, name, TRUE))
	{
		//CreateObject will leave here if the is_dir is true.
		// And we found another "dir".
		//For fileoperations, the existing file will be overwritten.
		if (is_dir || gd->gd_CurNode->on_Type == ST_USERDIR)
		{
			gd->gd_FileErr = ERROR_OBJECT_EXISTS;
			return NULL;
		}

		prot = gd->gd_CurNode->on_Protection;
		comm = gd->gd_CurNode->on_Comment;
		gd->gd_CurNode->on_Comment = NULL;
		save = gd->gd_CurNode;
	}

	if (!(rh_DeleteObject(gd, dptr, name, FALSE)) && (gd->gd_FileErr != ERROR_OBJECT_NOT_FOUND))
	{
		save->on_Comment = comm;
		return NULL;
	}

	node = AllocVec(sizeof(ObjNode), MEMF_FAST);
	if (!node)
	{
		FreeVec(comm);
		return NULL;
	}

	gd->gd_SpaceUsed+=1;
	node->on_Next	= dptr->on_List;
	node->on_Parent	= dptr;
	node->on_Type	= ST_USERDIR;
	node->on_List	= NULL;
	node->on_FirstLink = NULL;
	node->on_Size	= 0;
	node->on_Protection  = prot;
	node->on_Comment = comm;
	NewList((struct List *) &(node->on_NotifyList));
	NewList((struct List *) &(node->on_RecordList));
	NewList((struct List *) &(node->on_WaitList));

	dptr->on_List = node;
	DateStamp(&node->on_Date);
	DateStamp(&dptr->on_Date);

	rh_NotifyNode(gd, dptr);
	Strcpy(node->on_FileName, name);
	rh_FindNotifies(gd, node, FALSE);
	lock = rh_GetLock(gd, node, mode);

	if (lock)
		lock->rl_Flags |= LOCK_MODIFY;
	else
		rh_DeleteObject(gd, dptr, name, TRUE);

	return lock;
}

DOSIO rh_DeleteObject(pGlobalData gd, pObjNode dptr, STRPTR string, BOOL notify) {
	pObjNode	node;
	pRamLock	lock = NULL;

	node = rh_FindNode(gd, dptr, string, FALSE);
	if (!node) return DOSIO_FALSE;

	if (node->on_Type == ST_USERDIR && node->on_List != NULL)
	{
		gd->gd_FileErr = ERROR_DIRECTORY_NOT_EMPTY;
		return DOSIO_FALSE;
	}

	if (node->on_Protection & FIBF_DELETE)
	{
		gd->gd_FileErr = ERROR_DELETE_PROTECTED;
		return DOSIO_FALSE;
	}

	if (node->on_Type != ST_SOFTLINK)
	{
		lock = rh_GetLock(gd, node,EXCLUSIVE_LOCK);
		if (lock == NULL) return DOSIO_FALSE;

		rh_RemNotifies(gd, node,notify);
		if (node->on_FirstLink && (node->on_Type == ST_FILE || node->on_Type == ST_USERDIR))
		{
			node->on_FirstLink->on_Type		= node->on_Type;
			node->on_FirstLink->on_List		= node->on_List;
			node->on_FirstLink->on_Size		= node->on_Size;
			node->on_FirstLink->on_Protection = node->on_Protection;
			rh_CopyDate(&node->on_FirstLink->on_Date, &node->on_Date);
			node->on_FirstLink->on_Comment 	= node->on_Comment;

			node->on_Comment= NULL;
			node->on_List 	= NULL;

			pObjNode n;

			rh_FindNotifies(gd, node->on_FirstLink,FALSE);

			for (n = node->on_FirstLink->on_FirstLink; n; n = n->on_FirstLink)
			{
				n->on_List = node->on_FirstLink;
				rh_FindNotifies(gd, n, FALSE);
			}
			node->on_FirstLink = NULL;
		}
	}

	if (node->on_Type != ST_LINKFILE && node->on_Type != ST_LINKDIR)
	{
		gd->gd_SpaceUsed -= rh_FreeList(gd, node->on_List);
	}

	gd->gd_SpaceUsed -= 1;

	if (notify) DateStamp(&node->on_Parent->on_Date);

	rh_RemoveNode(gd, node);

	if (node->on_Comment) FreeVec(node->on_Comment);
	FreeVec(node);
	rh_FreeLock(gd, lock);

	return DOSIO_TRUE;
}

void rh_RemoveNode(pGlobalData gd, pObjNode node) {
	pObjNode n;
	node->on_Parent->on_DelCount++;

	// check this..
	n = (pObjNode) &(node->on_Parent->on_List);

	while (n)
	{
		if (n->on_Next == node)
		{
			n->on_Next = node->on_Next;
			break;
		}
		n = n->on_Next;
	}

	if (node->on_Type == ST_LINKFILE || node->on_Type == ST_LINKDIR)
	{
		for (n = node->on_List; n; n = n->on_FirstLink)
		{
			if (n->on_FirstLink == node)
			{
				n->on_FirstLink = node->on_FirstLink;
				break;
			}
		}
	}
}

DOSIO rh_Read(pGlobalData gd, pRamLock lock, INT8 *buf, INT32 size)
{
	pDataBlock	pos, next;
	INT32 		offset;
	INT32		max, count;
	INT32		avail, needed, tsize;

	pos		= lock->rl_Block;
	offset	= lock->rl_Offset;

	if (pos == NULL) return 0; // = 0 bytes transfered==EOF

	max		= pos->db_Size;
	count	= 0;
	needed	= size;

	while (needed >0)
	{
		avail = max - offset + 1;
		if (avail <= 0)
		{
			next = pos->db_Node;
			if (next == NULL) break;

			pos    = next;
			max    = pos->db_Size;
			offset = FIRSTBUFFPOS;
			continue;
		}
		tsize = (avail < needed ? avail : needed);

		xfer(((char *) pos) + offset,
		     ((char *) buf) + count, tsize);

		count  += tsize;
		offset += tsize;
		needed -= tsize;
	}
	lock->rl_Block	= pos;
	lock->rl_Offset	= offset;
	lock->rl_CPos	+= count;
	return count;
}

DOSIO rh_Write(pGlobalData gd, pRamLock lock, INT8 *buf, INT32 bsize)
{
	pObjNode	node;
	pDataBlock	pos, next;
	INT32 		offset, curpos;
	INT32		max, count;
	INT32		avail, needed, tsize;

	node   = (pObjNode) lock->rl_Lock.fl_Key;
	pos    = lock->rl_Block;
	offset = lock->rl_Offset;
	curpos = lock->rl_CPos;

	max = MAX_WRITE;

	count = 0;

	while (count < bsize)
	{
		needed = bsize - count;
		avail  = max - offset + 1;
		if (avail <= 0)
		{
			next = pos->db_Node;
			if (next == NULL)
			{
				next = AllocVec(sizeof(DataBlock), 0);
				if (next == NULL)
				{
					count = -1;
					goto exit;
				}
				next->db_Node = NULL;
				next->db_Size = FIRSTBUFFPOS - 1;
				pos->db_Node  = next;
				gd->gd_SpaceUsed += 1;
			}
			pos    = next;
			max    = MAX_WRITE;
			offset = FIRSTBUFFPOS;
			avail  = max - FIRSTBUFFPOS + 1;
		}
		tsize = (avail < needed ? avail : needed);

		xfer(((char *) buf) + count,
		     ((char *) pos) + offset, tsize);

		count  += tsize;
		offset += tsize;
		curpos += tsize;

		if (offset > pos->db_Size)
			pos->db_Size = offset-1;
	}

	/* save end positions */
	lock->rl_Block	= pos;
	lock->rl_Offset	= offset;
	lock->rl_CPos	= curpos;

exit:
	lock->rl_Flags |= LOCK_MODIFY;
	if (curpos > node->on_Size) node->on_Size = curpos;
	return count;
}

pObjNode rh_FindNode(pGlobalData gd, pObjNode dptr, STRPTR str, BOOL follow_links)
{
	char name[MAX_FILENAME+1];

	dptr = rh_FindDir(gd, dptr,str,name);
	if (dptr == NULL) return NULL;

	if (name[0] != '\0')
	{
		if (!rh_FindEntry(gd, dptr,name,follow_links)) return NULL;
		dptr = gd->gd_CurNode;
	}
	return dptr;
}

pObjNode rh_LocateNode(pGlobalData gd, pRamLock lock,STRPTR str, BOOL follow_links)
{
	pObjNode dptr;

	dptr = rh_CheckLock(gd, lock);
	if (dptr == NULL) return NULL;
	return rh_FindNode(gd, dptr,str,follow_links);
}

pRamLock rh_Locate(pGlobalData gd, pRamLock lock, STRPTR str, INT32 mode)
{
	pObjNode dptr = rh_LocateNode(gd, lock,str,TRUE);
	return (dptr ? rh_GetLock(gd, dptr,mode) : NULL);
}

pObjNode rh_CheckLock(pGlobalData gd, pRamLock lock)
{
	pRamLock p;
	pRamLock *last = NULL;

	if (lock == NULL) return gd->gd_Root;

	for (p = gd->gd_LockList; p && p != lock;
	     last = (pRamLock*)&(p->rl_Lock.fl_Next),
		 p = (pRamLock)p->rl_Lock.fl_Next)
		;

	if (p == NULL)
	{
		gd->gd_FileErr = ERROR_INVALID_LOCK;
		return NULL;
	}
	// only for speed increase, put the node to the front of the list
	if (last)
	{
		*last = (pRamLock)p->rl_Lock.fl_Next;
		p->rl_Lock.fl_Next = (pFileLock)gd->gd_LockList;
		gd->gd_LockList = p;
	}
	return  (pObjNode)lock->rl_Lock.fl_Key;
}

pRamLock rh_GetLock(pGlobalData gd, pObjNode ptr, INT32 access)
{
	pRamLock p, lock;
	p = gd->gd_LockList;
	p = rh_FindLock(gd, p, ptr);
	//KPrintF("Found Lock?: %x, access: %d\n",p, p->rl_Lock.fl_Access);
	if (p != NULL && (p->rl_Lock.fl_Access == EXCLUSIVE_LOCK || access == EXCLUSIVE_LOCK))
	{
		gd->gd_FileErr = ERROR_OBJECT_IN_USE;
		return NULL;
	}

	lock = AllocVec(sizeof(RamLock), MEMF_FAST);
	if (!lock) {gd->gd_FileErr = ERROR_NO_FREE_STORE; return NULL;}

	lock->rl_Lock.fl_Next   = (pFileLock)gd->gd_LockList;
	lock->rl_Lock.fl_Key    = (INT32) ptr;
	lock->rl_Lock.fl_Access = access;
	lock->rl_Lock.fl_Handler= gd->gd_MyPort;
	lock->rl_Lock.fl_Volume = gd->gd_VolumeNode;

	lock->rl_Block	= (pDataBlock)ptr->on_List;
	lock->rl_Offset	= FIRSTBUFFPOS;
	lock->rl_CPos	= 0;
	lock->rl_Flags	= 0;

	gd->gd_LockList = lock;
	return lock;
}

DOSIO rh_FreeLock (pGlobalData gd, pRamLock lock) {
	pRamLock p;

	if (lock == NULL) return DOSIO_TRUE;

	p = (pRamLock)&gd->gd_LockList;//->rl_Lock.fl_Next;
	
	while (p->rl_Lock.fl_Next != NULL && p->rl_Lock.fl_Next != (pFileLock)lock)
	{
//		KPrintF("[%s]",((pObjNode)p->rl_Lock.fl_Key)->on_FileName);
		p = (pRamLock)p->rl_Lock.fl_Next;
	}

	if (p->rl_Lock.fl_Next == NULL)
	{
		gd->gd_FileErr = ERROR_INVALID_LOCK;
		return DOSIO_FALSE;
	}
	p->rl_Lock.fl_Next = lock->rl_Lock.fl_Next;

	if (lock->rl_Flags & LOCK_MODIFY)
	{
		rh_Notify(gd, lock);
		((pObjNode) lock->rl_Lock.fl_Key)->on_Protection &= ~FIBF_ARCHIVE;
	}
	lock->rl_Lock.fl_Handler = NULL;

	FreeVec(lock);
	return DOSIO_TRUE;
}

pRamLock rh_FindLock(pGlobalData gd, pRamLock p, pObjNode node) {
	while (p != NULL && p->rl_Lock.fl_Key !=  (INT32)node)
		p = (pRamLock) p->rl_Lock.fl_Next;
	return p;
}

DOSIO rh_ChangeLock(pGlobalData gd, pRamLock lock, INT32 newmode) {
	pRamLock p;
	if (!lock || !rh_CheckLock(gd, lock)) return DOSIO_FALSE;
	if (newmode == EXCLUSIVE_LOCK && lock->rl_Lock.fl_Access != EXCLUSIVE_LOCK)
	{
		for (p = gd->gd_LockList; p; p = (pRamLock) p->rl_Lock.fl_Next)
		{
			if (p->rl_Lock.fl_Key == lock->rl_Lock.fl_Key && p != lock)
			{
				gd->gd_FileErr = ERROR_OBJECT_IN_USE;
				return DOSIO_FALSE;
			}
		}
	}
	lock->rl_Lock.fl_Access = newmode;
	return DOSIO_TRUE;
}

BOOL rh_CheckName(pGlobalData gd, STRPTR name) {
	INT16 i = 0;
	UINT8 ch;

	gd->gd_FileErr = ERROR_INVALID_COMPONENT_NAME;

	while (i++ <= MAX_FILENAME)
	{
		ch = (UINT8) *name++;
		if (ch == '\0') break;
		if (ch < ' ' || ch == '/' || ch == ':') return FALSE;
	}
	if (i == 1) return FALSE;
	gd->gd_FileErr = 0;
	return TRUE;
}

DOSIO rh_Comment(pGlobalData gd, pDosPacket pkt, STRPTR objname) {
	pObjNode node;
	STRPTR	str;
	INT32	type,comm;
	UINT32	len;

	str  = (STRPTR) pkt->dp_Arg4;
	type = pkt->dp_Action;
	comm = (type == ACTION_SET_COMMENT);

	if (type == ACTION_SET_COMMENT)
	{
		len  = Strlen(str);
	}

	if (comm && (len > MAX_COMMENT))
	{
		gd->gd_FileErr = ERROR_COMMENT_TOO_BIG;
		return DOSIO_FALSE;
	}

	node = rh_LocateNode(gd, (pRamLock) pkt->dp_Arg2, objname, TRUE);
	if (!node) return DOSIO_FALSE;

	if (node == gd->gd_Root)
	{
		gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
		return DOSIO_FALSE;
	}

	if (comm)
	{
		FreeVec(node->on_Comment);
		if (str)
		{
			node->on_Comment = AllocVec(len, MEMF_FAST);
			if (node->on_Comment)
			{
				Strcpy(node->on_Comment, str);
			} else {
				return DOSIO_FALSE;
			}
		} else
			node->on_Comment = NULL;
	} else if (type == ACTION_SET_PROTECT)
	{
		node->on_Protection = (INT32) str;
	} else
	{
		rh_CopyDate(&node->on_Date, (pDateStamp)str );
		rh_NotifyNode(gd, node);
	}
	return DOSIO_TRUE;
}

void rh_DeleteDir(pGlobalData gd, pObjNode node)
{
	pObjNode next;

	while (node != NULL)
	{
		next = node->on_Next;
		if (node->on_Type == ST_USERDIR) rh_DeleteDir(gd, node->on_List);
		rh_FreeList(gd, node->on_List);
		FreeVec(node);
		node = next;
	}
}

pObjNode rh_FindDir(pGlobalData gd, pObjNode dptr, STRPTR string, STRPTR name)
{
	INT32 ptr,p,len;
	STRPTR temp;
	temp = Strchr(string,':');
	if (temp) string = temp+1;
	len = Strlen(string);

	ptr = 0;
	gd->gd_CurNode = dptr;

	while (1)
	{
		p = SplitName(string,'/',name, ptr, MAX_FILENAME+1);
		if (p < 0) return dptr;

		gd->gd_PathPos = p;
		if (p == ptr+1)
		{
			gd->gd_CurNode = dptr = dptr->on_Parent;
			if (dptr == NULL)
			{
				gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
				return NULL;
			}
			if (p == len)
			{
				//KPrintF("dptr->on_FileName: [%s]\n", dptr->on_FileName);
				return dptr;
			}
			ptr = p;
			continue;
		} else
			ptr = p;

		if (!rh_FindEntry(gd, dptr,name,TRUE)) return NULL;
		dptr = gd->gd_CurNode;
	}
}

pRamLock rh_Parentfh(pGlobalData gd, pRamLock lock, INT32 action)
{
	pObjNode node;
	node = rh_CheckLock(gd, lock);
	if (node == NULL) return NULL;

	if (action != ACTION_COPY_DIR_FH && action != ACTION_COPY_DIR) node = node->on_Parent;
	return (node ? rh_GetLock(gd, node, SHARED_LOCK) : NULL);
}

DOSIO rh_Rename(pGlobalData gd, pRamLock from_dir, STRPTR from_str, pRamLock to_dir, STRPTR to_str)
{
	char		to_name[MAX_FILENAME+1], from_name[MAX_FILENAME+1];
	pObjNode	from_dptr, to_dptr, from_node;
	pRamLock	lock;
	INT32		samename;

	from_dptr = rh_CheckLock(gd, from_dir);
	if (from_dptr == NULL)	return DOSIO_FALSE;

	from_dptr = rh_FindDir(gd, from_dptr,from_str,from_name);
	if (!from_dptr) return DOSIO_FALSE;
	if (!rh_FindEntry(gd, from_dptr,from_name, FALSE)) return DOSIO_FALSE;
	from_node = gd->gd_CurNode;

	to_dptr = rh_CheckLock(gd, to_dir);
	if (to_dptr == NULL) return DOSIO_FALSE;

	to_dptr = rh_FindDir(gd, to_dptr,to_str,to_name);
	if (to_dptr == NULL) return DOSIO_FALSE;

	samename = (from_dptr == to_dptr && Stricmp(from_name,to_name) == SAME);

	if (!samename && rh_FindEntry(gd, to_dptr,to_name,FALSE))
	{
		gd->gd_FileErr = ERROR_OBJECT_EXISTS;
		return DOSIO_FALSE;
	}

	if (to_dptr->on_Type < 0)
	{
		gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
		return DOSIO_FALSE;
	}

	if (from_node->on_Type >= 0)
	{
		pObjNode tmp;
		for (tmp = to_dptr->on_Parent; tmp; tmp = tmp->on_Parent)
		{
			if (tmp == from_node)
			{
				gd->gd_FileErr = ERROR_OBJECT_IN_USE;
				return DOSIO_FALSE;
			}
		}
	}

	lock = rh_GetLock(gd, from_node, SHARED_LOCK);
	if (lock == NULL) return DOSIO_FALSE;

	if (!rh_CheckName(gd, to_name))
	{
		rh_FreeLock(gd, lock);
		return DOSIO_FALSE;
	}

	if (to_dptr == from_node)
	{
		gd->gd_FileErr = ERROR_OBJECT_IN_USE;
		rh_FreeLock(gd, lock);
		return DOSIO_FALSE;
	}

	DateStamp(&to_dptr->on_Date);
	DateStamp(&from_node->on_Parent->on_Date);
	rh_RemoveNode(gd, from_node);

	from_node->on_Next	= to_dptr->on_List;
	from_node->on_Parent= to_dptr;
	to_dptr->on_List	= from_node;

	Strcpy(from_node->on_FileName, to_name);
	rh_RemNotifies(gd, from_node, TRUE);
	rh_FindNotifies(gd, from_node,TRUE);
	rh_FreeLock(gd, lock);
	return DOSIO_TRUE;
}

DOSIO rh_RenameDisk(pGlobalData gd, pDosPacket pkt)
{
	STRPTR	name;
	STRPTR	ns;
	INT32	len;

	name = (STRPTR) pkt->dp_Arg1;
	len = Strlen(name);

	gd->gd_FileErr = ERROR_INVALID_COMPONENT_NAME;
	if (len < 1 || len > MAX_FILENAME) return DOSIO_FALSE;

	ns = AllocVec(len+2,MEMF_PUBLIC);
	if (ns == NULL)
	{
		gd->gd_FileErr = ERROR_DISK_FULL;
		return DOSIO_FALSE;
	}
	Strcpy(ns, name);
	//memcpy(ns,name,len+1); // CopyMem!!!
	//ns[len+1] = '\0';
	Strcpy(gd->gd_Root->on_FileName, name);
	//memcpy(gd->gd_Root->on_FileName,name+1,len);
	//gd->gd_Root->on_FileName[len] = '\0';

	DateStamp(&gd->gd_Root->on_Date);

	Forbid();
	FreeVec(gd->gd_VolumeNode->de_Node.ln_Name);
	gd->gd_VolumeNode->de_Node.ln_Name = ns;
	Permit();

	return DOSIO_TRUE;
}

BOOL rh_FindEntry(pGlobalData gd, pObjNode dptr, STRPTR name, BOOL follow_links)
{
	if ((dptr == NULL) || (dptr->on_Type != ST_USERDIR))
	{
		gd->gd_FileErr = (dptr == NULL ? ERROR_OBJECT_NOT_FOUND : ERROR_OBJECT_WRONG_TYPE);
		return FALSE;
	}

	gd->gd_CurNode = dptr->on_List;

	while (gd->gd_CurNode != NULL)
	{
		if (Stricmp(name,gd->gd_CurNode->on_FileName) == SAME)
		{
			if (follow_links)
			{
			    if (gd->gd_CurNode->on_Type == ST_LINKDIR || gd->gd_CurNode->on_Type == ST_LINKFILE)
			    {
					gd->gd_CurNode = gd->gd_CurNode->on_List;

			    } else if (gd->gd_CurNode->on_Type == ST_SOFTLINK)
				{
					gd->gd_FileErr = ERROR_IS_SOFT_LINK;
					return FALSE;
			    }
			}
			return TRUE;
		}
		gd->gd_CurNode = gd->gd_CurNode->on_Next;
	}

	gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
	return FALSE;
}

DOSIO rh_DiskInfo(pGlobalData gd, pDosPacket pkt)
{
	struct InfoData *p;
	p = (struct InfoData *) ((pkt->dp_Action == ACTION_DISK_INFO ?  pkt->dp_Arg1 : pkt->dp_Arg2));

	p->id_NumSoftErrors	= 0;
	p->id_UnitNumber	= -1;
	p->id_DiskState		= ID_VALIDATED;
	p->id_NumBlocks		= gd->gd_SpaceUsed;
	p->id_NumBlocksUsed	= gd->gd_SpaceUsed;
	p->id_BytesPerBlock	= MIN_BLKSIZE;
	p->id_DiskType		= ID_DOS_DISK;
	p->id_VolumeNode	= (pDosEntry)gd->gd_VolumeNode;
	p->id_InUse			= NULL;
	return DOSIO_TRUE;
}

INT32 rh_Seek(pGlobalData gd, pRamLock lock, INT32 dest, INT32 frompos, BOOL truncate)
{
	pObjNode	node;
	pDataBlock	p, lastp;
	INT8*		mem;
	INT32		count, ptr, newcount;


	node   = (pObjNode) lock->rl_Lock.fl_Key;
	count  = lock->rl_CPos;

	if (frompos == SEEK_CUR)	dest += count;
	else if (frompos == SEEK_END)	dest += node->on_Size;
	else if (frompos == SEEK_SET) 
	{
		/*NULL*/ ;
	} else {
seek_error:
		gd->gd_FileErr = ERROR_SEEK_ERROR;
//		KPrintF("Error1\n");
		return -1;
	}

	if (dest < 0 || (!truncate && dest > node->on_Size)) 
	{
//		KPrintF("frompos: %d\n", frompos);
//		KPrintF("dest: %d\n", dest);
//		KPrintF("truncate: %d\n", truncate);
//		KPrintF("Error2 (%d)\n", node->on_Size);
		goto seek_error;
		
	}

	if (dest > count)
	{
		lastp = p = lock->rl_Block;
		ptr   = count - (lock->rl_Offset - FIRSTBUFFPOS);
	} else {
		lastp = p = (pDataBlock)node->on_List;
		ptr = 0;
	}

	while (1)
	{
		if (p == NULL)
		{
			if (!truncate) goto seek_error;

			lock->rl_Block  = lastp;
			lock->rl_Offset = lastp->db_Size + 1;
			lock->rl_CPos   = node->on_Size;

			mem = AllocVec(DATA_BLOCK_SIZE*4,MEMF_CLEAR);
			if (!mem) goto seek_error;

			while (ptr+(DATA_BLOCK_SIZE*4) < dest)
			{
				if (rh_Write(gd, lock, mem,(DATA_BLOCK_SIZE*4)) != (DATA_BLOCK_SIZE*4))
				{
					FreeVec(mem);
					goto seek_error;
				}
				ptr += DATA_BLOCK_SIZE*4;
			}
			newcount = rh_Write(gd, lock, mem, dest-ptr);
			FreeVec( mem );
			if (newcount != dest-ptr) goto seek_error;
			return dest;
		}

		newcount = ptr + p->db_Size - (FIRSTBUFFPOS-1);
		if (newcount >= dest) break;

		ptr = newcount;
		lastp = p;
		p = p->db_Node;
	}

	if (truncate)
	{
		pRamLock curr;
		if (dest < node->on_Size)
		{
		    for (curr = gd->gd_LockList; curr; curr = (pRamLock)curr->rl_Lock.fl_Next)
		    {
				if (curr != lock)
				{
					if (curr->rl_Lock.fl_Key == lock->rl_Lock.fl_Key && curr->rl_CPos > dest)
					{
						dest = curr->rl_CPos;
						p = curr->rl_Block;
						ptr = dest + FIRSTBUFFPOS - curr->rl_Offset;
					}
				}
		    }
		}

		gd->gd_SpaceUsed -= rh_FreeList(gd, (pObjNode)p->db_Node);
		p->db_Node		= NULL;
		p->db_Size		= dest-ptr + (FIRSTBUFFPOS-1);
		node->on_Size	= dest;

		lock->rl_Flags |= LOCK_MODIFY;
		count = dest;

		if (lock->rl_CPos > dest)
		{
			goto do_seek;
		}
	} else {
do_seek:
		lock->rl_Block	= p;
		lock->rl_Offset	= dest-ptr + FIRSTBUFFPOS;
		lock->rl_CPos	= dest;
	}
//	return count;
	return lock->rl_CPos;
}

DOSIO rh_ExNext(pGlobalData gd, pDosPacket pkt, INT32 object)
{
	struct FileInfoBlock *ivec;
	pObjNode node;
	pObjNode ptr;
	pRamLock lock;

	lock = (pRamLock) pkt->dp_Arg1;
	ptr  = rh_CheckLock(gd, lock);
	if (ptr == NULL) return DOSIO_FALSE;

	if (object == 2) return rh_ExAll(gd, ptr,pkt);

	ivec = (struct FileInfoBlock *) pkt->dp_Arg2;
	node = (pObjNode) 				ivec->fib_DiskKey;

	if (object != 0 && node != ptr)
	{
		if (lock->rl_DelCount != ptr->on_DelCount)
		{
			pObjNode n;

			for (n = ptr->on_List; n; n = n->on_Next)
			{
				if (n == node) break;
			}
			if (!n)
			{
				node = ptr;
			}
		}
	}
	lock->rl_DelCount = ptr->on_DelCount;
	return rh_SetInfo(gd, ivec, object == 0 ? ptr : ptr == node ? node->on_List : node->on_Next);
}

DOSIO rh_ExAll(pGlobalData gd, pObjNode node, pDosPacket dp)
{
	struct ExAllData *ed	= (void *) dp->dp_Arg2;
	struct ExAllData *lasted= NULL;
	INT32 size				= dp->dp_Arg3;
	INT32 data				= dp->dp_Arg4;
	struct ExAllControl *ec = (void *) dp->dp_Arg5;
	pObjNode curr;
	INT32 savesize;

	if (node->on_Type != ST_USERDIR)
	{
		gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
		return DOSIO_FALSE;
	}

	if (data < ED_NAME || data > ED_COMMENT)
	{
		gd->gd_FileErr = ERROR_BAD_NUMBER;
		return DOSIO_FALSE;
	}

	ec->eac_Entries = 0;

	for (curr = ec->eac_LastKey ? (pObjNode) ec->eac_LastKey : node->on_List;
	     curr;
	     curr = curr->on_Next)
	{
		savesize = size;
		if (!rh_FillData(gd, ed,data,curr,&size))
		{
			if (lasted)	lasted->ed_Next = NULL;
			ec->eac_LastKey = (INT32) curr;
			return DOSIO_TRUE;
		}

		if (ec->eac_MatchString)
		{
			if (!MatchPatternNoCase(ec->eac_MatchString, curr->on_FileName)) goto reject;
		}

		if (ec->eac_MatchFunc)
		{
			if (!rh_DoMatch(gd, ec,data,ed))
			{
reject:			size = savesize;
				continue;
			}
		}
		lasted = ed;
		ec->eac_Entries++;
		ed = ed->ed_Next;
	}

	if (lasted) lasted->ed_Next = NULL;

	gd->gd_FileErr = ERROR_NO_MORE_ENTRIES;
	return DOSIO_FALSE;
}

static UINT8 str_offset[ED_COMMENT] = {
	8,12,16,20,32,36
};

BOOL rh_FillData(pGlobalData gd, struct ExAllData *ed, UINT32 data, pObjNode node, INT32 *size)
{
	STRPTR p = ((char *) ed) + str_offset[data-1];
	INT32 mysize = 0;

	switch (data) {
	case ED_COMMENT:
		mysize += 4 + (node->on_Comment ? Strlen(node->on_Comment) : 0) + 1;
	case ED_DATE:
		mysize += 3*4;
	case ED_PROTECTION:
		mysize += 4;
	case ED_SIZE:
		mysize += 4;
	case ED_TYPE:
		mysize += 4;
	case ED_NAME:
		mysize += 2*4 + Strlen(node->on_FileName) + 1;
		if (mysize > *size) return FALSE;
	}
	if (mysize & 1)	mysize++;

	*size -= mysize;

	switch (data) {
	case ED_COMMENT:
		ed->ed_Comment = p;
		*p = '\0';
		if (node->on_Comment) Strcpy(p,node->on_Comment);
		p += Strlen(p) + 1;
	case ED_DATE:
		rh_CopyDate((pDateStamp) &(ed->ed_Days), &node->on_Date);
	case ED_PROTECTION:
		ed->ed_Prot = node->on_Protection;
	case ED_SIZE:
		ed->ed_Size = node->on_Size;
	case ED_TYPE:
		ed->ed_Type = node->on_Type;
	case ED_NAME:
		ed->ed_Name = p;
		Strcpy(p,node->on_FileName);
		ed->ed_Next = (struct ExAllData *) (((INT32) ed) + mysize);
	}
	return TRUE;
}

DOSIO rh_SetInfo(pGlobalData  gd, struct FileInfoBlock *ivec, pObjNode ptr)
{
	INT32 type;

	if (ptr == NULL)
	{
		gd->gd_FileErr = ERROR_NO_MORE_ENTRIES;
		return DOSIO_FALSE;
	}

	ivec->fib_DiskKey	= (INT32) ptr;
	type                    =
	ivec->fib_DirEntryType	=
	ivec->fib_EntryType	= ptr->on_Type;

	Strcpy(ivec->fib_FileName, ptr->on_FileName);

	if (type == ST_LINKFILE || type == ST_LINKDIR) ptr = ptr->on_List;

	ivec->fib_Protection	= ptr->on_Protection;
	ivec->fib_Size			= ptr->on_Size;
	ivec->fib_NumBlocks		= (ptr->on_Size >> BLKSHIFT) + 1;
	ivec->fib_Date.ds_Days	= ptr->on_Date.ds_Days;
	ivec->fib_Date.ds_Minute= ptr->on_Date.ds_Minute;
	ivec->fib_Date.ds_Tick	= ptr->on_Date.ds_Tick;
	ivec->fib_Comment[0]	= '\0';
	if (ptr->on_Comment) Strcpy(ivec->fib_Comment, ptr->on_Comment);
	return DOSIO_TRUE;
}

DOSIO rh_LockRecord (pGlobalData  gd, pRamLock lock, pDosPacket dp)
{
	if (rh_FindRLock(gd, lock,dp->dp_Arg2,dp->dp_Arg3,dp->dp_Arg4)) return DOSIO_TRUE;

	if ((dp->dp_Arg4) & 1)
	{
		gd->gd_FileErr = ERROR_LOCK_COLLISION;
		return DOSIO_FALSE;
	}

	return rh_RLockWaitAdd(gd, lock, dp);
}

DOSIO rh_FindRLock(pGlobalData gd, pRamLock lock, UINT32 offset, UINT32 length, UINT32 mode)
{
	struct rlock *rlock;
	UINT32 top = offset + length;

	for (rlock = (void *) (((pObjNode) lock->rl_Lock.fl_Key)->on_RecordList.mlh_Head);
	     rlock->next;
	     rlock = rlock->next)
	{
		if (rlock->lock != lock &&
		    rlock->offset < top &&
		    rlock->offset + rlock->length > offset)
		{
			if (mode <= REC_EXCLUSIVE_IMMED) return DOSIO_FALSE;
			else
			{
				if (rlock->mode <= REC_EXCLUSIVE_IMMED) return DOSIO_FALSE;
			}
		}
	}

	rlock = AllocVec(sizeof(struct rlock), MEMF_FAST);
	if (!rlock) return DOSIO_FALSE;

	rlock->lock   = lock;
	rlock->offset = offset;
	rlock->length = length;
	rlock->mode   = mode;

	AddHead((struct List *) &(((pObjNode) lock->rl_Lock.fl_Key)->on_RecordList), (struct Node *) rlock);
	return DOSIO_TRUE;
}

DOSIO rh_RLockWaitAdd(pGlobalData gd, pRamLock lock, pDosPacket dp)
{
	struct rlock_wait *rwait;

	rwait = AllocVec(sizeof(struct rlock_wait), MEMF_FAST);
	if (!rwait) return DOSIO_FALSE;

	rwait->dp = dp;
#if 0
	/* play EVIL games, copy the dos timer block! */
	rwait->iob = *(DOSBase->dl_TimeReq);
	rwait->iob.tr_node.io_Message.mn_ReplyPort = timerport;

	rwait->iob.tr_node.io_Command = TR_ADDREQUEST;
	rwait->iob.tr_time.tv_micro   = mult32(rem32(dp->dp_Arg5,TICKS_PER_SECOND), (1000000/TICKS_PER_SECOND));
	rwait->iob.tr_time.tv_secs    = div32(dp->dp_Arg5,TICKS_PER_SECOND);
#endif
	SendIO(&(rwait->iob.tr_node));

	AddTail((struct List *) &(((pObjNode) lock->rl_Lock.fl_Key)->on_WaitList), (struct Node *) rwait);
	return 1;
}

DOSIO rh_Unlockrecord(pGlobalData gd, pRamLock lock, UINT32 offset, UINT32 length)
{
	struct rlock *rlock;

	for (rlock = (void *) (((pObjNode) lock->rl_Lock.fl_Key)->on_RecordList.mlh_Head);
	     rlock->next;
	     rlock = rlock->next)
	{
		if (rlock->lock   == lock &&
		    rlock->offset == offset &&
		    rlock->length == length)
		{
			Remove((struct Node *) rlock);
			FreeVec((void *) rlock);
			rh_Wakeup(gd, (pObjNode) lock->rl_Lock.fl_Key, offset, offset + length);
			return DOSIO_TRUE;
		}
	}

	gd->gd_FileErr = ERROR_RECORD_NOT_LOCKED;
	return DOSIO_FALSE;
}

void rh_Wakeup(pGlobalData gd, pObjNode node, UINT32 offset, UINT32 top)
{
	struct rlock_wait *rwait,*next;

	for (rwait = (void *) (node->on_WaitList.mlh_Head);
	     rwait->next;
	     rwait = next)
	{
		next = rwait->next;
		if (rwait->dp->dp_Arg2 < top &&
		    rwait->dp->dp_Arg2 + rwait->dp->dp_Arg3 > offset)
		{
			if (rh_FindRLock(gd, (pRamLock) rwait->dp->dp_Arg1,
					rwait->dp->dp_Arg2,
					rwait->dp->dp_Arg3,
					rwait->dp->dp_Arg4))
			{
				AbortIO((pIOStdReq) &(rwait->iob));
				WaitIO((pIOStdReq) &(rwait->iob));
				Remove((struct Node *) rwait);
				ReplyPkt(rwait->dp,DOSIO_TRUE,0);
				FreeVec((void *) rwait);
			}
		}
	}
}

void rh_KillRWait(pGlobalData gd, struct TimeRequest *iob)
{
	struct rlock_wait *rwait;

	rwait = (void *) (((char *) iob) - offsetof(struct rlock_wait,iob));
	Remove((struct Node *) rwait);

	rh_Wakeup(gd, (pObjNode)(((pRamLock) rwait->dp->dp_Arg1)->rl_Lock.fl_Key),
	       rwait->dp->dp_Arg2,rwait->dp->dp_Arg2 + rwait->dp->dp_Arg3);

	ReplyPkt(rwait->dp, FALSE, ERROR_LOCK_TIMEOUT);
	FreeVec((void *) rwait);
}

DOSIO rh_Makelink(pGlobalData gd, pRamLock lock, STRPTR string, INT32 arg, BOOL soft)
{
	pObjNode newnode, node;
	pRamLock new;
	INT32 type;

	if (!soft)
	{
		node = rh_CheckLock(gd, (pRamLock) arg);
		if (!node) return DOSIO_FALSE;
	} else
	{
		gd->gd_FileErr = ERROR_NOT_IMPLEMENTED;
		return DOSIO_FALSE;
	}

	newnode = rh_LocateNode(gd, lock, string, FALSE);
	if (newnode)
	{
		gd->gd_FileErr = ERROR_OBJECT_EXISTS;
		return DOSIO_FALSE;
	}

	new = rh_CreateObject(gd, lock,string,EXCLUSIVE_LOCK,FALSE);
	if (!new) return DOSIO_FALSE;

	newnode = (pObjNode) new->rl_Lock.fl_Key;

	if (soft)
	{
		newnode->on_Type = ST_SOFTLINK;
		rh_Write(gd, new, (INT8 *)arg, Strlen((STRPTR)arg));
	} else
	{
		node = (pObjNode) ((pRamLock)arg)->rl_Lock.fl_Key;
		type = node->on_Type;

		if (type < 0)	newnode->on_Type = ST_LINKFILE;
		else 			newnode->on_Type = ST_LINKDIR;

		newnode->on_FirstLink 	= node->on_FirstLink;
		node->on_FirstLink    	= newnode;
		newnode->on_List		= node;
	}

	rh_FreeLock(gd, new);
	return DOSIO_TRUE;
}

DOSIO rh_ReadLink(pGlobalData gd, pRamLock lock, STRPTR string, INT8 *buffer, UINT32 size)
{
	pObjNode node;
	BOOL is_dir = FALSE;
	INT32 res,len;

	node = rh_LocateNode(gd, lock,string,FALSE);

	if (!node)
	{
		if (gd->gd_FileErr == ERROR_IS_SOFT_LINK)
		{
			is_dir = TRUE;
			node = gd->gd_CurNode;
		} else
			return -1;
	}

	if (node->on_Type != ST_SOFTLINK)
	{
		gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
		return -1;
	}

	if (is_dir)
	{
		string = &string[gd->gd_PathPos];
		len = Strlen(string) + 1;
	} else
		len = 0;

	if (node->on_Size + 1 + len >= size)
	{
		gd->gd_FileErr = node->on_Size + 1 + len;
		return -2;
	}

	lock = rh_GetLock(gd, node, SHARED_LOCK);
	if (!lock) return -1;

	res = rh_Read(gd, lock, buffer,size);

	if (res >= 0)
	{
		if (is_dir)
		{
			if (buffer[res-1] != ':')
				buffer[res++] = '/';

			Strcpy((STRPTR)&(buffer[res]),string);
			res += len-1;
		} else {
			buffer[res] = '\0';
		}
	}
	rh_FreeLock(gd, lock);
	return res;
}

DOSIO rh_Addnotify(pGlobalData gd, struct NotifyRequest *req)
{
	struct notify *notify;
	pObjNode node;

	notify = AllocVec(sizeof(struct notify), MEMF_FAST);
	if (!notify) return DOSIO_FALSE;

	notify->Req		= req;
	req->nr_MsgCount= 0;
	notify->Name = rh_Tail(gd, (STRPTR)req->nr_FullName);

	node = rh_Exists(gd, (STRPTR)req->nr_FullName);
	if (node)
	{
		AddHead((struct List *)&(gd->gd_CurNode->on_NotifyList),(struct Node *) notify);
		notify->node = gd->gd_CurNode;
		if (req->nr_Flags & NRF_NOTIFY_INITIAL) rh_DoNotify(gd, notify);
		return DOSIO_TRUE;
	}
	AddHead((struct List *) &gd->gd_Orphaned,(struct Node *) notify);
	return DOSIO_TRUE;
}

DOSIO rh_RemNotify(pGlobalData gd, struct NotifyRequest *req)
{
	struct notify *notify;
	pObjNode node = rh_Exists(gd, (STRPTR)req->nr_FullName);

	if (node)
	{
		notify = (struct notify *) node->on_NotifyList.mlh_Head;
	} else {
		notify = (struct notify *) gd->gd_Orphaned.mlh_Head;
	}


	for ( ; notify->Succ; notify = notify->Succ)
	{
		if (notify->Req == req)
		{
			req = notify->Req;

			if ((req->nr_Flags & NRF_WAIT_REPLY) && req->nr_MsgCount)
				notify->msg->nm_DoNotTouch &= ~NRF_WAIT_REPLY;

			Remove((struct Node *) notify);
			FreeVec(notify);
			return DOSIO_TRUE;
		}
	}
	return NULL;
}

pObjNode rh_Exists(pGlobalData gd, STRPTR file)
{
	return rh_FindNode(gd, gd->gd_Root,file,TRUE);
}

void rh_Notify(pGlobalData gd, pRamLock lock)
{
	lock->rl_Flags &= ~LOCK_MODIFY;
	rh_NotifyNode(gd, (pObjNode) lock->rl_Lock.fl_Key);
}

void rh_NotifyNode(pGlobalData gd, pObjNode node)
{
	struct notify *notify;

	for (notify = (struct notify *) node->on_NotifyList.mlh_Head;
	     notify->Succ;
	     notify = notify->Succ)
	{
		rh_DoNotify(gd, notify);
	}
}

void rh_DoNotify (pGlobalData gd, struct notify *notify)
{
	struct NotifyMessage *msg;
	struct NotifyRequest *req;

	req = notify->Req;

	if ((req->nr_Flags & NRF_WAIT_REPLY) && req->nr_MsgCount > 0)
	{
		req->nr_Flags |= NRF_MAGIC;
		return;
	}

	if (req->nr_Flags & NRF_SEND_SIGNAL)
	{
		SignalTask(req->nr_stuff.nr_Signal.nr_Task, (1L << req->nr_stuff.nr_Signal.nr_SignalNum));
	}

	if ((req->nr_Flags & NRF_SEND_MESSAGE) &&  ((msg = rh_FindMsg(gd)) != NULL))
	{
		msg->nm_ExecMessage.mn_ReplyPort = gd->gd_MyReplyPort;
		msg->nm_Class       = NOTIFY_CLASS;
		msg->nm_Code        = NOTIFY_CODE;
		msg->nm_NReq	    = req;
		msg->nm_DoNotTouch  = req->nr_Flags & NRF_WAIT_REPLY;
		msg->nm_DoNotTouch2 = (INT32) notify;

		PutMsg(req->nr_stuff.nr_Msg.nr_Port,&(msg->nm_ExecMessage));
		req->nr_MsgCount++;
		notify->msg      =  msg;
	}
}

void rh_FindNotifies(pGlobalData gd, pObjNode node, INT32 flag)
{
	char filename[MAX_FILENAME+1];

	pObjNode dptr;
	struct notify *notify,*next;

	for (notify = (struct notify *) gd->gd_Orphaned.mlh_Head;
	     notify->Succ;
	     notify = next)
	{
		next = notify->Succ;
		if (Stricmp((STRPTR)notify->Name, (STRPTR)node->on_FileName) == SAME)
		{
			dptr = rh_FindDir(gd, gd->gd_Root, (STRPTR)notify->Req->nr_FullName, (STRPTR)filename);
			if (dptr == node->on_Parent)
			{
				Remove((struct Node *) notify);
				AddHead((struct List *) &(node->on_NotifyList), (struct Node *) notify);
				notify->node = node;
				if (flag) rh_DoNotify(gd, notify);
			}
		}
	}
}

struct NotifyMessage *rh_FindMsg(pGlobalData gd)
{
	struct NotifyMessage *msg;

	if (!(((struct Node *) gd->gd_FreeMessages.mlh_Head)->ln_Succ))
	{
		return AllocVec(sizeof(struct NotifyMessage), MEMF_FAST);
	}

	msg = (struct NotifyMessage *) gd->gd_FreeMessages.mlh_Head;
	Remove(&(msg->nm_ExecMessage.mn_Node));
	return msg;
}

void rh_RemNotifies(pGlobalData gd, pObjNode node, INT32 flag)
{
	struct notify *notify,*next;

	for (notify = (struct notify *) node->on_NotifyList.mlh_Head;
	     notify->Succ;
	     notify = next)
	{
		next = notify->Succ;

		Remove((struct Node *) notify);
		AddHead((struct List *) &gd->gd_Orphaned,(struct Node *) notify);
		notify->node = NULL;

		if (flag) rh_DoNotify(gd, notify);
	}
}

void rh_AddFreeMsg (pGlobalData gd, struct NotifyMessage *msg)
{
	struct NotifyRequest *req;

	if (msg->nm_DoNotTouch & NRF_WAIT_REPLY)
	{
		req = msg->nm_NReq;
		req->nr_MsgCount--;

		/* if it was modified, notify again */
		if (req->nr_Flags & NRF_MAGIC)
		{
			rh_DoNotify(gd, (struct notify *) msg->nm_DoNotTouch2);
			req->nr_Flags &= ~NRF_MAGIC;
		}
	}
	AddHead((struct List *) &gd->gd_FreeMessages,(struct Node *) msg);
}
