/**
* File: /ext2_functions．c
* User: cycl0ne
* Date: 2014-10-09
* Time: 08:13 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
* http://cs.smith.edu/~nhowe/Teaching/csc262/oldlabs/ext2.html
**/

#include "ext2_handler.h"

static fs_node_t *search_node(pGD gd, UINT32 inode_nr)
{
	fs_node_t *node;
	ForeachNode(&gd->gd_NodeList, node)
	{
		if (node->inode == inode_nr) return node;
	}
	return NULL;
}

fs_node_t *ext2_NodeFromFile(pGD gd, ext2_inodetable_t *inode, ext2_dir_t *direntry)
{
	if (!direntry) return NULL;
	if (!inode)	return NULL;
	
	fs_node_t *fnode = search_node(gd, direntry->inode);
	
	if (fnode) 
	{
		//fnode->count++;
		//KPrintF("----Found Node: %x %d\n", fnode, fnode->count);
		return fnode;
	}
	
	fnode = AllocVec(sizeof(fs_node_t), MEMF_PUBLIC|MEMF_CLEAR);
	if (!fnode) return NULL;

	/* Information from the direntry */
	fnode->gd = (void *)gd;
	fnode->inode = direntry->inode;
	memcpy(&fnode->name, &direntry->name, direntry->name_len);
	fnode->name[direntry->name_len] = '\0';

	/* Information from the inode */
	fnode->uid = inode->uid;
	fnode->gid = inode->gid;
	fnode->length = inode->size;
	fnode->mask = inode->mode & 0xFFF;
	fnode->nlink = inode->links_count;
	/* File Flags */
	fnode->flags = 0;

	if ((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) {
		fnode->flags |= FS_FILE;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) {
		fnode->flags |= FS_DIRECTORY;
	}
	if ((inode->mode & EXT2_S_IFBLK) == EXT2_S_IFBLK) {
		fnode->flags |= FS_BLOCKDEVICE;
	}
	if ((inode->mode & EXT2_S_IFCHR) == EXT2_S_IFCHR) {
		fnode->flags |= FS_CHARDEVICE;
	}
	if ((inode->mode & EXT2_S_IFIFO) == EXT2_S_IFIFO) {
		fnode->flags |= FS_PIPE;
	}
	if ((inode->mode & EXT2_S_IFLNK) == EXT2_S_IFLNK) {
		fnode->flags |= FS_SYMLINK;
	}

	fnode->atime   = inode->atime;
	fnode->mtime   = inode->mtime;
	fnode->ctime   = inode->ctime;
//	KPrintF("%s [%x]file a/m/c times are %d/%d/%d\n", fnode->name,fnode,fnode->atime, fnode->mtime, fnode->ctime);

	fnode->count = 0;
	AddHead(&gd->gd_NodeList, (pNode)fnode);
#if 0	
	fs_node_t *nodetmp = NULL;
	nodetmp = (fs_node_t *) GetHead(&gd->gd_NodeList);
	KPrintF("Two %d %x Head: %x %s\n", sizeof(fs_node_t), RN, GetHead(&gd->gd_NodeList), nodetmp->name);

	ForeachNode(&gd->gd_NodeList, nodetmp)
	{
		KPrintF("----------Found Nodenr: %d (%x), Name:   [%s]\n", nodetmp->inode, nodetmp, nodetmp->name);	
	}
#endif
	return fnode;
}

static UINT32 getParentNodefromBlock(pGD gd, UINT8* block)
{
	UINT32		dir_offset	= 0;	
	ext2_dir_t *parent		= NULL;

	// Get "."
	parent = (ext2_dir_t *)((UINT32)block + dir_offset);
	dir_offset	+= parent->rec_len;
	// Get ".."
	parent = (ext2_dir_t *)((UINT32)block + dir_offset);
	return parent->inode;
}

#if 0
static ext2_inodetable_t *GetParentInode(pGD gd, fs_node_t *node)
{
	ext2_inodetable_t	*inode 		= ext2_GetINode(gd, node->inode);
	
}

ROOT	(2)
 |-.	(2)
 |-..	(2)
 |-c	(3)
 |-s	(4)
 |-libs	(5)
 |-test	(6)
     |-.	(6)
	 |-..	(2)
	 |-claus(7)
		 |-.	(7) <---When iam here (in dir Claus)
		 |-..	(6)
		 |-deep	(8)

.. -> 6 (to get the name of 6 -> .. and then search for 6)

#endif
	
fs_node_t *ext2_ParentDirectory(pGD gd, fs_node_t *node)
{
	if (node == RN)
	{
		//KPrintF("return RN..");
		//gd->gd_FileErr = ERROR_OBJECT_NOT_FOUND;
		return NULL;
	}

	UINT8				block_nr	= 0;
	UINT32				dir_offset	= 0;
	UINT32				total_offset= 0;
	ext2_dir_t			*direntry	= NULL;

	ext2_inodetable_t	*inode	 	= ext2_GetINode(gd, node->inode);
	UINT8				*block		= ext2_GetBlockINode(gd, inode, block_nr);
	UINT32				parent		= getParentNodefromBlock(gd, block);
	
	ext2_ReleaseBlockINode(gd);
	ext2_ReleaseINode(gd, node->inode);

	inode	= ext2_GetINode(gd, parent);
	block	= ext2_GetBlockINode(gd, inode, block_nr);

	UINT32	parentparent = getParentNodefromBlock(gd, block);

	ext2_ReleaseBlockINode(gd);
	ext2_ReleaseINode(gd, parent);

	dir_offset	= 0;
	total_offset= 0;

	inode 		= ext2_GetINode(gd, parentparent);
	block 		= ext2_GetBlockINode(gd, inode, block_nr);
	
//	KPrintF("\n----Searching Inode: %d\n", parent);

	while (total_offset < inode->size)
	{
		if (dir_offset >= gd->block_size)
		{
			ext2_ReleaseBlockINode(gd);
			block_nr++;
			dir_offset	-= gd->block_size;
			block = ext2_GetBlockINode(gd, inode, block_nr);
		}
		
		ext2_dir_t *d_ent = (ext2_dir_t *)((UINT32)block + dir_offset);

		if ((parent != d_ent->inode) )
		{
//			KPrintF("S:%d, F:%d..", parent, d_ent->inode);
			dir_offset	+= d_ent->rec_len;
			total_offset+= d_ent->rec_len;
			continue;
		} else
		{
//			KPrintF("Found:%d..", d_ent->inode);
			direntry = AllocVec(d_ent->rec_len, MEMF_PUBLIC);
			if (direntry) memcpy(direntry, d_ent, d_ent->rec_len);
			break;
		}
		
		dir_offset	+= d_ent->rec_len;
		total_offset+= d_ent->rec_len;
	}

	ext2_ReleaseINode(gd, parentparent);
	ext2_ReleaseBlockINode(gd);			
	
	if (direntry) 
	{	
		inode = ext2_GetINode(gd, direntry->inode);

		if (inode)
		{
//			KPrintF("nff..");
			fs_node_t *fnode = ext2_NodeFromFile(gd, inode, direntry);
			
			if (!fnode) KPrintF("ERROR: Couldnt copy into buffer (FindDir)\n");
			
			ext2_ReleaseINode(gd, direntry->inode);
			FreeVec(direntry);
			return fnode;
		}
		FreeVec(direntry);
	}
	return NULL;
}

ext2_dir_t * ext2_DirEntry(pGD gd, ext2_inodetable_t * inode, UINT32 index) 
{
	UINT8 *block;
	UINT8  block_nr		= 0;
	UINT32 dir_offset	= 0;
	UINT32 total_offset = 0;
	UINT32 dir_index	= 0;
	
	block = ext2_GetBlockINode(gd, inode, block_nr);
//	KPrintF("DE: GetBlockInode: %d %d\n", block_nr, gd->gd_GBIN_RB);
	if (!block) KPrintF("couldnt load block %x, %d\n", inode, block_nr);

	//KPrintF("inode_size = %d\n", inode->size);
	
	while (total_offset < inode->size && dir_index <= index) 
	{
		ext2_dir_t *d_ent = (ext2_dir_t *)((UINT32)block + dir_offset);

		if (dir_index == index) 
		{
			ext2_dir_t *direntry = AllocVec(d_ent->rec_len, MEMF_PUBLIC);
			if (direntry) memcpy(direntry, d_ent, d_ent->rec_len);
			//KPrintF("Found Direntry\n");
			ext2_ReleaseBlockINode(gd);
			return direntry;
		}

		dir_offset	+= d_ent->rec_len;
		total_offset+= d_ent->rec_len;
		dir_index++;

		if (dir_offset > gd->block_size) 
		{
			//KPrintF("DE: Fetching new block DirOffset %d, block: %d\n", dir_offset, gd->block_size);
			ext2_ReleaseBlockINode(gd);
			block_nr++;
			dir_offset -= gd->block_size;
			block = ext2_GetBlockINode(gd, inode, block_nr);
		}
	}
	
	//KPrintF("DE: NoMoreEntries: %d\n", gd->gd_GBIN_RB);
	ext2_ReleaseBlockINode(gd);
	return NULL;
}

fs_node_t *ext2_FindDirectory(pGD gd, fs_node_t *node, char *name)
{
	if (!node) return NULL;
	
	if ((node == RN) && (name[0] == '.') && (name[1] == '.')) return NULL;
	
	UINT32				name_len	= Strlen(name);
	UINT8				*block 		= NULL;
	ext2_dir_t			*direntry	= NULL;
	ext2_inodetable_t	*inode 		= ext2_GetINode(gd, node->inode);

	if (inode == NULL) return NULL;
	if (!(inode->mode & EXT2_S_IFDIR)) 
	{
		ext2_ReleaseINode(gd, node->inode);
		return NULL;
	}
	
	UINT8	block_nr = 0;
	UINT32	dir_offset = 0;
	UINT32	total_offset = 0;
	

	block = ext2_GetBlockINode(gd, inode, block_nr);
	
	while (total_offset < inode->size)
	{
		if (dir_offset >= gd->block_size)
		{
			ext2_ReleaseBlockINode(gd);
			block_nr++;
			dir_offset	-= gd->block_size;
			block = ext2_GetBlockINode(gd, inode, block_nr);
		}
		
		ext2_dir_t *d_ent = (ext2_dir_t *)((UINT32)block + dir_offset);

		if (name_len != d_ent->name_len)
		{
			dir_offset	+= d_ent->rec_len;
			total_offset+= d_ent->rec_len;
			continue;
		}
		
		if (!Strnicmp(&d_ent->name, name, name_len))
		{
			direntry = AllocVec(d_ent->rec_len, MEMF_PUBLIC);
			if (direntry) memcpy(direntry, d_ent, d_ent->rec_len);
			break;
		}
		
		dir_offset	+= d_ent->rec_len;
		total_offset+= d_ent->rec_len;
	}
	
	ext2_ReleaseBlockINode(gd);
	ext2_ReleaseINode(gd, node->inode);
	
	if (direntry) 
	{	
		inode = ext2_GetINode(gd, direntry->inode);

		if (inode)
		{
			fs_node_t *fnode = ext2_NodeFromFile(gd, inode, direntry);
			
			if (!fnode) KPrintF("ERROR: Couldnt copy into buffer (FindDir)\n");
			
			ext2_ReleaseINode(gd, direntry->inode);
			FreeVec(direntry);
			return fnode;
		}
		FreeVec(direntry);
	}
	return NULL;
}

INT32 ext2_ReadDir(pGD gd, fs_node_t *node, UINT32 index, struct FileInfoBlock *fib)
{
	ext2_inodetable_t *	inode	= ext2_GetINode(gd, node->inode);
//	KPrintF("ReadDir InodeNo %d Index: %d ------------\n", node->inode, index);
	ext2_dir_t *		direntry= ext2_DirEntry(gd, inode, index);

	ext2_ReleaseINode(gd, node->inode);
	if (!direntry)
	{
		gd->gd_FileErr = ERROR_NO_MORE_ENTRIES;
		//KPrintF("ReadDir: ERROR_NO_MORE_ENTRIES\n");
		return DOSIO_FALSE;
	}
	
	inode = ext2_GetINode(gd, direntry->inode);
	if (!inode)
	{
		gd->gd_FileErr = ERROR_NO_MORE_ENTRIES;
		//KPrintF("ReadDir: ERROR_NO_MORE_ENTRIES 2\n");
		FreeVec(direntry);
		return DOSIO_FALSE;
	}

	fs_node_t *fnode = ext2_NodeFromFile(gd, inode, direntry);

	if (!fnode) KPrintF("ERROR: Couldnt copy into buffer (ReadDir)\n");

	ext2_ReleaseINode(gd, direntry->inode);
	FreeVec(direntry);
	
	fib->fib_DiskKey = index;
	if ((fnode->flags & FS_DIRECTORY) == FS_DIRECTORY) 
	{
		fib->fib_DirEntryType	=
		fib->fib_EntryType		= ST_USERDIR;
	} else 
	{
		fib->fib_DirEntryType	=
		fib->fib_EntryType		= ST_FILE;
	}

	Strcpy(fib->fib_FileName, fnode->name);
	if ((fnode->flags & FS_SYMLINK) == FS_SYMLINK) KPrintF("SymLink\n");

	fib->fib_DiskKey++; // = (INT32) ptr;

	fib->fib_Protection		= fnode->mask;
	fib->fib_Size			= fnode->length;
	fib->fib_NumBlocks		= 0;//(ptr->length >> BLKSHIFT) + 1;
	fib->fib_Date.ds_Days	= 0;//ptr->on_Date.ds_Days;
	fib->fib_Date.ds_Minute	= 0;//ptr->on_Date.ds_Minute;
	fib->fib_Date.ds_Tick	= 0;//ptr->on_Date.ds_Tick;
	fib->fib_Comment[0]		= '\0';

	if (fnode->count == 0)
	{
		Remove((pNode)&fnode->node);
		FreeVec(fnode);
	}
	return DOSIO_TRUE;
}

UINT32 ext2_ReadData(pGD gd, fs_node_t *node, UINT32 offset, UINT32 size, UINT8 *buffer)
{
	ext2_inodetable_t *inode = ext2_GetINode(gd, node->inode);
	UINT32 end;

	if (offset + size > inode->size)  end = inode->size;
	else end = offset + size;

	UINT32 start_block  = offset / gd->block_size;
	UINT32 end_block    = end / gd->block_size;
	UINT32 end_size     = end - end_block * gd->block_size;
	UINT32 size_to_read = end - offset;

	if (end_size == 0) end_block--;

	if (start_block == end_block) 
	{
		UINT8 *buf; //[this->block_size];
		buf = ext2_GetBlockINode(gd, inode, start_block);
		memcpy(buffer, (UINT8 *)(((UINT32)buf) + (offset % gd->block_size)), size_to_read);

		ext2_ReleaseBlockINode(gd);
		ext2_ReleaseINode(gd, node->inode);
		return size_to_read;
	} else 
	{
		UINT32 block_offset;
		UINT32 blocks_read = 0;
		UINT8 *buf; //[this->block_size];
		for (block_offset = start_block; block_offset < end_block; block_offset++, blocks_read++) 
		{
			if (block_offset == start_block) 
			{
				buf = ext2_GetBlockINode(gd, inode, block_offset);
				//inode_read_block(this, inode, node->inode, block_offset, buf);
				memcpy(buffer, (UINT8 *)(((UINT32)buf) + (offset % gd->block_size)), gd->block_size - (offset % gd->block_size));
				ext2_ReleaseBlockINode(gd);		
			} else {
				buf = ext2_GetBlockINode(gd, inode, block_offset);
				//inode_read_block(this, inode, node->inode, block_offset, buf);
				memcpy(buffer + gd->block_size * blocks_read - (offset % gd->block_size), buf, gd->block_size);
				ext2_ReleaseBlockINode(gd);	
			}
		}
		buf = ext2_GetBlockINode(gd, inode, end_block);
		//inode_read_block(this, inode, node->inode, end_block, buf);
		memcpy(buffer + gd->block_size * blocks_read - (offset % gd->block_size), buf, end_size);
		ext2_ReleaseBlockINode(gd);	
	}
	ext2_ReleaseINode(gd, node->inode);
	return size_to_read;
}

