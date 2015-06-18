/**
 * @file makeleanfs.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "ports.h"
#include "devices.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

// LEAN Structure
#define FSVERSION			0x0006
#define SUPERBLOCKMAGIC		0x4E41454C
#define INODEMAGIC			0x45444F4E
#define INDIRECTMAGIC		0x58444E49
#define MAXNAMELENGTH		4068
#define LOGSECTORSIZE		9
#define SECTORSIZE			(1<<LOGSECTORSIZE)
#define MAXSUPERBLOCKSECTOR	32
#define EXTENTSPERINODE		6
#define EXTENTSPERINDIRECT	38

#define SUPERBLOCKCLEAN		1<<0
#define SUPERBLOCKERROR		1<<1

typedef struct SuperBlock
{
	UINT32		sb_Checksum;
	UINT32		sb_Magic;
	UINT16		sb_FSVersion;
	UINT8		sb_preallocCount;
	UINT8	 	sb_logSectorsPerBand;
	UINT32		sb_state;
	UINT8		sb_uuid[16];
	char		sb_volumeLabel[64];
	UINT64		sb_sectorCount;
	UINT64		sb_freeSectorCount;
	UINT64		sb_primarySuper;
	UINT64		sb_backupSuper;
	UINT64		sb_bitmapStart;
	UINT64		sb_rootInode;
	UINT64		sb_badInode;
	UINT8		sb_reserved2[360];
} SuperBlock, *pSuperBlock;

typedef struct Indirect
{
	UINT32		i_checksum;
	UINT32		i_magic;
	UINT64		i_sectorCount;
	UINT64		i_inode;
	UINT64		i_thisSector;
	UINT64		i_prevIndirect;
	UINT64		i_nextIndirect;
	UINT8 		i_extentCount;
	UINT8 		i_reserved1[3];
	UINT32 		i_reserved2;
	UINT64 		i_extentStarts[EXTENTSPERINDIRECT];
	UINT32 		i_extentSizes[EXTENTSPERINDIRECT];
} Indirect, *pIndirect;

typedef struct Inode
{
	UINT32 		checksum;
	UINT32		magic;
	UINT8		extentCount;
	UINT8		reserved1[3];
	UINT32		indirectCount;
	UINT32		linkCount;
	UINT32		uid;
	UINT32		gid;
	UINT32		attributes;
	UINT64		fileSize;
	UINT64		sectorCount;
	UINT64		accessTime;
	UINT64		statusChangeTime;
	UINT64		modificationTime;
	UINT64		creationTime;
	UINT64		firstIndirect;
	UINT64		lastIndirect;
	UINT64		fork;
	UINT64		extentStarts[EXTENTSPERINODE];
	UINT32		extentSizes[EXTENTSPERINODE];
}Inode, *pInode;

typedef struct DirEntry
{
	UINT64		inode;
	UINT8		type;
	UINT8		recLen;
	UINT16		nameLen;
} DirEntry, *pDirEntry;

#define iaRUSR  (1 << 8)  // Owner:  read permission
#define iaWUSR  (1 << 7)  // Owner:  write permission
#define iaXUSR  (1 << 6)  // Owner:  execute permission
#define iaRGRP  (1 << 5)  // Group:  read permission
#define iaWGRP  (1 << 4)  // Group:  write permission
#define iaXGRP  (1 << 3)  // Group:  execute permission
#define iaROTH  (1 << 2)  // Others: read permission
#define iaWOTH  (1 << 1)  // Others: write permission
#define iaXOTH  (1 << 0)  // Others: execute permission
#define iaSUID  (1 << 11) // Set-user-ID on execute
#define iaSGID  (1 << 10) // Set-groupt-ID on execute
#define iaSVTX  (1 << 9)  // Sticky bit
	// Behavior flags
#define iaHidden         (1 << 12) // Don't show in directory listing
#define iaSystem         (1 << 13) // Warn that this is a system file
#define iaArchive        (1 << 14) // File changed since last backup
#define iaSync           (1 << 15) // Synchronous updates
#define iaNoAccessTime   (1 << 16) // Don't update last access time
#define iaImmutable      (1 << 17) // Don't move file clusters
#define iaPrealloc       (1 << 18) // Keep any preallocated sector on close
#define iaInlineExtAttr  (1 << 19) // Reserve space for inline extended attributes
	// File type codes
#define iaFmtMask       (7U << 29) //Bit mask to extract file type
#define iaFmtRegular    (1U << 29) // Regular file
#define iaFmtDirectory  (2U << 29) // Directory
#define iaFmtSymlink    (3U << 29) // Symbolic link
#define iaFmtFork       (4U << 29) // Fork

// Values for struct DirEntry.type

#define ftNone       (0) // No file type, empty DirEntry
#define ftRegular    (1) // Regular file
#define ftDirectory  (2) // Directory
#define ftSymlink    (3) // Symbolic link
#define ftFork       (4)  // Fork, for internal use only

/*-----------------------------------------*/

static UINT32 computeChecksum(const void* data, UINT32 size)
{
	UINT32 res = 0;
	const UINT32* d = (const UINT32*)(data);
	size /= sizeof(UINT32);
	for (INT32 i = 1; i != size; ++i) res = (res << 31) + (res >> 1) + d[i];
	return res;
}

static void createUuid(UINT8* dest)
{
	UINT32 x = 0x12345678;//getCurrentTimestamp() >> 10;
	for (INT32 i = 0; i != 16; ++i)
	{
		x = 1103515245 * x + 12345;
		dest[i] = (UINT8)x;
	}
	dest[6] = (dest[6] & 0x0F) | 0x40;
	dest[8] = (dest[8] & 0x3F) | 0x80;
}
//int make(const char* deviceName, uint64_t primarySuper, uint64_t sectorCount, uint8_t logSectorsPerBand, uint8_t preallocCount, const char* volumeLabel);

typedef struct GlobalData
{
	pDOSBase	dOSBase;
	pSysBase	sysBase;
	pUtilBase	utilBase;
	pMsgPort	p;
	pIOStdReq	io;

	SuperBlock	sb_;
	UINT8 buffer_[512];
	UINT64 sectorsPerBand_;
	UINT64 bandBitmapSize_;
	UINT32 bandCount_;
} GD, *pGD;

#define SysBase		gd->sysBase
#define DOSBase		gd->dOSBase
#define UtilBase	gd->utilBase
#define sb_			gd->sb_
#define sectorsPerBand_	gd->sectorsPerBand_
#define bandBitmapSize_	gd->bandBitmapSize_
#define buffer_			gd->buffer_
#define bandCount_		gd->bandCount_

static void hexdump(pGD gd, unsigned char *buf,int len)
{
	int cnt3,cnt4;
	int cnt=0;
	int cnt2=0;

	do
	{
		Printf("%08X | ",cnt);
		for (cnt3=0;cnt3<16;cnt3++)
		{
			if (cnt<len)
			{
				Printf("%02X ",buf[cnt++]);
			}
			else
				Printf("   ");
		}
		Printf("| ");
		for (cnt4=0;cnt4<cnt3;cnt4++)
		{
			if (cnt2==len)
				break;
			if (buf[cnt2]<0x20)
				Printf(".");
			else
				if (buf[cnt2]>0x7F && buf[cnt2]<0xC0)
					Printf(".");
				else
					Printf("%c", buf[cnt2]);
			cnt2++;
		}
		Printf("\n");
	}
	while (cnt!=len);
}

static int WriteSector(pGD gd, UINT32 off, UINT32 len, APTR buf)
{
	INT32 rc;
	gd->io->io_Command = CMD_WRITE;
	gd->io->io_Offset	= off;
	gd->io->io_Length	= len;
	gd->io->io_Data		= buf;
	//KPrintF("Writing to sector: %d len: %d,", off, len);
	rc = DoIO(gd->io);
	if (off==3) hexdump(gd, buf, 512);
	return rc;
}

static int ReadSector(pGD gd, UINT32 off, UINT32 len, APTR buf)
{
	INT32 rc;
	gd->io->io_Command = CMD_READ;
	gd->io->io_Offset	= off;
	gd->io->io_Length	= len;
	gd->io->io_Data		= buf;
	//KPrintF("Reading from sector: %d len: %d,", off, len);
	rc = DoIO(gd->io);
	return rc;
}

static int OpenDev(pGD gd, const char* deviceName, UINT32 unit)
{
	INT32 rc;
	gd->p = CreateMsgPort(NULL);
	if (gd->p)
	{
		gd->io = CreateIORequest(gd->p,0);
		if (gd->io)
		{
			if (!OpenDevice((STRPTR)deviceName, unit, gd->io, 0))
			{
				return RETURN_OK;
			} else rc = RETURN_ERROR;
			DeleteIORequest(gd->io);
		} else rc = RETURN_ERROR;
		DeleteMsgPort(gd->p);
	} else rc = RETURN_ERROR;
	return rc;
}

static int setBitmap(pGD gd, UINT64 begin, UINT64 end)
{
	Printf("Setting bitmap bits in range [%x..%x)...\n", (UINT32)begin, (UINT32)end);
	UINT64 sector = begin;
	for (UINT64 band = begin >> sb_.sb_logSectorsPerBand; sector < end; ++band)
	{
		UINT64 bitmapBegin = band << sb_.sb_logSectorsPerBand;
		if (band == 0) bitmapBegin += sb_.sb_bitmapStart;
		UINT64 bitmapEnd = bitmapBegin + bandBitmapSize_;
		UINT64 bitmapSector = bitmapBegin
			+ ((sector & (sectorsPerBand_ - 1)) // sector in band
			>> (3 + LOGSECTORSIZE)); // log2(bits per sector)
		for (; (bitmapSector < bitmapEnd) && (sector < end); ++bitmapSector)
		{
			//int res = disk_.read(buffer_, bitmapSector, 1, false);
			int res = ReadSector(gd, bitmapSector, 1, buffer_);
			if (res < 0) return res;
			unsigned bitOfSector = sector & (sectorsPerBand_ - 1) & (SECTORSIZE * 8 - 1);
			for (; (bitOfSector < SECTORSIZE * 8) && (sector < end); ++bitOfSector, ++sector)
			{
				//mkleanfs_.printf("      sector %" PRIu64 " (bitmap sector=%" PRIu64 ", bitOfSector=%u).\n", sector, bitmapSector, bitOfSector);
				unsigned k = bitOfSector >> 3;
				unsigned mask = 1 << (bitOfSector & 7);
				//assert(k < SECTORSIZE);
				//assert((buffer_[k] & mask) == 0);
				//assert(sb_.sb_freeSectorCount != 0);
				buffer_[k] |= mask;
				WriteSector(gd, bitmapSector, 1, buffer_);
				--sb_.sb_freeSectorCount;
			}
		}
	}
	return 0;
}

struct EmptyDir
{
	DirEntry dot;
	char dotName[4];
	DirEntry dotdot;
	char dotdotName[4];
} __attribute__((packed));


#define memcpy(a,b,c) CopyMem(b,a,c)

static int Make(pGD gd, const char* deviceName, UINT32 unitnum, UINT64 primarySuper, UINT64 sectorCount, UINT8 logSectorsPerBand, UINT8 preallocCount, const char* volumeLabel)
{
	int res;
//	SuperBlock	sb_;
//	UINT8 buffer_[512];
	OpenDev(gd, deviceName, unitnum);
	sectorsPerBand_ = 1 << logSectorsPerBand;
	bandBitmapSize_ = sectorsPerBand_ >> (3 + LOGSECTORSIZE);
	bandCount_ = (sectorCount + sectorsPerBand_ - 1) >> logSectorsPerBand;

	Printf("Creating new LEAN volume.\n"
		"  deviceName = %s\n"
		"  primarySuper = %x\n"
		"  sectorCount = %x\n"
		"  preallocCount = %d (%d bytes)\n"
		"  logSectorsPerBand = %d (%x sectors)\n"
		"  volumeLabel = \"%s\"\n", deviceName, (UINT32)primarySuper, (UINT32)sectorCount, preallocCount, (preallocCount + 1) * SECTORSIZE, logSectorsPerBand, (UINT32)sectorsPerBand_, volumeLabel);
	MemSet(&sb_, 0, sizeof(SuperBlock));
	sb_.sb_Magic = SUPERBLOCKMAGIC;
	sb_.sb_FSVersion = FSVERSION;
	sb_.sb_preallocCount = preallocCount;
	sb_.sb_logSectorsPerBand = logSectorsPerBand;
	sb_.sb_sectorCount = sectorCount;
	sb_.sb_freeSectorCount = sectorCount;
	INT32 volumeLabelLength = Strlen((STRPTR)volumeLabel);
	if (volumeLabelLength >= sizeof(sb_.sb_volumeLabel) - 1) volumeLabelLength = sizeof(sb_.sb_volumeLabel) - 1;
	CopyMem((APTR)volumeLabel, sb_.sb_volumeLabel, volumeLabelLength);
	sb_.sb_primarySuper = primarySuper;
	sb_.sb_backupSuper = sectorsPerBand_ - 1;
	if (sb_.sb_backupSuper >= sectorCount) sb_.sb_backupSuper = sectorCount - 1;
	sb_.sb_bitmapStart = primarySuper + 1;
	sb_.sb_rootInode = sb_.sb_bitmapStart + bandBitmapSize_;
	sb_.sb_state = SUPERBLOCKCLEAN;
	createUuid(sb_.sb_uuid);
	Printf("Creating %x bands...\n", (UINT32)bandCount_);
	for (UINT64 band = 0; band != bandCount_; ++band)
	{
		UINT64 bitmapStart = band << sb_.sb_logSectorsPerBand;
		if (band == 0) bitmapStart += sb_.sb_bitmapStart;
		Printf("  Band %x, size=%x, bitmap start=%x, bitmap size=%x.\n", (UINT32)band, (UINT32)sectorsPerBand_, (UINT32)bitmapStart, (UINT32)bandBitmapSize_);
		MemSet(buffer_, 0, sizeof(buffer_));
		for (UINT64 i = 0; i != bandBitmapSize_; ++i)
		{
			res = WriteSector(gd, bitmapStart + i, 1, buffer_);
			if (res < 0) return res;
		}
		res = setBitmap(gd, bitmapStart, bitmapStart + bandBitmapSize_);
		if (res < 0) return res;
	}
	Printf("Marking reserved sectors as allocated...\n");
	res = setBitmap(gd, 0, sb_.sb_primarySuper + 1);
	if (res < 0) return res;
	res = setBitmap(gd, sb_.sb_backupSuper, sb_.sb_backupSuper + 1);
	if (res < 0) return res;

	// Prepare and store the the root directory
	Printf("Writing the root directory at sector %x...\n", (UINT32)sb_.sb_rootInode);
	Inode inode;

	struct EmptyDir root;
	MemSet(&inode, 0, sizeof(inode));
	MemSet(&root, 0, sizeof(root));
	root.dot.inode = sb_.sb_rootInode;
	root.dot.type = ftDirectory;
	root.dot.recLen = 1;
	root.dot.nameLen = 1;
	root.dotName[0] = '.';
	root.dotdot.inode = sb_.sb_rootInode;
	root.dotdot.type = ftDirectory;
	root.dotdot.recLen = 1;
	root.dotdot.nameLen = 2;
	root.dotdotName[0] = '.';
	root.dotdotName[1] = '.';
	inode.magic = INODEMAGIC;
	inode.linkCount = 2;
	inode.fileSize = sizeof(root);
	inode.creationTime = 0x12345678; //getCurrentTimestamp();
	inode.accessTime = inode.creationTime;
	inode.statusChangeTime = inode.creationTime;
	inode.modificationTime = inode.creationTime;
	inode.extentCount = 1;
	inode.attributes = iaRUSR | iaWUSR | iaXUSR | iaPrealloc | iaInlineExtAttr | iaFmtDirectory;
	inode.extentStarts[0] = sb_.sb_rootInode;
	inode.extentSizes[0] = MAX(2, preallocCount + 1);
	inode.sectorCount = inode.extentSizes[0];
	inode.checksum = computeChecksum(&inode, sizeof(Inode));
	UINT32 inlineExtAttr = SECTORSIZE - sizeof(Inode) - sizeof(UINT32);

	MemSet(buffer_, 0, sizeof(buffer_));
	memcpy(buffer_, &inode, sizeof(inode));
	memcpy(buffer_ + sizeof(inode), &inlineExtAttr, sizeof(inlineExtAttr));
//	KPrintF("Writing Inode to sector: %d\n", sb_.sb_rootInode);
	res = WriteSector(gd, sb_.sb_rootInode, 1, buffer_);
//	res = disk_.write(buffer_, sb_.rootInode, 1, false);
	if (res < 0) return res;

	MemSet(buffer_, 0, sizeof(buffer_));
	memcpy(buffer_, &root, sizeof(root));
	res = WriteSector(gd, sb_.sb_rootInode + 1, 1, buffer_);
//	res = disk_.write(buffer_, sb_.rootInode + 1, 1, false);
	if (res < 0) return res;

	res = setBitmap(gd, sb_.sb_rootInode, sb_.sb_rootInode + inode.extentSizes[0]);
	if (res < 0) return res;

	// Store the superblock and the superblock backup
	sb_.sb_Checksum = computeChecksum(&sb_, sizeof(sb_));
	Printf("Writing the superblock at sector %x...\n", (UINT32)sb_.sb_primarySuper);
	res = WriteSector(gd, sb_.sb_primarySuper, 1, &sb_);
	//disk_.write(&sb_, sb_.sb_primarySuper, 1, false);
	if (res < 0) return res;
	Printf("Writing the superblock backup at sector %x...\n", (UINT32)sb_.sb_backupSuper);
	res = WriteSector(gd, sb_.sb_backupSuper, 1, &sb_);
	//res = disk_.write(&sb_, sb_.sb_backupSuper, 1, false);
	if (res < 0) return res;

	// Commit everything
	//res = disk_.flush();
	//if (res < 0) return res;
	Printf("LEAN File System successfully created, %x/%x sectors free.\n", sb_.sb_freeSectorCount, sb_.sb_sectorCount);

	
	
	Flush(Output());
	return 0;
}


#define TEMPLATE 		"DEVICE/A,UNIT/N/A,SUPER/N/A,SECTOR/N/A,PREALLOC/N/A,NAME/A"
#define OPT_DEVICE		0
#define OPT_UNIT		1
#define OPT_SUPER		2
#define OPT_SECTOR		3
#define OPT_PREALLOC	4
#define OPT_NAME		5
#define OPT_COUNT		6

#undef SysBase
DOSIO cmd_makeleanfs(APTR SysBase)
{
	DOSIO		rc = RETURN_ERROR;
	UINT32		opts[OPT_COUNT];
	struct RDargs*	rdargs;
	pGD	gd = AllocVec(sizeof(GD), MEMF_CLEAR);
	gd->sysBase = SysBase;

	if ( (gd->dOSBase = OpenLibrary("dos.library", 0)) )
	{
#if 1
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL)
		{
			KPrintF("Error SET: ReadArgs\n");
			PrintFault(IoErr(), NULL);
		} else{
#endif	
		PutStr("success\n");
#if 0
		gd->utilBase = OpenLibrary("utility.library", 0);
		char deviceName[] 	= "pata.device";
		UINT32 unitNum		= 1;
		UINT64 super		= 1;
		UINT64 sector		= 32768;
		UINT8  logSector	= 12;
		UINT8  prealloc    	= 0;
		char VolName[]		= "My Volume";

		rc = Make(gd, deviceName, unitNum, super, sector, logSector, prealloc, VolName);
#endif
		//rc = RETURN_OK;
#if 1
		}
		FreeArgs(rdargs);
#endif
		CloseLibrary(DOSBase);
	} else
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

//int make(const char* deviceName, uint64_t primarySuper, uint64_t sectorCount, uint8_t logSectorsPerBand, uint8_t preallocCount, const char* volumeLabel);


