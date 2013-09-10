#include "exec_funcs.h"
#include "lib_virtio_internal.h"


#define SysBase LibVirtioBase->SysBase

struct LibVirtioBase* lib_virtio_OpenLib(struct LibVirtioBase *LibVirtioBase)
{
    LibVirtioBase->Library.lib_OpenCnt++;

	return(LibVirtioBase);
}

APTR lib_virtio_CloseLib(struct LibVirtioBase *LibVirtioBase)
{
	LibVirtioBase->Library.lib_OpenCnt--;

	return (LibVirtioBase);
}

APTR lib_virtio_ExpungeLib(struct LibVirtioBase *LibVirtioBase)
{
	return (NULL);
}

APTR lib_virtio_ExtFuncLib(void)
{
	return (NULL);
}


//**************


void lib_virtio_Write8(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT8 val)
{
	IO_Out8(base+offset, val);
}
void lib_virtio_Write16(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT16 val)
{
	IO_Out16(base+offset, val);
}
void lib_virtio_Write32(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset, UINT32 val)
{
	IO_Out32(base+offset, val);
}

UINT8 lib_virtio_Read8(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset)
{
	return IO_In8(base+offset);
}
UINT16 lib_virtio_Read16(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset)
{
	return IO_In16(base+offset);
}
UINT32 lib_virtio_Read32(struct LibVirtioBase *LibVirtioBase, UINT16 base, UINT16 offset)
{
	return IO_In32(base+offset);
}


//******************


void lib_virtio_ExchangeFeatures(LibVirtioBase *LibVirtioBase, VirtioDevice *vd)
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
		DPrintF("guest feature %d\n", (f->guest_support << f->bit));

		// just load the host/device feature int the struct
		f->host_support |=  ((host_features >> f->bit) & 1);
		DPrintF("host feature %d\n\n", ((host_features >> f->bit) & 1));
	}

	// let the device know about our features
	VirtioWrite32(vd->io_addr, VIRTIO_GUEST_F_OFFSET, guest_features);
}


//*************

int lib_virtio_AllocateQueues(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, INT32 num_queues)
{
	int r = 1;

	// Assume there's no device with more than 256 queues
	if (num_queues < 0 || num_queues > 256)
	{
		DPrintF("Invalid num_queues!\n");
		return 0;
	}

	vd->num_queues = num_queues;

	// allocate queue memory
	vd->queues = AllocVec(num_queues * sizeof(vd->queues[0]), MEMF_FAST|MEMF_CLEAR);

	if (vd->queues == NULL)
	{
		DPrintF("Couldn't allocate memory for %d queues\n", vd->num_queues);
		return 0;
	}

	//not needed because of MEMF_CLEAR
	memset(vd->queues, 0, num_queues * sizeof(vd->queues[0]));

	return r;
}


int lib_virtio_InitQueues(LibVirtioBase *LibVirtioBase, VirtioDevice *vd)
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
		DPrintF("Queue %d, q->num (%d)\n", i, q->num);
		if (q->num & (q->num - 1)) {
			DPrintF("Queue %d num=%d not ^2\n", i, q->num);
			r = 0;
			goto free;
		}

		r = LibVirtio_alloc_phys_queue(LibVirtioBase,q);

		if (r != 1)
		{
			DPrintF("Couldn't allocate queue number %d\n", i);
			goto free;
		}

		LibVirtio_init_phys_queue(LibVirtioBase, q);

		//Let the host know about the guest physical page
		VirtioWrite32(vd->io_addr, VIRTIO_QADDR_OFFSET, q->page);
	}

	return 1;

free:
	for (j = 0; j < i; j++)
	{
		LibVirtio_free_phys_queue(LibVirtioBase, &vd->queues[i]);
	}

	return r;
}



void lib_virtio_FreeQueues(LibVirtioBase *LibVirtioBase, VirtioDevice *vd)
{
	int i;
	for (i = 0; i < vd->num_queues; i++)
	{
		LibVirtio_free_phys_queue(LibVirtioBase, &vd->queues[i]);
	}

	FreeVec(vd->queues);
	vd->queues = 0;
}


int lib_virtio_HostSupports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit)
{
	return LibVirtio_supports(LibVirtioBase, vd, bit, 1);
}

int lib_virtio_GuestSupports(LibVirtioBase *LibVirtioBase, VirtioDevice *vd, int bit)
{
	return LibVirtio_supports(LibVirtioBase, vd, bit, 0);
}


