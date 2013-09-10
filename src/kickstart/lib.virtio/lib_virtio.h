#ifndef lib_virtio_h
#define lib_virtio_h

// this file shall go to top most include folder in future

#include "types.h"
#include "virtio_ring.h"
#include "expansionbase.h"

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
#define VIRTIO_STATUS_RESET			0x00
#define VIRTIO_STATUS_ACK			0x01
#define VIRTIO_STATUS_DRV			0x02
#define VIRTIO_STATUS_DRV_OK		0x04
#define VIRTIO_STATUS_FAIL			0x80

struct virtio_queue
{
	void* unaligned_addr;
	void* paddr;			/* physical addr of ring */
	UINT32 page;				/* physical guest page  = paddr/4096*/

	UINT16 num;				/* number of descriptors collected from device config offset*/
	UINT32 ring_size;			/* size of ring in bytes */
	struct vring vring;

	UINT16 free_num;				/* free descriptors */
	UINT16 free_head;			/* next free descriptor */
	UINT16 free_tail;			/* last free descriptor */
	UINT16 last_used;			/* we checked in used */

	void **data;				/* pointer to array of pointers */
};

// Feature description
typedef struct virtio_feature
{
	char *name;
	UINT8 bit;
	UINT8 host_support;
	UINT8 guest_support;
} virtio_feature;


typedef struct VirtioDevice
{
	PCIAddress		pciAddr;
	volatile UINT16			io_addr;

	UINT8 intLine;
	UINT8 intPin;

	int num_features;
	virtio_feature*   features;

	UINT16 num_queues;
	struct virtio_queue *queues;

} VirtioDevice;


typedef struct LibVirtioBase
{
	struct Library		Library;
	APTR				SysBase;

} LibVirtioBase;


//library functions
void VirtioWrite8(UINT16 base, UINT16 offset, UINT8 val);
void VirtioWrite16(UINT16 base, UINT16 offset, UINT16 val);
void VirtioWrite32(UINT16 base, UINT16 offset, UINT32 val);
UINT8 VirtioRead8(UINT16 base, UINT16 offset);
UINT16 VirtioRead16(UINT16 base, UINT16 offset);
UINT32 VirtioRead32(UINT16 base, UINT16 offset);

void VirtioExchangeFeatures(VirtioDevice *vd);
int VirtioAllocateQueues(VirtioDevice *vd, INT32 num_queues);
int VirtioInitQueues(VirtioDevice *vd);
void VirtioFreeQueues(VirtioDevice *vd);
int VirtioHostSupports(VirtioDevice *vd, int bit);
int VirtioGuestSupports(VirtioDevice *vd, int bit);



//vectors
#define VirtioWrite8(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT8)) 	_GETVECADDR(LibVirtioBase, 5))(LibVirtioBase, a, b, c))
#define VirtioWrite16(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 6))(LibVirtioBase, a, b, c))
#define VirtioWrite32(a,b,c) (((void(*)(APTR, UINT16, UINT16, UINT32)) 	_GETVECADDR(LibVirtioBase, 7))(LibVirtioBase, a, b, c))
#define VirtioRead8(a,b) (((UINT8(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 8))(LibVirtioBase, a, b))
#define VirtioRead16(a,b) (((UINT16(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 9))(LibVirtioBase, a, b))
#define VirtioRead32(a,b) (((UINT32(*)(APTR, UINT16, UINT16)) 	_GETVECADDR(LibVirtioBase, 10))(LibVirtioBase, a, b))

#define VirtioExchangeFeatures(a) (((void(*)(APTR, VirtioDevice*)) 	_GETVECADDR(LibVirtioBase, 11))(LibVirtioBase, a))
#define VirtioAllocateQueues(a,b) (((int(*)(APTR, VirtioDevice*, INT32)) 	_GETVECADDR(LibVirtioBase, 12))(LibVirtioBase, a, b))
#define VirtioInitQueues(a) (((int(*)(APTR, VirtioDevice*)) 	_GETVECADDR(LibVirtioBase, 13))(LibVirtioBase, a))
#define VirtioFreeQueues(a) (((void(*)(APTR, VirtioDevice*)) 	_GETVECADDR(LibVirtioBase, 14))(LibVirtioBase, a))
#define VirtioHostSupports(a,b) (((int(*)(APTR, VirtioDevice*, int)) 	_GETVECADDR(LibVirtioBase, 15))(LibVirtioBase, a, b))
#define VirtioGuestSupports(a,b) (((int(*)(APTR, VirtioDevice*, int)) 	_GETVECADDR(LibVirtioBase, 16))(LibVirtioBase, a, b))




#endif //lib_virtio_h
