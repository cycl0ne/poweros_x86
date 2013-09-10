#include "exec_funcs.h"
#include "lib_virtio_internal.h"

#define SysBase LibVirtioBase->SysBase

int LibVirtio_supports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit, int host)
{
	for (int i = 0; i < vd->num_features; i++)
	{
		struct virtio_feature *f = &vd->features[i];

		if (f->bit == bit)
			return host ? f->host_support : f->guest_support;
	}
	DPrintF("ERROR: Bit not found!!\n");
	return 0;
}

//*******************


int LibVirtio_alloc_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q)
{
	/* How much memory do we need? */
	q->ring_size = vring_size(q->num, 4096);
	DPrintF("q->ring_size (%d)\n", q->ring_size);

	UINT32 addr;
	q->unaligned_addr = AllocVec(q->ring_size, MEMF_FAST|MEMF_CLEAR);
	if (q->unaligned_addr == NULL)
	{
		DPrintF("Couln't allocate memory for queue, of size %d\n", q->ring_size);
		return 0;
	}
	DPrintF("q->unaligned_addr (%x)\n", q->unaligned_addr);
	DPrintF("q->unaligned_addr + q->ring_size = (%x)\n", q->unaligned_addr + q->ring_size);

	addr = (UINT32)q->unaligned_addr & 4095;
	DPrintF("addr (%x)\n", addr);
	addr = (UINT32)q->unaligned_addr - addr;
	DPrintF("addr (%x)\n", addr);
	addr = addr + 4096;
	DPrintF("addr (%x)\n", addr);

	q->paddr = (void*)addr;

	q->data = AllocVec(sizeof(q->data[0]) * q->num, MEMF_FAST|MEMF_CLEAR);

	if (q->data == NULL)
	{
		FreeVec(q->unaligned_addr);
		q->unaligned_addr = 0;
		q->paddr = 0;
		DPrintF("Couln't allocate memory for data pointers, of size %d\n", q->num);
		return 0;
	}

	return 1;
}

void LibVirtio_init_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q)
{
	//not needed because of MEMF_CLEAR
	memset(q->unaligned_addr, 0, q->ring_size);
	memset(q->data, 0, sizeof(q->data[0]) * q->num);

	/* physical page in guest */
	q->page = (UINT32)q->paddr / 4096;

	/* Set pointers in q->vring according to size */
	vring_init(&q->vring, q->num, q->paddr, 4096);

	DPrintF("vring desc %x\n", q->vring.desc);
	DPrintF("vring avail %x\n", q->vring.avail);
	DPrintF("vring used %x\n", q->vring.used);

	return;
}

void LibVirtio_free_phys_queue(LibVirtioBase *LibVirtioBase, struct virtio_queue *q)
{
	FreeVec(q->unaligned_addr);
	q->unaligned_addr = 0;
	q->paddr = 0;

	FreeVec(q->data);
	q->data = 0;
}
