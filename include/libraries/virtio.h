/**
* File: /virtio．h
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef virtio_h
#define virtio_h

#include "types.h"
#include "libraries.h"
#include "exec_interface.h"
#include "pci.h"
#include "virtio_ring.h"

//vendor id for all virtio devices
#define VIRTIO_VENDOR_ID 0x1af4

//pci config offsets
#define VIRTIO_HOST_F_OFFSET			0x0000
#define VIRTIO_GUEST_F_OFFSET			0x0004
#define VIRTIO_QADDR_OFFSET				0x0008
#define VIRTIO_QSIZE_OFFSET				0x000C
#define VIRTIO_QSEL_OFFSET				0x000E
#define VIRTIO_QNOTFIY_OFFSET			0x0010
#define VIRTIO_DEV_STATUS_OFFSET		0x0012
#define VIRTIO_ISR_STATUS_OFFSET		0x0013
#define VIRTIO_DEV_SPECIFIC_OFFSET		0x0014

//device status
#define VIRTIO_STATUS_RESET				0x00
#define VIRTIO_STATUS_ACK				0x01
#define VIRTIO_STATUS_DRV				0x02
#define VIRTIO_STATUS_DRV_OK			0x04
#define VIRTIO_STATUS_FAIL				0x80

struct virtio_queue
{
	void* unaligned_addr;
	void* paddr;			/* physical addr of ring */
	uint32_t page;				/* physical guest page  = paddr/4096*/

	uint16_t num;				/* number of descriptors collected from device config offset*/
	uint32_t ring_size;			/* size of ring in bytes */
	struct vring vring;

	uint16_t free_num;				/* free descriptors */
	uint16_t free_head;			/* next free descriptor */
	uint16_t free_tail;			/* last free descriptor */
	uint16_t last_used;			/* we checked in used */

	void **data;				/* pointer to array of pointers */
};

// Feature description
typedef struct virtio_feature
{
	char *name;
	uint8_t bit;
	uint8_t host_support;
	uint8_t guest_support;
} virtio_feature;

typedef struct VirtIODevice
{
	PCIAddress			pciAddr;
	volatile uint16_t	io_addr;

	uint8_t				intLine;
	uint8_t				intPin;

	int32_t				num_features;
	virtio_feature*		features;

	uint16_t			num_queues;
	struct virtio_queue *queues;

} VirtIODevice_t, *pVirtIODevice;



#endif
