/**
 * @file console_device.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#ifndef vioblk_device_h
#define vioblk_device_h

#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "virtio.h"

#include "exec_interface.h"
#include "expansion_interface.h"
#include "virtio_interface.h"

// Internal Flags
#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)

// Errors
#define	BLK_ERR_Unknown				(-20)	// general; catch all other errors
#define	BLK_ERR_BadSectorRequest	(-21)	// requested sector number is wrong
#define	BLK_ERR_BadSectorDataSum	(-22)	// sector data has incorrect checksum
#define	BLK_ERR_NotEnoughSectors	(-23)	// couldn't find enough sectors
#define	BLK_ERR_WriteProtected		(-24)	// can't write to a write-protected disk
#define	BLK_ERR_DiskNotFound		(-25)	// no disk found in the drive
#define	BLK_ERR_NoMem				(-26)	// couldn't get enough memory
#define	BLK_ERR_BadUnitNum			(-27)	// asked for a unit >= available number of units
#define	BLK_ERR_BadDriveType		(-28)	// not a drive that block device handles
#define	BLK_ERR_DriveInUse			(-29)	// someone else is using the drive
#define	BLK_ERR_DiskChanged			(-30)	// disk changed

// non standard async commands
#define VB_GETDEVICEINFO            (CMD_NONSTD+0)
#define VB_GETDISKCHANGECOUNT       (CMD_NONSTD+1)
#define VB_GETDISKPRESENCESTATUS    (CMD_NONSTD+2)
#define VB_EJECT                    (CMD_NONSTD+3)
#define VB_FORMAT                   (CMD_NONSTD+4)
#define VB_GETNUMTRACKS             (CMD_NONSTD+5)
#define VB_WRITEPROTECTIONSTATUS    (CMD_NONSTD+6)

#define MAX_CMD VB_WRITEPROTECTIONSTATUS

//disk present in drive?
#define VBF_NO_DISK	0x01
#define VBF_DISK_IN	0x00

//disk write protection status
#define VBF_WRITE_PROTECTED 0x01
#define VBF_NOT_WRITE_PROTECTED 0x00

//flags for cache
#define VBF_CLEAN      (0)
#define VBF_DIRTY      (1<<0)
#define VBF_INVALID      (1<<1)

// Units
#define VB_UNIT_MAX		4

//device id
#define VIRTIO_BLK_DEVICE_ID 0x1001

/* These two define direction. */
#define VIRTIO_BLK_T_IN		0
#define VIRTIO_BLK_T_OUT	1

#define VB_WRITE VIRTIO_BLK_T_OUT
#define VB_READ VIRTIO_BLK_T_IN

//how many ques we have
#define VIRTIO_BLK_NUM_QUEUES 1

// Feature bits for virtio blk device
#define VIRTIO_BLK_F_BARRIER	0	// Does host support barriers?
#define VIRTIO_BLK_F_SIZE_MAX	1	// Indicates maximum segment size
#define VIRTIO_BLK_F_SEG_MAX	2	// Indicates maximum # of segments
#define VIRTIO_BLK_F_GEOMETRY	4	// Legacy geometry available
#define VIRTIO_BLK_F_RO			5	// Disk is read-only
#define VIRTIO_BLK_F_BLK_SIZE	6	// Block size of disk is available
#define VIRTIO_BLK_F_SCSI		7	// Supports scsi command passthru
#define VIRTIO_BLK_F_FLUSH		9	// Cache flush command support
#define VIRTIO_BLK_F_TOPOLOGY	10	// Topology information is available
#define VIRTIO_BLK_ID_BYTES		20	// ID string length


/* This is the first element of the read scatter-gather list. */
struct virtio_blk_outhdr {
	/* VIRTIO_BLK_T* */
	UINT32 type;
	/* io priority. */
	UINT32 ioprio;
	/* Sector (ie. 512 byte offset) */
	UINT64 sector;
};

//*****************

struct VirtioBlkGeometry
{
	UINT16 cylinders;
	UINT8 heads;
	UINT8 sectors;
};

struct VirtioBlkDeviceInfo
{
	UINT64 capacity; //number of 512 byte sectors
	UINT32 blk_size; //512 or 1024 etc.
	struct VirtioBlkGeometry geometry;
};


typedef struct VirtioBlk
{
	//dev structure
	VirtIODevice_t vd;

	//device capacity etc.
	struct VirtioBlkDeviceInfo Info;

	//Headers for requests
	struct virtio_blk_outhdr *hdrs;

	//Status bytes for requests.
	//Usually a status is only one byte in length, but we need the lowest bit
	//to propagate writable. For this reason we take u16_t and use a mask for
	//the lower byte later.
	UINT16 *status;


} VirtioBlk;

typedef struct VIOUnit {
	Unit_t				unit;
	struct VirtioBlk	vb;

	// IRQ Handling	
	BOOL				gotIRQ;

	BOOL				diskPresence;
	BOOL				writeProtection;
	uint32_t			diskChangeCounter;	
} VIOUnit_t, *pVIOUnit;


typedef struct VIOBlkBase
{
	Device		  	dev_Device;
	pSysBase		dev_SysBase;
	pVIOBase		dev_VIOBase;
	pExpansionBase	dev_ExpansionBase;

	VIOUnit_t	dev_Unit[VB_UNIT_MAX];
    int32_t    dev_MaxUnits;

	pInterrupt			IntHandler;

	pTask		dev_BootTask;
	pTask		dev_Task;
	pMsgPort	dev_Port;
    uint32_t    dev_signalTask;
} VIOBlkBase_t, *pVIOBlkBase;

#endif
