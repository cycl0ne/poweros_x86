/*
** Virtio Library Functions
** 
**
*/
#include "virtio_lib.h"

#define SysBase VIOBase->SysBase

static int Virtio_supports(pVIOBase VIOBase, pVirtIODevice vd, int bit, int host);
static int Virtio_alloc_phys_queue(pVIOBase VIOBase, struct virtio_queue *q);
static void Virtio_init_phys_queue(pVIOBase VIOBase, struct virtio_queue *q);
static void Virtio_free_phys_queue(pVIOBase VIOBase, struct virtio_queue *q);


void virtio_Write8(pVIOBase VIOBase, uint16_t base, uint16_t offset, uint8_t val)
{
	(void)VIOBase;
	IO_Out8(base+offset, val);
}

void virtio_Write16(pVIOBase VIOBase, uint16_t base, uint16_t offset, uint16_t val)
{
	(void)VIOBase;
	IO_Out16(base+offset, val);
}

void virtio_Write32(pVIOBase VIOBase, uint16_t base, uint16_t offset, uint32_t val)
{
	(void)VIOBase;
	IO_Out32(base+offset, val);
}

uint8_t virtio_Read8(pVIOBase VIOBase, uint16_t base, uint16_t offset)
{
	(void)VIOBase;
	return IO_In8(base+offset);
}

uint16_t virtio_Read16(pVIOBase VIOBase, uint16_t base, uint16_t offset)
{
	(void)VIOBase;
	return IO_In16(base+offset);
}

uint32_t virtio_Read32(pVIOBase VIOBase, uint16_t base, uint16_t offset)
{
	(void)VIOBase;
	return IO_In32(base+offset);
}

/*
** 
**
**
*/

void virtio_ExchangeFeatures(pVIOBase VIOBase, pVirtIODevice vd)
{
	UINT32 guest_features = 0, host_features = 0;
	virtio_feature *f;

	//collect host features
	host_features = VirtioRead32(vd->io_addr, VIRTIO_HOST_F_OFFSET);

	for (int i = 0; i < vd->num_features; i++)
	{
		f = &vd->features[i];

		// prepare the features the guest/driver supports
		guest_features |= (f->guest_support << f->bit);
		KPrintF("guest feature %d\n", (f->guest_support << f->bit));

		// just load the host/device feature int the struct
		f->host_support |=  ((host_features >> f->bit) & 1);
		KPrintF("host feature %d\n\n", ((host_features >> f->bit) & 1));
	}

	// let the device know about our features
	VirtioWrite32(vd->io_addr, VIRTIO_GUEST_F_OFFSET, guest_features);
}


//*************

int virtio_AllocateQueues(pVIOBase VIOBase, pVirtIODevice vd, INT32 num_queues)
{
	int r = 1;

	// Assume there's no device with more than 256 queues
	if (num_queues < 0 || num_queues > 256)
	{
		KPrintF("Invalid num_queues!\n");
		return 0;
	}

	vd->num_queues = num_queues;

	// allocate queue memory
	vd->queues = AllocVec(num_queues * sizeof(vd->queues[0]), MEMF_FAST|MEMF_CLEAR);

	if (vd->queues == NULL)
	{
		KPrintF("Couldn't allocate memory for %d queues\n", vd->num_queues);
		return 0;
	}

	//not needed because of MEMF_CLEAR
	//Memset(vd->queues, 0, num_queues * sizeof(vd->queues[0]));

	return r;
}


int virtio_InitQueues(pVIOBase VIOBase, pVirtIODevice vd)
{
	//Initialize all queues
	int i, j, r;
	struct virtio_queue *q;

	for (i = 0; i < vd->num_queues; i++)
	{
		q = &vd->queues[i];

		//select the queue
		VirtioWrite16(vd->io_addr, VIRTIO_QSEL_OFFSET, i);
		q->num = VirtioRead16(vd->io_addr, VIRTIO_QSIZE_OFFSET);
		KPrintF("Queue %d, q->num (%d)\n", i, q->num);
		if (q->num & (q->num - 1)) {
			KPrintF("Queue %d num=%d not ^2\n", i, q->num);
			r = 0;
			goto free;
		}

		r = Virtio_alloc_phys_queue(VIOBase,q);

		if (r != 1)
		{
			KPrintF("Couldn't allocate queue number %d\n", i);
			goto free;
		}

		Virtio_init_phys_queue(VIOBase, q);

		//Let the host know about the guest physical page
		VirtioWrite32(vd->io_addr, VIRTIO_QADDR_OFFSET, q->page);
	}

	return 1;

free:
	for (j = 0; j < i; j++)
	{
		Virtio_free_phys_queue(VIOBase, &vd->queues[i]);
	}

	return r;
}



void virtio_FreeQueues(pVIOBase VIOBase, pVirtIODevice vd)
{
	int i;
	for (i = 0; i < vd->num_queues; i++)
	{
		Virtio_free_phys_queue(VIOBase, &vd->queues[i]);
	}

	FreeVec(vd->queues);
	vd->queues = 0;
}


int virtio_HostSupports(pVIOBase VIOBase, pVirtIODevice vd, int bit)
{
	return Virtio_supports(VIOBase, vd, bit, 1);
}

int virtio_GuestSupports(pVIOBase VIOBase, pVirtIODevice vd, int bit)
{
	return Virtio_supports(VIOBase, vd, bit, 0);
}

//INTERNAL FUNCTIONS
static int Virtio_supports(pVIOBase VIOBase, pVirtIODevice vd, int bit, int host)
{
	for (int i = 0; i < vd->num_features; i++)
	{
		struct virtio_feature *f = &vd->features[i];

		if (f->bit == bit)
			return host ? f->host_support : f->guest_support;
	}
	KPrintF ("ERROR: Bit not found!!\n");
	return 0;
}

//*******************

static int Virtio_alloc_phys_queue(pVIOBase VIOBase, struct virtio_queue *q)
{
	/* How much memory do we need? */
	q->ring_size = vring_size(q->num, 4096);
	KPrintF ("q->ring_size (%d)\n", q->ring_size);

	UINT32 addr;
	q->unaligned_addr = AllocVec(q->ring_size, MEMF_FAST|MEMF_CLEAR);
	if (q->unaligned_addr == NULL)
	{
		KPrintF ("Couln't allocate memory for queue, of size %d\n", q->ring_size);
		return 0;
	}
	KPrintF ("q->unaligned_addr (%x)\n", q->unaligned_addr);
	KPrintF ("q->unaligned_addr + q->ring_size = (%x)\n", q->unaligned_addr + q->ring_size);

	addr = (UINT32)q->unaligned_addr & 4095;
	KPrintF ("addr (%x)\n", addr);
	addr = (UINT32)q->unaligned_addr - addr;
	KPrintF ("addr (%x)\n", addr);
	addr = addr + 4096;
	KPrintF ("addr (%x)\n", addr);

	q->paddr = (void*)addr;

	q->data = AllocVec(sizeof(q->data[0]) * q->num, MEMF_FAST|MEMF_CLEAR);

	if (q->data == NULL)
	{
		FreeVec(q->unaligned_addr);
		q->unaligned_addr = 0;
		q->paddr = 0;
		KPrintF ("Couln't allocate memory for data pointers, of size %d\n", q->num);
		return 0;
	}

	return 1;
}

static void Virtio_init_phys_queue(pVIOBase VIOBase, struct virtio_queue *q)
{
	//not needed because of MEMF_CLEAR
	MemSet(q->unaligned_addr, 0, q->ring_size);
	MemSet(q->data, 0, sizeof(q->data[0]) * q->num);

	/* physical page in guest */
	q->page = (UINT32)q->paddr / 4096;

	/* Set pointers in q->vring according to size */
	vring_init(&q->vring, q->num, q->paddr, 4096);

	KPrintF ("vring desc %x\n", q->vring.desc);
	KPrintF ("vring avail %x\n", q->vring.avail);
	KPrintF ("vring used %x\n", q->vring.used);

	return;
}

static void Virtio_free_phys_queue(pVIOBase VIOBase, struct virtio_queue *q)
{
	FreeVec(q->unaligned_addr);
	q->unaligned_addr = 0;
	q->paddr = 0;

	FreeVec(q->data);
	q->data = 0;
}





