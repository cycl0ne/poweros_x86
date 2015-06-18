/**
* File: /ext2_lock．c
* User: cycl0ne
* Date: 2014-10-15
* Time: 01:55 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "bcache_interface.h"
#include "bitmap.h"
#include "ext2_handler.h"

static fs_node_t *GetRootNode(pGD gd)
{
	// do we have to do something with root special?
//	KPrintF("Returning Root\n");
	return gd->root_node;
}

static void ReleaseNode(pGD gd, fs_node_t *node)
{
	if (!node) return;
	node->count--;

	//KPrintF("ReleaseNode: Node: %s, %x, %d\n", node->name, node, node->count);

	//Root Node cant be disposed.
	if (node == gd->root_node) return;

	if (node->count < 1)
	{
		//Remove it from our List and from Memory
#if 0
		pFSLock lock = NULL;
		fs_node_t *node = NULL;

		ForeachNode(&gd->gd_LockList, lock)
		{
			node = (fs_node_t*) lock->rl_Lock.fl_Key;
			KPrintF("********Lock: %x N:%x,%s, %d \n", lock, node, node->name, node->count);
		}
#endif
		Remove((pNode)node);
		FreeVec(node);
	}
	return;
}

static BOOL FindEntry(pGD gd, fs_node_t *node, STRPTR name,
                      BOOL follow_links __attribute__((__unused__)))
{
	if ( (node == NULL) || ((node->flags & FS_DIRECTORY) != FS_DIRECTORY) )
	{
		gd->gd_FileErr = (node == NULL ? ERROR_OBJECT_NOT_FOUND : ERROR_OBJECT_WRONG_TYPE);
		return FALSE;
	}

	if (Strcmp(name, ".") == SAME)
	{
		gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
		return FALSE;
	}

	if (Strcmp(name, "..") == SAME)
	{
		gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
		return FALSE;
	}

	gd->gd_CurNode = ext2_FindDirectory(gd, node, name);
	if (gd->gd_CurNode) return TRUE;

	gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
	return FALSE;
}

static fs_node_t *FindDir(pGD gd, fs_node_t *node, STRPTR string, STRPTR name)
{
	STRPTR	temp = Strchr(string, ':');
	if (temp) string = temp+1;

	INT32	len_string	= Strlen(string);

	INT32	p;
	INT32	ptr = 0;
	gd->gd_CurNode = node;

	while(1)
	{
		p = SplitName(string, '/', name, ptr, MAX_FILENAME+1);
		if (p < 0) return node;

		gd->gd_PathPos = p;
		if (p == ptr+1)
		{
			gd->gd_CurNode = node = ext2_ParentDirectory(gd, node);

			if (node == NULL)
			{
				gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
				return NULL;
			}
			if (p == len_string)
			{
				return node;
			}
			ptr = p;
			continue;
		} else
			ptr = p;

		if (!FindEntry(gd, node, name, TRUE)) return NULL;
		node = gd->gd_CurNode;
	}
}

static fs_node_t *FindNode(pGD gd, fs_node_t *node, STRPTR str, BOOL follow_links)
{
	char name[MAX_FILENAME+1];
	node = FindDir(gd, node, str, name);
	if (node == NULL) return NULL;
	if (name[0] != '\0')
	{
		if (!FindEntry(gd, node, name, follow_links)) return NULL;
		node = gd->gd_CurNode;
	}
	return node;
}

static fs_node_t *CheckLock(pGD gd, pFSLock lock)
{
	pFSLock p;
	pFSLock last = NULL;

	if (lock == NULL) return GetRootNode(gd);

	ForeachNode(&gd->gd_LockList, p)
	{
		if (p == lock) break;
		last = p;
	}

	if (p == NULL)
	{
		gd->gd_FileErr = ERROR_INVALID_LOCK;
		return NULL;
	}

	if (last)
	{
		Remove((pNode)&p->rl_Lock.fl_Node);
		AddHead((pList)&gd->gd_LockList, (pNode)&p->rl_Lock.fl_Node);
	}

	return  (fs_node_t *)lock->rl_Lock.fl_Key;
}


static fs_node_t *LocateNode(pGD gd, pFSLock lock, STRPTR str, BOOL follow_links)
{
	fs_node_t *node = CheckLock(gd, lock);
//	if (lock != NULL) KPrintF("node: %s\n", node->name);
	if (node == NULL) return NULL;
	return FindNode(gd, node, str, follow_links);
}

static pFSLock FindLock(pGD gd, fs_node_t *node)
{
  pFSLock p;
  pFSLock ret = NULL;

  ForeachNode(&gd->gd_LockList, p)
  {
    if (p->rl_Lock.fl_Key == (INT32)node)
    {
      ret = p;
      break;
    }
  }
  return ret;
}

static pFSLock GetLock(pGD gd, fs_node_t *node, INT32 access)
{
	pFSLock p	= NULL;
	pFSLock lock= NULL;

	p = FindLock(gd, node);
	//KPrintF("Found Lock?: %x, access: %d\n",p, p->rl_Lock.fl_Access);
	if (p != NULL && (p->rl_Lock.fl_Access == EXCLUSIVE_LOCK || access == EXCLUSIVE_LOCK))
	{
		KPrintF("_GetLock: ERROR_OBJECT_IN_USE\n");
		ReleaseNode(gd, node);
		gd->gd_FileErr = ERROR_OBJECT_IN_USE;
		return NULL;
	}

	lock = AllocVec(sizeof(FSLock), MEMF_PUBLIC|MEMF_CLEAR);
	if (!lock)
	{
		KPrintF("_GetLock: ERROR_NO_FREE_STORE\n");
		ReleaseNode(gd, node);
		gd->gd_FileErr = ERROR_NO_FREE_STORE;
		return NULL;
	}
	//KPrintF("********Lock: %x, %d, %s\n",node, node->count, node->name);
	node->count++;
	lock->rl_Lock.fl_Key    = (INT32) node;
	lock->rl_Lock.fl_Access = access;
	lock->rl_Lock.fl_Handler= gd->gd_msgport;
	lock->rl_Lock.fl_Volume = gd->gd_VolumeNode;

	//lock->rl_Block	= (pDataBlock)ptr->on_List;
	//lock->rl_Offset	= FIRSTBUFFPOS;
	//lock->rl_CPos	= 0;
	//lock->rl_Flags	= 0;
	AddHead((pList)&gd->gd_LockList, (pNode) lock);
	return lock;
}

static DOSIO SetInfo(pGD gd, struct FileInfoBlock *ivec, fs_node_t *ptr)
{

	if (ptr == NULL)
	{
		gd->gd_FileErr = ERROR_NO_MORE_ENTRIES;
		return DOSIO_FALSE;
	}
	ivec->fib_DiskKey = (INT32) ptr;

	if ((ptr->flags & FS_DIRECTORY) == FS_DIRECTORY)
	{
		ivec->fib_DirEntryType	=
		ivec->fib_EntryType		= ST_USERDIR;
	} else {
		ivec->fib_DirEntryType	=
		ivec->fib_EntryType		= ST_FILE;
	}

	Strcpy(ivec->fib_FileName, ptr->name);

	if ((ptr->flags & FS_SYMLINK) == FS_SYMLINK) KPrintF("SymLink\n");

	ivec->fib_Protection	= ptr->mask;
	ivec->fib_Size			= ptr->length;
	ivec->fib_NumBlocks		= 0;//(ptr->length >> BLKSHIFT) + 1;
	ivec->fib_Date.ds_Days	= 0;//ptr->on_Date.ds_Days;
	ivec->fib_Date.ds_Minute= 0;//ptr->on_Date.ds_Minute;
	ivec->fib_Date.ds_Tick	= 0;//ptr->on_Date.ds_Tick;
	ivec->fib_Comment[0]	= '\0';
//	if (ptr->on_Comment) Strcpy(ivec->fib_Comment, ptr->on_Comment);
	return DOSIO_TRUE;
}

/**
* --------------------- DOS FUNCTION VISIBLE
**/

/**
* Locate an object and lock it.
**/

pFSLock ext2_Locate(pGD gd, pFSLock lock, STRPTR str, INT32 mode) //pDosPacket dp)
{
//	pFSLock lock = (pFSLock)dp->dp_Arg1;
//	STRPTR  str  = (STRPTR)dp->dp_Arg2;
//	INT32   mode = (INT32)dp->dp_Arg3;
	fs_node_t *node;
#if 0
	KPrintF("Searching NodeList:\n");
	pFSLock	locki;

	ForeachNode(&gd->gd_NodeList, node)
	{
		KPrintF("Found Nodenr: %d (%x), Name: [%s], %d\n", node->inode, node, node->name, node->count);	
	}
	ForeachNode(&gd->gd_LockList, locki)
	{
		node = (fs_node_t*) locki->rl_Lock.fl_Key;
		KPrintF("Lock: %x N:%x,%s, %d \n", locki, node, node->name, node->count);
	}
	KPrintF("----------Searching NodeList\n");
#endif
//	gd->gd_FileErr = ERROR_ACTION_NOT_KNOWN;
//	return DOSIO_FALSE;

	node = LocateNode(gd, lock, str, TRUE);
	if (!node) return NULL;
	return GetLock(gd, node, mode);
}

// SAME LOCK
DOSIO ext2_SameLock(pGD gd, pDosPacket dp)
{
	fs_node_t *node;
	fs_node_t *node2;

	node = CheckLock(gd, (pFSLock) dp->dp_Arg1);
	if (node)
	{
		node2 = CheckLock(gd,(pFSLock)dp->dp_Arg2);
		if (node2 ==  node) return DOSIO_TRUE;
	}
	return DOSIO_FALSE;
}

/**
* Unlock an object.
**/

INT32 ext2_FreeLock(pGD gd, pFSLock lock)
{
//	pFSLock lock = (pFSLock)dp->dp_Arg1;
	pFSLock p;
	if (!lock) return DOSIO_TRUE;

	ForeachNode(&gd->gd_LockList, p)
	{
		if (p == lock) break;
	}

	if (!p)
	{
		gd->gd_FileErr = ERROR_INVALID_LOCK;
		return DOSIO_FALSE;
	}

	Remove((pNode)lock);
	if (lock->rl_Flags & LOCK_MODIFY)
	{
		//Implement Notify
		//Archive Bits?
	}
	lock->rl_Lock.fl_Handler= NULL;
	ReleaseNode(gd, (fs_node_t*) lock->rl_Lock.fl_Key);
	lock->rl_Lock.fl_Key	= 0;
	FreeVec(lock);
	return DOSIO_TRUE;
}

#if 0
0	Unknown
1	Regular File
2	Directory
3	Character Device
4	Block Device
5	Named pipe
6	Socket
7	Symbolic Link
#endif

/**
* Examine an Object.
**/
DOSIO ext2_ExNext(pGD gd, pDosPacket pkt, INT32 objType)
{

	struct FileInfoBlock *fib;
//	fs_node_t	*node;
	fs_node_t	*ptr;
	pFSLock		lock;

	lock = (pFSLock) pkt->dp_Arg1;
	ptr  = CheckLock(gd, lock);

//	KPrintF("ExNext(%s).. ", ptr->name);

	if (ptr == NULL) return DOSIO_FALSE;
	if (objType == 2)
	{
		KPrintF("ExAll not implemented\n");
		gd->gd_FileErr = ERROR_ACTION_NOT_KNOWN;
		return DOSIO_FALSE; // FIX: exall
	}

	fib = (struct FileInfoBlock *) pkt->dp_Arg2;
//	node = (fs_node_t*)				ivec->fib_DiskKey;

//	if (objType != 0 && node != ptr)
//	{
		//if (lock-)
//	}
	//delcount...
#if 0
	return rh_SetInfo(gd, ivec, if (object == 0) ? ptr :
					  ptr == node ? node->on_List : node->on_Next);
#endif

	if (objType == 0)
	{
		return SetInfo(gd, fib, ptr);
	} else
	{
		//KPrintF("DiskKey: %d\n", fib->fib_DiskKey);
		if (fib->fib_DiskKey == (INT32)ptr)
		{
			fib->fib_DiskKey = 2; // Ignore . & ..
			//return ext2_ReadDir(gd, ptr, fib->fib_DiskKey, fib);
		}
		return ext2_ReadDir(gd, ptr, fib->fib_DiskKey, fib);
	}
	return DOSIO_FALSE;
}

/**
* Copy a Lock
**/
INT32 ext2_Parentfh(pGD gd, pDosPacket pkt)
{
	INT32 action = pkt->dp_Action;
	pFSLock lock = (pFSLock)pkt->dp_Arg1;
	fs_node_t *node;

	node = CheckLock(gd, lock);
	if (node== NULL) { KPrintF("Node: ZERO...");gd->gd_FileErr=0; return 0;}

	if (action != ACTION_COPY_DIR_FH && action != ACTION_COPY_DIR)
	{
//		KPrintF("ParentAction:%d on node: %s..", action, node->name);
		node = ext2_ParentDirectory(gd, node);//ext2_FindDirectory(gd, node, "..");
//		KPrintF("gotNode:[%x]..", node);
	}
	if (node)
		return (INT32) GetLock(gd, node, SHARED_LOCK);
	else
		return 0;
}

static FSLock *OpenFile(GD *gd, FSLock *parent_lock, char *name, unsigned mode);

/**
* Open a file
**/
DOSIO ext2_Open(pGD gd, pDosPacket pkt, INT32 action)
{
	pFileHandle fh		= (pFileHandle) pkt->dp_Arg1;
	pFSLock		dlock	= (pFSLock) pkt->dp_Arg2;
	STRPTR		string	= (STRPTR) pkt->dp_Arg3;
	//BOOL		created = FALSE;
	//fs_node_t *node;

	//APTR		data	= NULL;
	pFSLock		lock	= NULL;

        lock = OpenFile(gd, dlock, string, action);
        if (lock) fh->fh_Arg1 = (INT32)lock;
        return lock ? DOSIO_TRUE : DOSIO_FALSE;

        /*
        lock = ext2_Locate(gd, dlock, string, SHARED_LOCK);

	if (action == 0)
	{
                if (lock) return ResizeFile(gd, lock, node, new_offset);

                lock = ext2_CreateObject(
                    gd, dlock, string, EXT2_S_IFREG | 0664, 0, EXCLUSIVE_LOCK);
                if (!lock) return DOSIO_FALSE;
	} else
	{
		if (!lock && action == 2 && gd->gd_FileErr == ERROR_OBJECT_NOT_FOUND)
		{
			KPrintF("Read Only\n");
			return DOSIO_FALSE;
		}
	}

	if (lock)
	{
		node = (fs_node_t*) lock->rl_Lock.fl_Key;
		if (created)
		{
			KPrintF("Todo\n");
		} else if ((node->flags & FS_DIRECTORY) == FS_DIRECTORY)
		{
			ext2_FreeLock(gd, lock);
			gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
			if (data) FreeVec(data);
			return DOSIO_FALSE;
		}
		if (data)
		{

		}
		lock->rl_Offset = 0;
		fh->fh_Arg1 = (INT32)lock;
		return DOSIO_TRUE;
	}
	if (data) FreeVec(data);
	return DOSIO_FALSE;
        */
}

/**
* Close a file
**/
DOSIO ext2_Close(pGD gd, pFSLock lock)
{
//	fs_node_t *node = (fs_node_t*) lock->rl_Lock.fl_Key;
	if (lock->rl_Flags & LOCK_MODIFY) {}; //DateStamp...write -> not supported ;-)
	return ext2_FreeLock(gd, lock);
}

//UINT32 ext2_ReadData(pGD gd, fs_node_t *node, UINT32 offset, UINT32 size, UINT8 *buffer)

DOSIO ext2_Read(pGD gd, pFSLock lock, UINT8* buffer, INT32 size)
{
	INT32	offset	= lock->rl_Offset;
	fs_node_t *node	= (fs_node_t *)lock->rl_Lock.fl_Key;
//	KPrintF("ext2_Read: Reading %d bytes, %d offset\n", size, offset);
	INT32	out		= ext2_ReadData(gd, node, offset, size, buffer);
//	KPrintF("Read: %d\n", out);
	lock->rl_Offset += out;
	return out;
}

// Buffers are given in 512bytes. Since we have xxx bytes per sector block, we recalculate it.Buffers

DOSIO ext2_AddBuffers(pGD gd, pDosPacket dp)
{
	INT32	buffersize = dp->dp_Arg1;

	if (gd->gd_CacheSize == buffersize/(INT32)(gd->block_size>>9))
    return DOSIO_TRUE;

	// We have a MaxValue of 300 Blocks in Cache;
	if (buffersize > 2400) buffersize = 2400;
	// Callers wants the actual size
	KPrintF("(gd->block_size >> 9) = %d, csize: %d %d\n", (gd->block_size >> 9), gd->gd_CacheSize,gd->block_size);
	if (buffersize <= 0) return gd->gd_CacheSize* (gd->block_size >> 9);
	FlushCache(gd->gd_Cache);
	DestroyCache(gd->gd_Cache);

	gd->gd_CacheSize= buffersize / (gd->block_size>>9);
	gd->gd_Cache	= CreateCache(gd->gd_request, gd->block_size, gd->gd_CacheSize);

	if (gd->gd_Cache) return DOSIO_TRUE;

	gd->gd_FileErr = ERROR_NO_FREE_STORE;
	return DOSIO_TRUE;
}

static ext2_bgdescriptor_t *FindBlockGroup(pGD gd, ext2_bgdescriptor_t *parent,
                                           BOOL for_inode, BOOL for_block) {
  ext2_bgdescriptor_t *lgroup = parent, *rgroup = parent;
  BOOL lvalid = TRUE, rvalid = FALSE;

  do {
    if (lvalid &&
        (!for_inode || lgroup->free_inodes_count) &&
        (!for_block || lgroup->free_blocks_count)) return lgroup;
    if (rvalid &&
        (!for_inode || rgroup->free_inodes_count) &&
        (!for_block || rgroup->free_blocks_count)) return rgroup;
    lvalid = --lgroup >= gd->block_groups;
    rvalid = (uint32_t)(++rgroup - gd->block_groups) < gd->block_group_count;
  } while (lvalid || rvalid);

  gd->gd_FileErr = ERROR_DISK_FULL;
  return NULL;
}

// TODO: should be moved to utility lib
static inline uint32_t DateStampToUnixTime(DateStamp *date) {
  return date->ds_Tick / 100 + date->ds_Minute * 60 + date->ds_Days * 86400;
}

static uint32_t ReserveDataBlock(GD *gd, ext2_bgdescriptor_t *group,
                                 ext2_inodetable_t *inode_entry, BOOL clear) {
  group = FindBlockGroup(gd, group, FALSE, TRUE);

  void *block_bitmap; // read block bitmap
  if (GetBlock(gd->gd_Cache, &block_bitmap, group->block_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  // find a free block index within a group
  int blbit = FindFirstUnsetBit((uint32_t*)block_bitmap, gd->block_size / 4);
  if (blbit == -1 || (uint32_t)blbit >= gd->superblock->blocks_per_group) {
    PutBlock(gd->gd_Cache, group->block_bitmap);
    gd->gd_FileErr = ERROR_DISK_CORRUPTED;
    return 0;
  }

  uint32_t block =
      (group - gd->block_groups) * gd->superblock->blocks_per_group +
      blbit + gd->superblock->first_data_block;

  if (clear) { // clear the new block
    void *data;
    if (GetBlock(gd->gd_Cache, &data, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutBlock(gd->gd_Cache, group->block_bitmap);
      return 0;
    }

    MemSet(data, 0, gd->block_size);

    if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutBlock(gd->gd_Cache, group->block_bitmap);
      return 0;
    }
  }

  // modify and save block bitmap
  SetBit(block_bitmap, blbit);
  if (MarkBlockDirty(gd->gd_Cache, group->block_bitmap) < 0 ||
      PutBlock(gd->gd_Cache, group->block_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  --group->free_blocks_count;
  --gd->superblock->free_blocks_count;
  inode_entry->blocks += gd->block_size / 512;
  return block;
}

static uint32_t AddIndirectPointer(GD *gd, ext2_bgdescriptor_t *group,
                                   ext2_inodetable_t *inode_entry,
                                   uint32_t indirect_block, unsigned index,
                                   BOOL clear_new_block) {
  void *pointers;
  if (GetBlock(gd->gd_Cache, &pointers, indirect_block) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  uint32_t *pointer = (uint32_t*)pointers + index;
  if (*pointer) {
    PutBlock(gd->gd_Cache, indirect_block);
    return *pointer;
  }

  uint32_t block = ReserveDataBlock(gd, group, inode_entry, clear_new_block);
  if (!block) {
    gd->gd_FileErr = IoErr();
    PutBlock(gd->gd_Cache, indirect_block);
    return 0;
  }

  *pointer = block;

  if (MarkBlockDirty(gd->gd_Cache, indirect_block) < 0 ||
      PutBlock(gd->gd_Cache, indirect_block) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  return block;
}

static uint32_t AddInodeDataBlock(GD *gd, ext2_bgdescriptor_t *group,
                                  ext2_inodetable_t *inode_entry,
                                  unsigned block_index) {
  unsigned bptrs = gd->pointers_per_block;
  uint32_t block;

  if (block_index < EXT2_DIRECT_BLOCKS) {
    if ((block = inode_entry->block[block_index])) return block;
    if (!(block = ReserveDataBlock(gd, group, inode_entry, FALSE))) return 0;
    inode_entry->block[block_index] = block;
  }
  else if ((block_index -= EXT2_DIRECT_BLOCKS) < bptrs) {
    // make sure the single indirect block exists
    if (!(block = inode_entry->block[EXT2_DIRECT_BLOCKS])) {
      if (!(block = ReserveDataBlock(gd, group, inode_entry, TRUE))) return 0;
      inode_entry->block[EXT2_DIRECT_BLOCKS] = block;
    }

    // reserve and assign data block
    if (!(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index, FALSE)))
      return 0;
  }
  else if ((block_index -= bptrs) < bptrs * bptrs) {
    // make sure the double indirect block exists
    if (!(block = inode_entry->block[EXT2_DIRECT_BLOCKS + 1])) {
      if (!(block = ReserveDataBlock(gd, group, inode_entry, TRUE))) return 0;
      inode_entry->block[EXT2_DIRECT_BLOCKS + 1] = block;
    }

    // reserve and assign indirect and data blocks
    if (!(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index / bptrs, TRUE)) ||
        !(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index % bptrs, FALSE)))
      return 0;
  }
  else if ((block_index -= bptrs * bptrs) < bptrs * bptrs * bptrs) {
    // make sure the tripple indirect block exists
    if (!(block = inode_entry->block[EXT2_DIRECT_BLOCKS + 2])) {
      if (!(block = ReserveDataBlock(gd, group, inode_entry, TRUE))) return 0;
      inode_entry->block[EXT2_DIRECT_BLOCKS + 2] = block;
    }

    // reserve and assign 2 indirect and data blocks
    unsigned block_index2 = block_index % bptrs;
    if (!(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index / bptrs, TRUE)) ||
        !(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index2 / bptrs, TRUE)) ||
        !(block = AddIndirectPointer(
            gd, group, inode_entry, block, block_index2 % bptrs, FALSE)))
      return 0;
  }
  else {
    gd->gd_FileErr = ERROR_OBJECT_TOO_LARGE;
    return 0;
  }

  return block;
}

// TODO: should be moved to utility lib
static inline unsigned BlocksForSize(unsigned block_size, unsigned size) {
  return (size + block_size - 1) / block_size;
}

static inline unsigned DirEntrySize(ext2_dir_t *dir_entry) {
  unsigned size = sizeof(ext2_dir_t) + dir_entry->name_len - 1;
  return BlocksForSize(4, size) * 4;
}

static void FillDirEntry(GD *gd, uint32_t inode, char *name,
                         ext2_dir_t *dir_entry) {
  unsigned length = Strlen(name);

  dir_entry->inode = inode;
  dir_entry->name_len = length;
  dir_entry->file_type = EXT2_FT_UNKNOWN;

  memcpy(&dir_entry->name, name, length);
  MemSet(&dir_entry->name + length, 0, BlocksForSize(4, length) * 4 - length);

  dir_entry->rec_len = DirEntrySize(dir_entry);
}

static BOOL FillInodeEntry(GD *gd, ext2_bgdescriptor_t *group,
                           uint16_t mode, uint32_t flags, uint32_t inode,
                           uint32_t parent_inode,
                           ext2_inodetable_t *inode_entry) {
  DateStamp sys_time;
  DateStamp(&sys_time); // get current time
  uint32_t unix_time = DateStampToUnixTime(&sys_time);

  MemSet(inode_entry, 0, gd->superblock->inode_size);
  inode_entry->mode = mode;
  inode_entry->atime = unix_time;
  inode_entry->ctime = unix_time;
  inode_entry->mtime = unix_time;
  inode_entry->links_count = 1;
  inode_entry->flags = flags;

  if (mode & EXT2_S_IFDIR) {
    void *dir_entries;
    uint32_t block = AddInodeDataBlock(gd, group, inode_entry, 0);
    if (!block || GetBlock(gd->gd_Cache, &dir_entries, block) < 0) {
      gd->gd_FileErr = IoErr();
      return FALSE;
    }

    ext2_dir_t *de = (ext2_dir_t*)dir_entries;
    FillDirEntry(gd, inode, ".", de);
    de = (ext2_dir_t*)((char*)de + de->rec_len);
    FillDirEntry(gd, parent_inode, "..", de);
    de->rec_len = gd->block_size - ((char*)de - (char*)dir_entries);

    if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      return FALSE;
    }

    inode_entry->size = gd->block_size;
    ++inode_entry->links_count;
    ++group->used_dirs_count;
  }

  --group->free_inodes_count;
  --gd->superblock->free_inodes_count;
  return TRUE;
}

static uint32_t AddInode(GD *gd, ext2_bgdescriptor_t *group,
                         uint16_t mode, uint32_t flags, uint32_t parent_inode,
                         ext2_inodetable_t *inode_entry) {
  void *inode_bitmap; // read inode bitmap
  if (GetBlock(gd->gd_Cache, &inode_bitmap, group->inode_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  // find a free inode index within a group
  int inbit = FindFirstUnsetBit((uint32_t*)inode_bitmap, gd->block_size / 4);
  if (inbit == -1 || (uint32_t)inbit >= gd->inodes_per_group) {
    PutBlock(gd->gd_Cache, group->inode_bitmap);
    gd->gd_FileErr = ERROR_DISK_CORRUPTED;
    return 0;
  }

  void *inode_entries; // read block with a free inode
  unsigned inode_size = gd->superblock->inode_size;
  unsigned entries_per_block = gd->block_size / inode_size;
  uint32_t inode_block = group->inode_table + inbit / entries_per_block;
  if (GetBlock(gd->gd_Cache, &inode_entries, inode_block) < 0) {
    gd->gd_FileErr = IoErr();
    PutBlock(gd->gd_Cache, group->inode_bitmap);
    return 0;
  }

  uint32_t inode = (group-gd->block_groups) * gd->inodes_per_group + inbit + 1;
  if (!FillInodeEntry(
          gd, group, mode, flags, inode, parent_inode, inode_entry)) {
    PutBlock(gd->gd_Cache, inode_block);
    PutBlock(gd->gd_Cache, group->inode_bitmap);
    return 0;
  }

  // copy inode entry into the block
  memcpy(inode_entries + (inbit % entries_per_block) * inode_size,
         inode_entry, inode_size);
  if (MarkBlockDirty(gd->gd_Cache, inode_block) < 0 ||
      PutBlock(gd->gd_Cache, inode_block) < 0) {
    gd->gd_FileErr = IoErr();
    PutBlock(gd->gd_Cache, group->inode_bitmap);
    return 0;
  }

  // modify and save inode bitmap
  SetBit(inode_bitmap, inbit);
  if (MarkBlockDirty(gd->gd_Cache, group->inode_bitmap) < 0 ||
      PutBlock(gd->gd_Cache, group->inode_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  return inode;
}

static inline ext2_bgdescriptor_t *GetInodeGroup(GD *gd, uint32_t inode) {
  unsigned gid = (inode - 1) / gd->inodes_per_group;
  return gd->block_groups + gid;
}

static BOOL GetInodeEntry(GD *gd, uint32_t inode,
                          uint32_t *block, ext2_inodetable_t **inode_entry) {
  ext2_bgdescriptor_t *group = GetInodeGroup(gd, inode);
  --inode, inode %= gd->inodes_per_group;

  void *inode_entries;
  unsigned entries_per_block = gd->block_size / gd->superblock->inode_size;
  *block = group->inode_table + inode / entries_per_block;
  if (GetBlock(gd->gd_Cache, &inode_entries, *block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  *inode_entry = (ext2_inodetable_t*)inode_entries + inode % entries_per_block;
  return TRUE;
}

static BOOL PutInodeEntry(GD *gd, uint32_t block,
                          ext2_inodetable_t *inode_entry, BOOL modified) {
  DateStamp sys_time;
  DateStamp(&sys_time); // get current time
  uint32_t unix_time = DateStampToUnixTime(&sys_time);

  inode_entry->atime = unix_time;
  if (modified) inode_entry->mtime = unix_time;

  if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
      PutBlock(gd->gd_Cache, block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  return TRUE;
}

static BOOL AddDirEntry(GD *gd, ext2_bgdescriptor_t *group, fs_node_t *parent,
                        ext2_dir_t *dir_entry, uint16_t mode) {
  uint16_t rec_len = dir_entry->rec_len;
  void *dir_entries;

  uint32_t inode_block;
  ext2_inodetable_t *inode_entry;
  if (!GetInodeEntry(gd, parent->inode, &inode_block, &inode_entry))
    return FALSE;

  // try to insert the given dir entry to a possible hole
  BOOL inserted = FALSE;
  unsigned inode_blocks = BlocksForSize(gd->block_size, inode_entry->size);
  for (unsigned i = 0; !inserted && i < inode_blocks; ++i) {
    uint32_t block = ext2_GetBlockNumber(gd, inode_entry, i);
    if (GetBlock(gd->gd_Cache, &dir_entries, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return FALSE;
    }

    for (ext2_dir_t *de = dir_entries;
         (unsigned)((char*)de - (char*)dir_entries) < gd->block_size;
         de = (ext2_dir_t*)((char*)de + de->rec_len)) {
      uint16_t size = DirEntrySize(de);
      if (de->rec_len - size >= dir_entry->rec_len) {
        dir_entry->rec_len = de->rec_len - size;
        memcpy((char*)de + size, dir_entry, rec_len);
        de->rec_len = size;
        inserted = TRUE;
        break;
      }
    }

    if ((inserted && MarkBlockDirty(gd->gd_Cache, block) < 0) ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return FALSE;
    }
  }

  if (!inserted) { // no place to insert - will add an additional block
    uint32_t block = AddInodeDataBlock(gd, group, inode_entry, inode_blocks);
    if (!block || GetBlock(gd->gd_Cache, &dir_entries, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return FALSE;
    }

    dir_entry->rec_len = gd->block_size;
    memcpy(dir_entries, dir_entry, rec_len);

    if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return FALSE;
    }

    inode_entry->size += gd->block_size;
  }

  if (mode & EXT2_S_IFDIR) ++inode_entry->links_count;
  if(!PutInodeEntry(gd, inode_block, inode_entry, TRUE)) return FALSE;
  return TRUE;
}

static BOOL WriteLowLevel(GD *gd, void *data, uint64_t lba, unsigned sectors) {
  gd->gd_request->io_Command = CMD_WRITE;
  gd->gd_request->io_Length = sectors;
  gd->gd_request->io_Data = data;
  gd->gd_request->io_Offset = lba;
  DoIO(gd->gd_request);

  if (gd->gd_request->io_Error) {
    gd->gd_FileErr = ERROR_IO_FAILURE;
    return FALSE;
  }

  return TRUE;
}

static BOOL WriteBlock(GD *gd, void *data, uint32_t block,
                       unsigned offset, unsigned size) {
  void *block_data;
  if (ZeroBlock(gd->gd_Cache, block) < 0 ||
      GetBlock(gd->gd_Cache, &block_data, block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  memcpy(block_data + offset, data, size);

  if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
      PutBlock(gd->gd_Cache, block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  return TRUE;
}

BOOL BackupSbAndBgdt(GD *gd) {
  BOOL sparse = gd->superblock->feature_ro_compat &
      EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;

  // enumerate all groups containing sb and bgdt backups
  for (unsigned i = 0, group = 0; group < gd->block_group_count;
       ++i, group = sparse ? ext2_sparse_block_groups[i] : i) {
    uint32_t group_block = gd->superblock->first_data_block +
        gd->superblock->blocks_per_group * group;

    // write superblock
    if (gd->block_size == 1024 || group)
      WriteBlock(
          gd, &gd->superblock, group_block, 0, sizeof(ext2_superblock_t));
    else
      // disk cache treats 0 as illegal block number
      WriteLowLevel(gd, gd->superblock, 2, 2);

    // write block group descriptor table
    ext2_bgdescriptor_t *group = gd->block_groups;
    unsigned bgd_per_block = gd->block_size / sizeof(ext2_bgdescriptor_t);
    for (unsigned i = 0; group < gd->block_groups + gd->block_group_count;
         ++i, group += bgd_per_block)
      WriteBlock(gd, group, group_block + 1 + i, 0, gd->block_size);
  }

  return TRUE;
}

static BOOL IsNameValid(GD *gd, fs_node_t *parent, char *name) {
  if (Strlen(name) > 255) {
    gd->gd_FileErr = ERROR_FILENAME_TOO_LONG;
    return FALSE;
  }

  if (Strchr(name, '/') || Strchr(name, ':')) {
    gd->gd_FileErr = ERROR_ILLEGAL_FILENAME;
    return FALSE;
  }

  if (ext2_FindDirectory(gd, parent, name)) {
    gd->gd_FileErr = ERROR_NON_UNIQUE_FILENAME;
    return FALSE;
  }

  return TRUE;
}

struct DirEntry {
  ext2_dir_t header;
  char name[254];
} __attribute__((packed));

FSLock *ext2_CreateObject(GD *gd, FSLock *lock, char *name,
                          uint16_t ext2_mode, uint32_t ext2_flags,
                          int32_t lock_mode) {
  fs_node_t *node = CheckLock(gd, lock);
KPrintF("CO: node: %x\n", node);
//if (gd->gd_Debug >4) for(;;);

  if (!node) return NULL;
  char basename[256];
  fs_node_t *parent = FindDir(gd, node, name, basename);
  if (!parent) return NULL;

  if (!IsNameValid(gd, parent, basename)) return NULL;

  ext2_bgdescriptor_t *group =
    FindBlockGroup(gd, GetInodeGroup(gd, parent->inode), TRUE, TRUE);
  if (!group) return NULL;

  ext2_inodetable_t inode_entry;
  uint32_t inode = AddInode(
    gd, group, ext2_mode, ext2_flags, parent->inode, &inode_entry);
  if (!inode) return NULL;

  struct DirEntry dir_entry;
  FillDirEntry(gd, inode, basename, &dir_entry.header);
  if (!AddDirEntry(gd, group, parent, &dir_entry.header, ext2_mode))
    return NULL;

  ext2_SetTimeDirty(gd);

  node = ext2_NodeFromFile(gd, &inode_entry, &dir_entry.header);
  if (!node) return NULL;

  return GetLock(gd, node, lock_mode);
}

unsigned ext2_Write(GD *gd, FSLock *lock, void *buffer, unsigned size) {
  fs_node_t *node = (fs_node_t*)lock->rl_Lock.fl_Key;
  if (lock->rl_Offset > node->length) {
    KPrintF("Internal error: bad EXT2 file pointer\n");
    gd->gd_FileErr = ERROR_SEEK_ERROR;
    return 0;
  }

  uint32_t inode_block;
  ext2_inodetable_t *inode_entry;
  if (!GetInodeEntry(gd, node->inode, &inode_block, &inode_entry)) return 0;

  uint32_t offset = lock->rl_Offset + size; // TODO: better to use uint64_t
  unsigned inode_blocks = BlocksForSize(gd->block_size, inode_entry->size);

  for (uint32_t block_index = lock->rl_Offset / gd->block_size;
       block_index * gd->block_size < offset; ++block_index) {

    uint32_t block = block_index < inode_blocks ?
        ext2_GetBlockNumber(gd, inode_entry, block_index) :
        AddInodeDataBlock(gd, GetInodeGroup(gd, node->inode),
                          inode_entry, block_index);

    void *data;
    if (!block || GetBlock(gd->gd_Cache, &data, block) < 0) {
      gd->gd_FileErr = IoErr();
      break;
    }

    unsigned from = lock->rl_Offset % gd->block_size;
    unsigned to = (block_index + 1) * gd->block_size < offset ?
        gd->block_size : offset % gd->block_size;
    if (!to) to = gd->block_size;
    unsigned delta = to - from;
    memcpy(data + from, buffer, delta);

    if (MarkBlockDirty(gd->gd_Cache, block) < 0 ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      break;
    }

    lock->rl_Offset += delta;
    buffer += delta;
  }

  if (lock->rl_Offset > inode_entry->size) {
    inode_entry->size = lock->rl_Offset;
    node->length = lock->rl_Offset;
  }

  if(!PutInodeEntry(gd, inode_block, inode_entry, TRUE)) return 0;

  ext2_SetTimeDirty(gd);
  return size - (offset - lock->rl_Offset);
}

static BOOL FreeDataBlock(GD *gd, ext2_bgdescriptor_t *group,
                          ext2_inodetable_t *inode_entry,
                          unsigned block_index) {
  void *block_bitmap;
  if (GetBlock(gd->gd_Cache, &block_bitmap, group->block_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  unsigned blbit = (block_index - gd->superblock->first_data_block) -
      (group - gd->block_groups) * gd->superblock->blocks_per_group;

  UnsetBit(block_bitmap, blbit);

  if (MarkBlockDirty(gd->gd_Cache, group->block_bitmap) < 0 ||
      PutBlock(gd->gd_Cache, group->block_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  ++group->free_blocks_count;
  ++gd->superblock->free_blocks_count;
  inode_entry->blocks -= gd->block_size / 512;
  return TRUE;
}

static BOOL RemoveIndirectPointer(GD *gd, ext2_bgdescriptor_t *group,
                                  ext2_inodetable_t *inode_entry,
                                  uint32_t *indirect_block, unsigned level,
                                  unsigned index) {
  if (!*indirect_block) return TRUE;
  if (!level) goto do_free;

  void *block_data;
  if (GetBlock(gd->gd_Cache, &block_data, *indirect_block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  unsigned bptrs = gd->pointers_per_block;
  uint32_t *block = (uint32_t*)block_data + index / bptrs, before = *block;
  if (!RemoveIndirectPointer(gd, group, inode_entry,
                             block, level - 1, index % bptrs)) return FALSE;

  BOOL free_block = *block != before;
  if (free_block) {
    if (MarkBlockDirty(gd->gd_Cache, *indirect_block) < 0) {
      gd->gd_FileErr = IoErr();
      return FALSE;
    }

    for (unsigned i = 0; i < bptrs; ++i)
      if (((uint32_t*)block_data)[i]) {
        free_block = FALSE;
        break;
      }
  }

  if (PutBlock(gd->gd_Cache, *indirect_block) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  if (free_block) {
 do_free:
    if (!FreeDataBlock(gd, group, inode_entry, *indirect_block)) return FALSE;
    *indirect_block = 0;
  }

  return TRUE;
}

static BOOL RemoveInodeDataBlock(GD *gd, ext2_bgdescriptor_t *group,
                                 ext2_inodetable_t *inode_entry,
                                 unsigned block_index) {
  unsigned bptrs = gd->pointers_per_block;

  if (block_index < EXT2_DIRECT_BLOCKS) {
    uint32_t *block = &inode_entry->block[block_index];
    return RemoveIndirectPointer(
        gd, group, inode_entry, block, 0, block_index);
  }
  else if ((block_index -= EXT2_DIRECT_BLOCKS) < bptrs) {
    uint32_t *block = &inode_entry->block[EXT2_DIRECT_BLOCKS];
    return RemoveIndirectPointer(
        gd, group, inode_entry, block, 1, block_index);
  }
  else if ((block_index -= bptrs) < bptrs * bptrs) {
    uint32_t *block = &inode_entry->block[EXT2_DIRECT_BLOCKS + 1];
    return RemoveIndirectPointer(
        gd, group, inode_entry, block, 2, block_index);
  }
  else if ((block_index -= bptrs * bptrs) < bptrs * bptrs * bptrs) {
    uint32_t *block = &inode_entry->block[EXT2_DIRECT_BLOCKS + 2];
    return RemoveIndirectPointer(
        gd, group, inode_entry, block, 3, block_index);
  }
  else {
    return TRUE;
  }
}

static int32_t ResizeObject(GD *gd, FSLock *lock,
                            fs_node_t *node, uint32_t size) {
  FSLock *l;
  ForeachNode(&gd->gd_LockList, l) {
    if (l->rl_Lock.fl_Key == lock->rl_Lock.fl_Key && l != lock &&
        l->rl_Offset < size) size = l->rl_Offset;
  }

  ext2_bgdescriptor_t *group = GetInodeGroup(gd, node->inode);
  unsigned before_blocks = BlocksForSize(gd->block_size, node->length);
  unsigned after_blocks = BlocksForSize(gd->block_size, size);

  uint32_t inode_block;
  ext2_inodetable_t *inode_entry;
  if (!GetInodeEntry(gd, node->inode, &inode_block, &inode_entry)) return -1;

  for (unsigned i = before_blocks; i < after_blocks; ++i)
    if (!AddInodeDataBlock(gd, group, inode_entry, i)) {
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return -1;
    }
  for (unsigned i = after_blocks; i < before_blocks; ++i)
    if (!RemoveInodeDataBlock(gd, group, inode_entry, i)) {
      PutInodeEntry(gd, inode_block, inode_entry, FALSE);
      return -1;
    }

  inode_entry->size = size;
  node->length = size;

  if(!PutInodeEntry(gd, inode_block, inode_entry, TRUE)) return -1;

  if (lock->rl_Offset > size) lock->rl_Offset = size;
  return size;
}

int32_t ext2_Seek(GD *gd, FSLock *lock,
                  int32_t offset, int origin, BOOL resize) {
  fs_node_t *node = (fs_node_t*)lock->rl_Lock.fl_Key;
  if (!(node->flags & FS_FILE)) goto error;

  uint32_t new_offset;
  switch (origin) {
    case OFFSET_CURRENT:
      new_offset = lock->rl_Offset + offset;
      break;
    case OFFSET_BEGINNING:
      new_offset = offset;
      break;
    case OFFSET_END:
      new_offset = node->length - offset;
      break;
    default:
      goto error;
  }

  if (resize) {
    int32_t result = ResizeObject(gd, lock, node, new_offset);
    ext2_SetTimeDirty(gd);
    return result;
  }

  if (new_offset <= node->length) return lock->rl_Offset = new_offset;

error:
  gd->gd_FileErr = ERROR_SEEK_ERROR;
  return -1;
}

static FSLock *OpenFile(GD *gd, FSLock *lock,
                        char *name, unsigned mode) {
  gd->gd_Debug++;
  KPrintF("Ext2:Open %s, %x, %d\n", name, lock, mode);
  
  FSLock *lock2 = ext2_Locate(gd, lock, name, SHARED_LOCK);
  fs_node_t *node = lock2 ? (fs_node_t*)lock2->rl_Lock.fl_Key : NULL;

  if (node && node->flags & FS_DIRECTORY) {
    gd->gd_FileErr = ERROR_OBJECT_WRONG_TYPE;
    return NULL;
  }

  switch (mode) {
  case MODE_OLDFILE:
    if (!lock2) {
      gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
      return NULL;
    }
    break;
  case MODE_READWRITE:
    if (!lock2) {
      lock2 = ext2_CreateObject(
          gd, lock, name, EXT2_S_IFREG | 0664, 0, EXCLUSIVE_LOCK);
    }
    break;
  case MODE_NEWFILE:
    if (lock2) {
      ResizeObject(gd, lock2, node, 0);
    } else {
      lock2 = ext2_CreateObject(
          gd, lock, name, EXT2_S_IFREG | 0664, 0, EXCLUSIVE_LOCK);
    }
    break;
  };

  return lock2;
}

static BOOL IsDirEmpty(GD *gd, fs_node_t *node) {
  uint32_t inode_block;
  ext2_inodetable_t *inode_entry;
  if (!GetInodeEntry(gd, node->inode, &inode_block, &inode_entry))
    return FALSE;

  BOOL empty = ext2_DirEntry(gd, inode_entry, 2) == NULL;

  if (!PutInodeEntry(gd, inode_block, inode_entry, TRUE)) return FALSE;
  return empty;
}

static BOOL RemoveDirEntry(GD *gd, ext2_bgdescriptor_t *group,
                           ext2_inodetable_t *inode_entry, fs_node_t *node) {
  BOOL removed = FALSE;
  unsigned inode_blocks = BlocksForSize(gd->block_size, inode_entry->size);
  for (unsigned i = 0; !removed && i < inode_blocks; ++i) {
    void *dir_entries;
    uint32_t block = ext2_GetBlockNumber(gd, inode_entry, i);
    if (GetBlock(gd->gd_Cache, &dir_entries, block) < 0) {
      gd->gd_FileErr = IoErr();
      return FALSE;
    }

    for (ext2_dir_t *pde = NULL, *de = dir_entries;
         (unsigned)((char*)de - (char*)dir_entries) < gd->block_size;
         pde = de, de = (ext2_dir_t*)((char*)de + de->rec_len))
      if (pde && de->inode == node->inode) {
        // just extend the previous entry
        pde->rec_len += de->rec_len;
        removed = TRUE;
        break;
      }
      else if (pde->inode == node->inode && de) {
        // move the current entry into place of the previous
        uint16_t rec_len = pde->rec_len;
        memcpy(pde, de, DirEntrySize(de));
        pde->rec_len += rec_len;
        removed = TRUE;
        break;
      }
      else if (!pde && de->inode == node->inode &&
               de->rec_len == gd->block_size) {
        // to remove the last element remove the whole block
        if (i < inode_blocks - 1) {
          // this block is not last so copy data from the last one to it
          uint32_t last_block =
            ext2_GetBlockNumber(gd, inode_entry, inode_blocks - 1);

          void *dir_entries2;
          if (GetBlock(gd->gd_Cache, &dir_entries2, last_block) < 0) {
            gd->gd_FileErr = IoErr();
            PutBlock(gd->gd_Cache, block);
            return FALSE;
          }

          memcpy(dir_entries2, dir_entries, gd->block_size);

          if (PutBlock(gd->gd_Cache, last_block) < 0) {
            gd->gd_FileErr = IoErr();
            PutBlock(gd->gd_Cache, block);
            return FALSE;
          }
        }

        // remove the last block
        if (!RemoveInodeDataBlock(gd, group, inode_entry, inode_blocks - 1)) {
            PutBlock(gd->gd_Cache, block);
            return FALSE;
        }

        removed = TRUE;
        break;
      }

    if ((removed && MarkBlockDirty(gd->gd_Cache, block) < 0) ||
        PutBlock(gd->gd_Cache, block) < 0) {
      gd->gd_FileErr = IoErr();
      return FALSE;
    }
  }

  --inode_entry->links_count;
  return TRUE;
}

static BOOL RemoveInode(GD *gd, FSLock *lock, fs_node_t *node) {
  if (ResizeObject(gd, lock, node, 0) == -1) return FALSE;

  void *inode_bitmap;
  ext2_bgdescriptor_t *group = GetInodeGroup(gd, node->inode);
  if (GetBlock(gd->gd_Cache, &inode_bitmap, group->inode_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return 0;
  }

  unsigned inbit = node->inode - 1 -
      (group - gd->block_groups) * gd->inodes_per_group;
  UnsetBit(inode_bitmap, inbit);

  if (MarkBlockDirty(gd->gd_Cache, group->inode_bitmap) < 0 ||
      PutBlock(gd->gd_Cache, group->inode_bitmap) < 0) {
    gd->gd_FileErr = IoErr();
    return FALSE;
  }

  if (node->flags & FS_DIRECTORY) --group->used_dirs_count;
  ++group->free_inodes_count;
  ++gd->superblock->free_inodes_count;
  return TRUE;
}

BOOL ext2_DeleteObject(GD *gd, FSLock *lock, char *name) {
  fs_node_t *node = CheckLock(gd, lock);
  if (!node) return NULL;
  char basename[256];
  fs_node_t *parent = FindDir(gd, node, name, basename);
  if (!parent) return NULL;

  node = FindNode(gd, parent, basename, FALSE);
  if (!node) {
    gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
    return FALSE;
  }

  lock = GetLock(gd, node, EXCLUSIVE_LOCK);
  if (!lock) {
    gd->gd_FileErr = ERROR_OBJECT_IN_USE;
    return FALSE;
  }

  uint32_t inode_block;
  ext2_inodetable_t *inode_entry;
  if (!GetInodeEntry(gd, parent->inode, &inode_block, &inode_entry)) {
    ext2_FreeLock(gd, lock);
    return FALSE;
  }

  if ((node->flags & FS_DIRECTORY) && !IsDirEmpty(gd, node)) {
    PutInodeEntry(gd, inode_block, inode_entry, FALSE);
    ext2_FreeLock(gd, lock);
    if (!gd->gd_FileErr) gd->gd_FileErr = ERROR_DIRECTORY_NOT_EMPTY;
    return FALSE;
  }

  ext2_bgdescriptor_t *group = GetInodeGroup(gd, parent->inode);
  if (!RemoveDirEntry(gd, group, inode_entry, node)) {
    PutInodeEntry(gd, inode_block, inode_entry, FALSE);
    ext2_FreeLock(gd, lock);
    return FALSE;
  }

  BOOL remove_inode = !inode_entry->links_count;
  if (!PutInodeEntry(gd, inode_block, inode_entry, TRUE) ||
      (remove_inode && !RemoveInode(gd, lock, node))) {
    ext2_FreeLock(gd, lock);
    return FALSE;
  }

  ext2_FreeLock(gd, lock);
  ext2_SetTimeDirty(gd);
  return TRUE;
}
