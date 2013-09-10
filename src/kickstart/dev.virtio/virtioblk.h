#ifndef virtioblk_h
#define virtioblk_h

#include "lib_virtio.h"

//****************
// Header for the std. Virtio Blk Handling specification
//****************

//****************
#define VIRTIO_BLK_DEVICE_ID 0x1001

/* These two define direction. */
#define VIRTIO_BLK_T_IN		0
#define VIRTIO_BLK_T_OUT	1

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

/*
struct VirtioBlkRequest
{
    struct IOStdReq node;
    struct VirtioBlkDeviceInfo info;
    UINT32 sector_start;
    UINT32 num_sectors;
    UINT8 write;
    UINT8* buf;
};
*/

typedef struct VirtioBlk
{
	//dev structure
	struct VirtioDevice vd;

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

#endif
