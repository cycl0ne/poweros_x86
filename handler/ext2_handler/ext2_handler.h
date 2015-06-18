
//Activate new Filelock
#define NEWFILELOCK 1 

#include "types.h"
#include "lists.h"
#include "devices.h"
#include "tagitem.h"
#include "timer.h"
#include "ext2.h"
#include "dos.h"
#include "dos_io.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_notify.h"
#include "dos_exall.h"
#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"
#include "timer.h"

#define memcpy(a,b,c) CopyMem(b,a,c)

#define DOSBase gd->gd_DOSBase
#define SysBase gd->gd_SysBase
#define UtilBase gd->gd_UtilBase
#define BCBase gd->gd_BCBase

#define SB 		(gd->superblock)
#define BGDS	(gd->block_group_count)
#define BGD  	(gd->block_groups)
#define RN   	(gd->root_node)

#define BLKSHIFT		10
#define MIN_BLKSIZE		(1L << BLKSHIFT)
#define FIRSTBUFFPOS	8
#define DATA_EXTRA		(FIRSTBUFFPOS >> 2)
#define MAX_WRITE		(FIRSTBUFFPOS+MIN_BLKSIZE-1)
#define DATA_BLOCK_SIZE ((MIN_BLKSIZE >> 2) + DATA_EXTRA)
#define MAX_FILENAME	255
#define MAX_COMMENT		79
#define FIRSTBUFFPOS	8
#define LOCK_MODIFY		1
#define SAME 0

struct dirent {
	uint32_t ino;           /* Inode number. */
	char name[256];         /* The filename. */
};

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP  ".."
#define PATH_DOT "."

#define O_RDONLY     0x0000
#define O_WRONLY     0x0001
#define O_RDWR       0x0002
#define O_APPEND     0x0008
#define O_CREAT      0x0200
#define O_TRUNC      0x0400
#define O_EXCL       0x0800

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define _IFMT       0170000 /* type of file */
#define _IFDIR      0040000 /* directory */
#define _IFCHR      0020000 /* character special */
#define _IFBLK      0060000 /* block special */
#define _IFREG      0100000 /* regular */
#define _IFLNK      0120000 /* symbolic link */
#define _IFSOCK     0140000 /* socket */
#define _IFIFO      0010000 /* fifo */

typedef struct fs_node {
	MinNode	 node;
	UINT32	 count;
	char name[256];         /* The filename. */
	uint32_t mask;          /* The permissions mask. */
	uint32_t uid;           /* The owning user. */
	uint32_t gid;           /* The owning group. */
	uint32_t flags;         /* Flags (node type, etc). */
	uint32_t inode;         /* Inode number. */
	uint32_t length;        /* Size of the file, in byte. */
	uint32_t impl;          /* Used to keep track which fs it belongs to. */
	uint32_t open_flags;    /* Flags passed to open (read/write/append, etc.) */

	/* times */
	uint32_t atime;         /* Accessed */
	uint32_t mtime;         /* Modified */
	uint32_t ctime;         /* Created  */

	uint32_t offset;       /* Offset for read operations XXX move this to new "file descriptor" entry */
	int32_t refcount;
	uint32_t nlink;
    APTR     gd;
}fs_node_t;

typedef struct FSLock {
	FileLock	 		rl_Lock;
	APTR				rl_Block;
	UINT32		 		rl_Offset;
	UINT32				rl_CPos;
	UINT32				rl_Flags;
	UINT32				rl_DelCount;
}FSLock, *pFSLock;

typedef struct GlobalData
{
	struct Process *	gd_myproc;
	APTR				gd_SysBase;
	APTR				gd_DOSBase;
	APTR				gd_UtilBase;
	APTR				gd_BCBase;
	INT32				gd_CacheSize;
	APTR				gd_Cache;
	struct MsgPort*		gd_msgport;           /* communication port to DOS (normally == g->devnode->dn_Task) */
	struct MsgPort*		gd_port;              /* for communication with diskdevice    */
	struct IOStdReq*	gd_request;           /* request structure for diskdevice     */
	struct MsgPort*		gd_tport;              /* for communication with timerdevice    */
	struct TimeRequest*	gd_trequest;           /* request structure for timerdevice     */
	UINT32				gd_timesec;				/* How many secs until flush */
	BOOL				gd_timedirty;			/* if true, flush and set to false */
	
	STRPTR 				gd_mountName;
	pStartupMsg 		gd_fssm;
	pDosEntry 			gd_dosEntry;

	ext2_superblock_t       * superblock;          /* Device superblock, contains important information */
	ext2_bgdescriptor_t     * block_groups;        /* Block Group Descriptor / Block groups */
	fs_node_t               * root_node;           /* Root FS node (attached to mountpoint) */

	UINT32              block_size;          /* Size of one block */
	UINT32              pointers_per_block;  /* Number of pointers that fit in a block */
	UINT32              inodes_per_group;    /* Number of inodes in a "group" */
	UINT32              block_group_count;   /* Number of blocks groups */

	fs_node_t			*gd_CurNode;
	List				gd_NodeList;
	List				gd_LockList;
	pDosEntry			gd_VolumeNode;
	INT32				gd_DiskType;
	INT32				gd_FileErr;
	INT32				gd_PathPos;
	UINT32				gd_GBIN_RB;	// Cache RealBlock
	UINT32				gd_BufferBlock;
	UINT8				gd_Buffer[4096*2];
	ext2_inodetable_t	gd_INodeBuffer;
	uint32_t			gd_Debug;
} GD, *pGD;

APTR ext2_GetBlock(pGD gd, UINT32 block_no);
INT32 ext2_ReleaseBlock(pGD gd, UINT32 block_no);
UINT32 ext2_GetBlockNumber(pGD gd, ext2_inodetable_t *inode, UINT32 iblock);
UINT8 *ext2_GetBlockINode(pGD gd, ext2_inodetable_t *inode, UINT32 block_no);
UINT32 ext2_ReleaseBlockINode(pGD gd);
ext2_inodetable_t * ext2_GetINode(pGD gd, UINT32 inode);
UINT32 ext2_ReleaseINode(pGD gd, UINT32 inode);
fs_node_t *ext2_NodeFromFile(pGD gd, ext2_inodetable_t *inode, ext2_dir_t *direntry);
fs_node_t *ext2_FindDirectory(pGD gd, fs_node_t *node, char *name);
fs_node_t *ext2_ParentDirectory(pGD gd, fs_node_t *node);
INT32 ext2_ReadDir(pGD gd, fs_node_t *node, UINT32 index, struct FileInfoBlock *fib);
ext2_dir_t * ext2_DirEntry(pGD gd, ext2_inodetable_t * inode, UINT32 index);
UINT32 ext2_ReadData(pGD gd, fs_node_t *node, UINT32 offset, UINT32 size, UINT8 *buffer);
void ext2_SetTimeDirty(pGD gd);

FSLock *ext2_CreateObject(GD *gd, FSLock *lock, char *name, uint16_t ext2_mode,
                          uint32_t ext2_flags, int32_t lock_mode);
unsigned ext2_Write(GD *gd, FSLock *lock, void *buffer, unsigned size);
int32_t ext2_Seek(GD *gd, FSLock *lock,
                  int32_t offset, int origin, BOOL resize);
BOOL ext2_DeleteObject(GD *gd, FSLock *lock, char *name);
