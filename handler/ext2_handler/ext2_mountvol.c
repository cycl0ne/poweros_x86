/**
 * @file ext2handler.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */

#include "ext2_handler.h"


//#define DC   	(this->disk_cache)


static UINT32 ext2_root(pGD gd, ext2_inodetable_t *inode, fs_node_t *fnode) 
{
	if (!fnode) return 0;

	/* Information for root dir */
	fnode->gd = (void *)gd;
	fnode->inode = 2;
	
	/* Information from the inode */
	fnode->uid = inode->uid;
	fnode->gid = inode->gid;
	fnode->length = inode->size;
	fnode->mask = inode->mode & 0xFFF;
	fnode->nlink = inode->links_count;
	/* File Flags */
	fnode->flags = 0;
	if ((inode->mode & EXT2_S_IFREG) == EXT2_S_IFREG) 
	{
		KPrintF( "Root appears to be a regular file.");
		KPrintF( "This is probably very, very wrong.");
		return 0;
	}
	if ((inode->mode & EXT2_S_IFDIR) == EXT2_S_IFDIR) 
	{
		fnode->flags |= FS_DIRECTORY;
		//fnode->create = NULL; //ext2_create;
		//fnode->mkdir = NULL; //ext2_mkdir;
	} else {
		KPrintF( "Root doesn't appear to be a directory.");
		KPrintF( "This is probably very, very wrong.");

		KPrintF( "Other useful information:");
		KPrintF( "%d", inode->uid);
		KPrintF( "%d", inode->gid);
		KPrintF( "%d", inode->size);
		KPrintF( "%d", inode->mode);
		KPrintF( "%d", inode->links_count);

		return 0;
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
#if 0
	fnode->read    = read_ext2;
	fnode->write   = NULL;
	fnode->open    = open_ext2;
	fnode->close   = close_ext2;
	fnode->readdir = readdir_ext2;
	fnode->finddir = finddir_ext2;
	fnode->ioctl   = NULL;
#endif
	return 1;
}


BOOL ext2_MountVol(pGD gd)
{
//	KPrintF("Mounting ext2 file system\n");
	gd->block_size = 1024;
	APTR mem = NULL;
	
	SB = AllocVec(gd->block_size, MEMF_PUBLIC|MEMF_CLEAR);
//	KPrintF("Reading Superblock (mem: %x [%x])\n", mem, (UINT8 *)SB);
	mem = ext2_GetBlock(gd, 1);
	memcpy(SB, mem, gd->block_size);
	ext2_ReleaseBlock(gd, 1);


	if (SB->magic != EXT2_SUPER_MAGIC)
	{
		KPrintF("ERROR: ... not an EXT2 filesystem? (magic didn't match, got 0x%x)", SB->magic);
		return FALSE;
	}

//	KPrintF("Inode_size: %d\n", SB->inode_size);
	if (SB->inode_size == 0) 
	{
		SB->inode_size = 128;
	}
	gd->block_size = 1024 << SB->log_block_size;
	gd->pointers_per_block = gd->block_size / 4;

//	KPrintF("Log block size = %d -> %d\n", SB->log_block_size, gd->block_size);

	BGDS = SB->blocks_count / SB->blocks_per_group;
//	KPrintF("BGDS: %d\n", BGDS);
	
	if (SB->blocks_per_group * BGDS < SB->blocks_count) 
	{
		BGDS += 1;
	}
//	KPrintF("BGDS: %d\n", BGDS);
	
	gd->inodes_per_group = SB->inodes_count / BGDS;

	// load the block group descriptors
	int bgd_block_span = sizeof(ext2_bgdescriptor_t) * BGDS / gd->block_size + 1;
//	KPrintF("bgd_block_span = %d, block_size = %d\n", bgd_block_span, gd->block_size);
	BGD = AllocVec(gd->block_size * bgd_block_span, MEMF_PUBLIC);

//	KPrintF("bgd_block_span = %d\n", bgd_block_span);

	int bgd_offset = 2;

	if (gd->block_size > 1024) bgd_offset = 1;
	for (int i = 0; i < bgd_block_span; ++i) 
	{
		mem = ext2_GetBlock(gd, bgd_offset + i);
		memcpy((UINT8 *)((UINT32)BGD + gd->block_size * i), mem, gd->block_size);
		ext2_ReleaseBlock(gd, bgd_offset + i);
		//ext2_ReadBlock(gd, bgd_offset + i, (UINT8 *)((UINT32)BGD + gd->block_size * i));
	}

	ext2_inodetable_t *root_inode = ext2_GetINode(gd, 2);

	RN = (fs_node_t *)AllocVec(sizeof(fs_node_t), MEMF_PUBLIC|MEMF_CLEAR);
	if (RN == NULL)
	{
		KPrintF("ERROR: No Memory for RooNode\n");
		ext2_ReleaseINode(gd, 2);
		return FALSE;
	}
	
	if (!ext2_root(gd, root_inode, RN)) 
	{
		ext2_ReleaseINode(gd, 2);
		return FALSE;
	}

	// Add the counter of the node and put it into our nodelist for cache.
	RN->count++;

	AddHead((pList)&gd->gd_NodeList, (pNode)&gd->root_node->node);

#if 0
	fs_node_t *nodetmp = NULL;
	nodetmp = (fs_node_t *) GetHead(&gd->gd_NodeList);
	KPrintF("Two %d %x Head: %x %s\n", sizeof(fs_node_t), RN, GetHead(&gd->gd_NodeList), nodetmp->name);

	ForeachNode(&gd->gd_NodeList, nodetmp)
	{
		KPrintF("Found Nodenr: %d (%x), Name:   [%s]\n", nodetmp->inode, nodetmp, nodetmp->name);	
	}
#endif
	ext2_ReleaseINode(gd, 2);	
	KPrintF("Mounted EXT2 disk, root VFS node is at %x", &RN->node);	
	return TRUE;
}

