#include "virtiodev.h"

#define SysBase ((struct VDBase *)io->io_Device)->vd_SysBase

void vd_EndCommand(struct IOStdReq *io, UINT32 error)
{
	io->io_Error = error;
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
	ReplyMsg(&io->io_Message);
	return;	
}

#undef SysBase
#define SysBase VDBase->vd_SysBase
#define ExpansionBase VDBase->vd_ExpansionBase
#define LibVirtioBase VDBase->vd_VirtIOBase

#include "expansion_funcs.h"

BOOL vd_VirtioBlk_setup(struct VDBase *VDBase, VirtioBlk *vb, INT32 unit_num)
{
	VirtioDevice* vd = &(vb->vd);

	if (!PCIFindDeviceByUnit(VIRTIO_VENDOR_ID, VIRTIO_BLK_DEVICE_ID, &(vd->pciAddr), unit_num)) {
		DPrintF("VirtioBlk_setup: No Virtio device found.\n");
		return FALSE;
	} else
	{
		DPrintF("VirtioBlk_setup: Virtio block device found.\n");
	}

	DPrintF("VirtioBlk_setup: (vd->pciAddr).bus %x\n", (vd->pciAddr).bus);
	DPrintF("VirtioBlk_setup: (vd->pciAddr).device %x\n", (vd->pciAddr).device);
	DPrintF("VirtioBlk_setup: (vd->pciAddr).function %x\n", (vd->pciAddr).function);

	PCISetMemEnable(&vd->pciAddr, TRUE);
	vd->io_addr = PCIGetBARAddr(&vd->pciAddr, 0);
	DPrintF("VirtioBlk_setup: ioAddress %x\n", vd->io_addr);

	vd->intLine = PCIGetIntrLine(&vd->pciAddr);
	DPrintF("VirtioBlk_setup: intLine %x\n", vd->intLine);

	vd->intPin = PCIGetIntrPin(&vd->pciAddr);
	DPrintF("VirtioBlk_setup: intPin %x\n", vd->intPin);

	return TRUE;
}

BOOL vd_VirtioBlk_alloc_phys_requests(VDBase *VDBase, VirtioBlk *vb)
{
	/* Allocate memory for request headers and status field */

	vb->hdrs = AllocVec(VIRTIO_BLK_NUM_QUEUES * sizeof(vb->hdrs[0]), MEMF_FAST|MEMF_CLEAR);

	if (vb->hdrs == NULL)
	{
		DPrintF("Couldn't allocate memory for vb->hdrs\n");
		return FALSE;
	}

	vb->status = AllocVec(VIRTIO_BLK_NUM_QUEUES * sizeof(vb->status[0]), MEMF_FAST|MEMF_CLEAR);

	if (vb->status == NULL)
	{
		FreeVec(vb->hdrs);
		DPrintF("Couldn't allocate memory for vb->status\n");
		return FALSE;
	}

	return TRUE;
}

BOOL vd_VirtioBlk_configuration(VDBase *VDBase, VirtioBlk *vb)
{
	VirtioDevice* vd = &(vb->vd);
	UINT32 sectors_low, sectors_high, size_mbs;

	/* capacity is always there */
	sectors_low	= VirtioRead32(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+0);
	sectors_high= VirtioRead32(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+4);

	vb->Info.capacity = ((UINT64)sectors_high << 32) | sectors_low;

	/* If this gets truncated, you have a big disk... */
	size_mbs = (UINT32)(vb->Info.capacity * 512 / 1024 / 1024);
	DPrintF("Capacity: %d MB\n", size_mbs);

	if (VirtioHostSupports(vd, VIRTIO_BLK_F_GEOMETRY))
	{
		vb->Info.geometry.cylinders = VirtioRead16(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+16);
		vb->Info.geometry.heads = VirtioRead8(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+18);
		vb->Info.geometry.sectors = VirtioRead8(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+19);

		DPrintF("Geometry: cyl=%d heads=%d sectors=%d\n",
					vb->Info.geometry.cylinders,
					vb->Info.geometry.heads,
					vb->Info.geometry.sectors);
	}

	if (VirtioHostSupports(vd, VIRTIO_BLK_F_BLK_SIZE))
	{
		vb->Info.blk_size = VirtioRead32(vd->io_addr, VIRTIO_DEV_SPECIFIC_OFFSET+20);
		DPrintF("Block Size: %d\n", vb->Info.blk_size);
	}

	return TRUE;
}


