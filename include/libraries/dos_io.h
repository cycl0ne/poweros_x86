#ifndef DOS_IO_H
#define DOS_IO_H

#include "types.h"
#include "ports.h"
#include "semaphores.h"

typedef INT32 (*Command)(void *);

#define MAX_SEGNAME 256

typedef struct Segment {
	Node			seg_Node;	// Link them together
	UINT8			seg_Name[MAX_SEGNAME];
	INT32			seg_Flags;	// System or Userloaded
	INT32			seg_Count;	// In use?
	Command			seg_Entry;	// the main(); of the program
	APTR			seg_Memory;	// if there has been Memory allocated for this Program
	SignalSemaphore	seg_Lock;	// Lock for this Segment
} Segment, *pSegment;

// Flags seg_Flags
//
#define CMD_SYSTEM		-1
#define CMD_INTERNAL	-2
#define CMD_DISABLED	-999
#define CMD_USER		0

// The DOS DateStamp structure for timekeeping
// 
typedef struct DateStamp {
	INT32		ds_Days;
	INT32		ds_Minute;
	INT32		ds_Tick;
} DateStamp, *pDateStamp;


// The FileLock structure. Locks Dirs and Files in the Handler
// 
#ifdef NEWFILELOCK
typedef struct FileLock {
	MinNode				fl_Node;
	INT32				fl_Key;		// Private for Handlers (passed Argument)
	INT32				fl_Access;	// Access to this File
	pMsgPort			fl_Handler;	// Should be renamed to fl_Handler, the Packet Port to the Handler
	struct DosEntry *	fl_Volume;	// Pointer to the DosEntry for this Lock
}FileLock, *pFileLock;
#else
typedef struct FileLock {
	struct FileLock *	fl_Next;	// single linked list
	APTR				fl_Priv;	// Placeholder
	INT32				fl_Key;		// Private for Handlers (passed Argument)
	INT32				fl_Access;	// Access to this File
	pMsgPort			fl_Handler;	// Should be renamed to fl_Handler, the Packet Port to the Handler
	struct DosEntry *	fl_Volume;	// Pointer to the DosEntry for this Lock
}FileLock, *pFileLock;
#endif

// Passed to Lock() and also the value for the FileLock.fl_Access
// 
#define SHARED_LOCK		-2	// Shared between others
#define ACCESS_READ		-2	// Synonym, READ ACCESS
#define EXCLUSIVE_LOCK	-1	// Locked for Write
#define ACCESS_WRITE	-1	// Synonym

//  This structure is passed from an Open() and has to be delivered to Close()
// 
typedef struct FileHandle
{
	MinNode			fh_Node;
	UINT32			fh_Flags;
	BOOL			fh_Interactive;
	pMsgPort		fh_Type;		// rename it to a proper thing fh_Handler

/*
// Everything below this is for buffered io, not used at the moment
	UINT8 *				fh_Buf;
	INT32				fh_BufSize;
	INT32				fh_Pos;
	INT32				fh_End;
	INT32				fh_Func1;
	INT32				fh_Func2;
	INT32				fh_Func3;
*/
	INT32		fh_Arg1;
	INT32		fh_Arg2;

	INT8*		fh_buffer;
	UINT32		fh_bufsize;
	UINT32		fh_bufidx;
	UINT32		fh_bufend;
	UINT32		fh_ungetidx;
	INT8*		fh_ungetbuf;
	INT32		fh_status;
	UINT32		fh_pos;
}FileHandle, *pFileHandle; 

// Open parameters. Same as their ACTION_FINDxxxx Pendant!
// 
#define MODE_OLDFILE	     1005	// Open existing file. Error if not found
#define MODE_NEWFILE	     1006	// Open/Create new File. EXCLUSIVE_LOCK
#define MODE_READWRITE	     1004	// Open old File with SHARED_LOCK, creates file if it doesnt exist

// Parameters for SEEK()
// 
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define OFFSET_BEGINNING		-1	// rel. begin of file
#define OFFSET_CURRENT			 0	// rel. current file
#define OFFSET_END				 1	// rel. end of file


// The assign structure a_Next is used for Multipath assigns.
//
#if 0
typedef struct Assign {
	struct Assign*		a_Next;
	pFileLock			a_Lock;
}Assign, *pAssign;
#endif

typedef struct Assign {
	MinNode		a_Node;
	pFileLock	a_Lock;
} Assign, *pAssign;

typedef struct DosEnvec {
    UINT32 de_TableSize;	     /* Size of Environment vector */
    UINT32 de_SizeBlock;	     /* in longwords: standard value is 128 */
    UINT32 de_SecOrg;	     /* not used; must be 0 */
    UINT32 de_Surfaces;	     /* # of heads (surfaces). drive specific */
    UINT32 de_SectorPerBlock; /* not used; must be 1 */
    UINT32 de_BlocksPerTrack; /* blocks per track. drive specific */
    UINT32 de_Reserved;	     /* DOS reserved blocks at start of partition. */
    UINT32 de_PreAlloc;	     /* DOS reserved blocks at end of partition */
    UINT32 de_Interleave;     /* usually 0 */
    UINT32 de_LowCyl;	     /* starting cylinder. typically 0 */
    UINT32 de_HighCyl;	     /* max cylinder. drive specific */
    UINT32 de_NumBuffers;     /* Initial # DOS of buffers.  */
    UINT32 de_BufMemType;     /* type of mem to allocate for buffers */
    UINT32 de_MaxTransfer;    /* Max number of bytes to transfer at a time */
    UINT32 de_Mask;	     /* Address Mask to block out certain memory */
    INT32  de_BootPri;	     /* Boot priority for autoboot */
    UINT32 de_DosType;	     /* ASCII (HEX) string showing filesystem type;
			      * 0X444F5300 is old filesystem,
			      * 0X444F5301 is fast file system */
    UINT32 de_Baud;	     /* Baud rate for serial handler */
    UINT32 de_Control;	     /* Control word for handler/filesystem */
    UINT32 de_BootBlocks;     /* Number of blocks containing boot code */

}DosEnvec, *pDosEnvec;

#define DE_TABLESIZE	0	/* minimum value is 11 (includes NumBuffers) */
#define DE_SIZEBLOCK	1	/* in longwords: standard value is 128 */
#define DE_SECORG	2	/* not used; must be 0 */
#define DE_NUMHEADS	3	/* # of heads (surfaces). drive specific */
#define DE_SECSPERBLK	4	/* not used; must be 1 */
#define DE_BLKSPERTRACK 5	/* blocks per track. drive specific */
#define DE_RESERVEDBLKS 6	/* unavailable blocks at start.	 usually 2 */
#define DE_PREFAC	7	/* not used; must be 0 */
#define DE_INTERLEAVE	8	/* usually 0 */
#define DE_LOWCYL	9	/* starting cylinder. typically 0 */
#define DE_UPPERCYL	10	/* max cylinder.  drive specific */
#define DE_NUMBUFFERS	11	/* starting # of buffers.  typically 5 */
#define DE_MEMBUFTYPE	12	/* type of mem to allocate for buffers. */
#define DE_BUFMEMTYPE	12	/* same as above, better name
				 * 1 is public, 3 is chip, 5 is fast */
#define DE_MAXTRANSFER	13	/* Max number bytes to transfer at a time */
#define DE_MASK		14	/* Address Mask to block out certain memory */
#define DE_BOOTPRI	15	/* Boot priority for autoboot */
#define DE_DOSTYPE	16	/* ASCII (HEX) string showing filesystem type;
				 * 0X444F5300 is old filesystem,
				 * 0X444F5301 is fast file system */
#define DE_BAUD		17	/* Baud rate for serial handler */
#define DE_CONTROL	18	/* Control word for handler/filesystem */
#define DE_BOOTBLOCKS	19	/* Number of blocks containing boot code */


typedef struct FileSysStartupMsg 
{
    UINT32		fssm_Unit;	/* exec unit number for this device */
    STRPTR		fssm_Device;	/* null terminated string to the device name */
    pDosEnvec	fssm_Environ;	/* ptr to environment table (see above) */
    UINT32		fssm_Flags;	/* flags for OpenDevice() */
}StartupMsg, *pStartupMsg;

// The DosEntry structure for Handlers/Assign/Volumes
//
typedef struct DosEntry {
	Node		de_Node;
	INT32		de_Type;
	pMsgPort	de_Handler;
	pFileLock	de_Lock;
	
	union
	{
		struct {
			pSegment		de_HandlerSegment;
			UINT32			de_HandlerStack;
			UINT32			de_HandlerPrio;
			pStartupMsg		de_Startup;
		} handlerNode;

		struct {
			DateStamp		de_VolumeDate;
			pFileLock		de_LockList;
			UINT32			de_DiskType;
		} volumeNode;

		struct {
			STRPTR			de_AssignName;
			//pAssign			de_AssignListe;
			MinList			de_AssignList;
			STRPTR			de_AssignLate;
		} assignNode;
	} de_Misc;

} DosEntry, *pDosEntry;

// de_Type Values from DosEntry
//
#define DLT_DEVICE		0 // Handler
#define DLT_VOLUME		1 // Volume
#define DLT_DIRECTORY	2 // Assign
#define DLT_LATE		3 // Assign
#define DLT_NONBINDING	4 // Assign
#define DLT_PRIVATE		-1 // NOT USED ATM

//
//
#define LDB_DEVICES	2
#define LDF_DEVICES	(1L << LDB_DEVICES)
#define LDB_VOLUMES	3
#define LDF_VOLUMES	(1L << LDB_VOLUMES)
#define LDB_ASSIGNS	4
#define LDF_ASSIGNS	(1L << LDB_ASSIGNS)
#define LDB_ENTRY	5
#define LDF_ENTRY	(1L << LDB_ENTRY)
#define LDB_DELETE	6
#define LDF_DELETE	(1L << LDB_DELETE)

//
//
#define LDB_READ	0
#define LDF_READ	(1L << LDB_READ)
#define LDB_WRITE	1
#define LDF_WRITE	(1L << LDB_WRITE)

#define LDF_ALL		(LDF_DEVICES|LDF_VOLUMES|LDF_ASSIGNS)

// FIB
//
typedef struct FileInfoBlock {
	INT32				fib_DiskKey;
	INT32				fib_DirEntryType;  /* Type of Directory. If < 0, then a plain file.
											* If > 0 a directory */
	char				fib_FileName[108]; /* Null terminated. Max 30 chars used for now */
	INT32				fib_Protection;    /* bit mask of protection, rwxd are 3-0.	   */
	INT32				fib_EntryType;
	INT32				fib_Size;	     /* Number of bytes in file */
	INT32				fib_NumBlocks;     /* Number of blocks in file */
	struct DateStamp	fib_Date;/* Date file last changed */
	char				fib_Comment[80];  /* Null terminated comment associated with file */

	UINT16				fib_OwnerUID;		/* owner's UID */
	UINT16				fib_OwnerGID;		/* owner's GID */
} FileInfoBlock, *pFileInfoBlock;

// fib_DirEntryType
//
#define ST_ROOT		1
#define ST_USERDIR	2
#define ST_SOFTLINK	3	/* looks like dir, but may point to a file! */
#define ST_LINKDIR	4	/* hard link to dir */
#define ST_FILE		-3	/* must be negative for FIB! */
#define ST_LINKFILE	-4	/* hard link to file */
#define ST_PIPEFILE	-5	/* for pipes that support ExamineFH */

// fib_Protection
//
#define FIBB_OTR_READ      15	/* Other: file is readable */
#define FIBB_OTR_WRITE     14	/* Other: file is writable */
#define FIBB_OTR_EXECUTE   13	/* Other: file is executable */
#define FIBB_OTR_DELETE    12	/* Other: prevent file from being deleted */
#define FIBB_GRP_READ      11	/* Group: file is readable */
#define FIBB_GRP_WRITE     10	/* Group: file is writable */
#define FIBB_GRP_EXECUTE   9	/* Group: file is executable */
#define FIBB_GRP_DELETE    8	/* Group: prevent file from being deleted */

#define FIBB_SCRIPT    6	/* program is a script (execute) file */
#define FIBB_PURE      5	/* program is reentrant and rexecutable */
#define FIBB_ARCHIVE   4	/* cleared whenever file is changed */
#define FIBB_READ      3	/* ignored by old filesystem */
#define FIBB_WRITE     2	/* ignored by old filesystem */
#define FIBB_EXECUTE   1	/* ignored by system, used by Shell */
#define FIBB_DELETE    0	/* prevent file from being deleted */

#define FIBF_OTR_READ      (1<<FIBB_OTR_READ)
#define FIBF_OTR_WRITE     (1<<FIBB_OTR_WRITE)
#define FIBF_OTR_EXECUTE   (1<<FIBB_OTR_EXECUTE)
#define FIBF_OTR_DELETE    (1<<FIBB_OTR_DELETE)
#define FIBF_GRP_READ      (1<<FIBB_GRP_READ)
#define FIBF_GRP_WRITE     (1<<FIBB_GRP_WRITE)
#define FIBF_GRP_EXECUTE   (1<<FIBB_GRP_EXECUTE)
#define FIBF_GRP_DELETE    (1<<FIBB_GRP_DELETE)

#define FIBF_SCRIPT    (1<<FIBB_SCRIPT)
#define FIBF_PURE      (1<<FIBB_PURE)
#define FIBF_ARCHIVE   (1<<FIBB_ARCHIVE)
#define FIBF_READ      (1<<FIBB_READ)
#define FIBF_WRITE     (1<<FIBB_WRITE)
#define FIBF_EXECUTE   (1<<FIBB_EXECUTE)
#define FIBF_DELETE    (1<<FIBB_DELETE)

struct InfoData { 
   INT32	  id_NumSoftErrors;	/* number of soft errors on disk */
   INT32	  id_UnitNumber;	/* Which unit disk is (was) mounted on */
   INT32	  id_DiskState;		/* See defines below */
   INT32	  id_NumBlocks;		/* Number of blocks on disk */
   INT32	  id_NumBlocksUsed;	/* Number of block in use */
   INT32	  id_BytesPerBlock;   
   INT32	  id_DiskType;		/* Disk Type code */
   pDosEntry  id_VolumeNode;
   INT32	  id_InUse;		/* Flag, zero if not in use */
}; /* InfoData */

// id_DiskState
//
#define ID_WRITE_PROTECTED 80	 /* Disk is write protected */
#define ID_VALIDATING	   81	 /* Disk is currently being validated */
#define ID_VALIDATED	   82	 /* Disk is consistent and writeable */

//DiskTypes
//
#define ID_NO_DISK_PRESENT	(-1)
#define ID_UNREADABLE_DISK	(0x42414400L)	/* 'BAD\0' */
#define ID_DOS_DISK		(0x444F5300L)	/* 'DOS\0' */
#define ID_FFS_DISK		(0x444F5301L)	/* 'DOS\1' */
#define ID_INTER_DOS_DISK	(0x444F5302L)	/* 'DOS\2' */
#define ID_INTER_FFS_DISK	(0x444F5303L)	/* 'DOS\3' */
#define ID_FASTDIR_DOS_DISK	(0x444F5304L)	/* 'DOS\4' */
#define ID_FASTDIR_FFS_DISK	(0x444F5305L)	/* 'DOS\5' */
#define ID_NOT_REALLY_DOS		(0x534F444EL)	/* 'NDOS'  */
#define ID_NOT_REALLY_DOS_BE	(0x4E444F53L)	/* Big Endian: 'NDOS'  */
#define ID_KICKSTART_DISK	(0x4B49434BL)	/* 'KICK'  */
#define ID_MSDOS_DISK		(0x4d534400L)	/* 'MSD\0' */

typedef struct RDargs
{
  UINT8* inputBuf;
  UINT32* memBuf;
}RDargs, *pRDargs;

#define MAX_OPTS 64
#define MAX_ARGS 128
#define BUF_CHUNK 512

#define SHELL_BOOT			0
#define SHELL_RUNEX			1
#define SHELL_SYSTEM_SYNC 	2
#define SHELL_SYSTEM_ASYNC	3
#define SHELL_NEWCLI		4

typedef struct StartupMsg
{
	UINT32		sm_SType;		// SM Type: Shell, Disk,...
	
	// - Shell Message -------------
	UINT32		sm_ShellType;	// SHELL_RUNEX, SHELL_SYSTEM, SHELL_SYSTEM_ASYNC
	pFileHandle	sm_Command;
	pFileHandle	sm_Input;
	pFileHandle	sm_Output;
	pFileLock	sm_CurDir;
	BOOL		sm_Run;
	pMsgPort	sm_FileSysTask;
} ShellSM, *pShellSM;

#include "dos.h"

#endif
