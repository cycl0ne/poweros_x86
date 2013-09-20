#include "virtiodev.h"
#include "sysbase.h"
#include "resident.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"
#include "lib_virtio.h"
#include "arch_config.h"
#include "virtioblk.h"

#define DEVICE_VERSION_STRING "\0$VER: virtioblk.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1
#define MAX_VIRTIO_BLK_FEATURES 10

static const char name[] = "virtioblk.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

static virtio_feature blkf[MAX_VIRTIO_BLK_FEATURES] = {
	{ "barrier",	VIRTIO_BLK_F_BARRIER,	0,	0 	},
	{ "sizemax",	VIRTIO_BLK_F_SIZE_MAX,	0,	0	},
	{ "segmax",		VIRTIO_BLK_F_SEG_MAX,	0,	0	},
	{ "geometry",	VIRTIO_BLK_F_GEOMETRY,	0,	0	},
	{ "read-only",	VIRTIO_BLK_F_RO,		0,	0	},
	{ "blocksize",	VIRTIO_BLK_F_BLK_SIZE,	0,	0	},
	{ "scsi",		VIRTIO_BLK_F_SCSI,		0,	0	},
	{ "flush",		VIRTIO_BLK_F_FLUSH,		0,	0	},
	{ "topology",	VIRTIO_BLK_F_TOPOLOGY,	0,	0	},
	{ "idbytes",	VIRTIO_BLK_ID_BYTES,	0,	0	}
};

static virtio_feature blk_feature[MAX_VBLK][MAX_VIRTIO_BLK_FEATURES];

APTR vdev_OpenDev(struct VDBase *VDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR vdev_CloseDev(struct VDBase *VDBase, struct IORequest *ioreq);
APTR vdev_ExpungeDev(struct VDBase *VDBase);
APTR vdev_ExtFuncDev(struct VDBase *VDBase);
void vdev_BeginIO(VDBase *VDBase, struct IORequest *ioreq);
void vdev_AbortIO(VDBase *VDBase, struct IORequest *ioreq);
static struct VDBase *vdev_Init(struct VDBase *VDBase, UINT32 *segList, struct SysBase *SysBase);

static APTR FuncTab[] = 
{
	(void(*)) vdev_OpenDev,
	(void(*)) vdev_CloseDev,
	(void(*)) vdev_ExpungeDev,
	(void(*)) vdev_ExtFuncDev,

	(void(*)) vdev_BeginIO,
	(void(*)) vdev_AbortIO,
	(APTR) ((UINT32)-1)
};

static const struct VDBase VDevData =
{
	.vd_Device.dd_Library.lib_Node.ln_Name = (APTR)&name[0],
	.vd_Device.dd_Library.lib_Node.ln_Type = NT_DEVICE,
	.vd_Device.dd_Library.lib_Node.ln_Pri = 50,
	.vd_Device.dd_Library.lib_OpenCnt = 0,
	.vd_Device.dd_Library.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED,
	.vd_Device.dd_Library.lib_NegSize = 0,
	.vd_Device.dd_Library.lib_PosSize = 0,
	.vd_Device.dd_Library.lib_Version = DEVICE_VERSION,
	.vd_Device.dd_Library.lib_Revision = DEVICE_REVISION,
	.vd_Device.dd_Library.lib_Sum = 0,
	.vd_Device.dd_Library.lib_IDString = (APTR)&version[7],
	.vd_MaxUnit = 0,
};

static volatile const APTR InitTab[4]=
{
	(APTR)sizeof(struct VDBase),
	(APTR)FuncTab,
	(APTR)&VDevData,
	(APTR)vdev_Init
};

static volatile const struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	40,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

UINT32 vd_WorkerTask(VDBase *VDBase, APTR SysBase);
#define LibVirtioBase VDBase->vd_VirtIOBase

static struct VDBase *vdev_Init(struct VDBase *VDBase, UINT32 *segList, struct SysBase *SysBase)
{
	VDBase->vd_SysBase	= (APTR)SysBase;
	VDBase->vd_BootTask = FindTask(NULL);

	VDBase->vd_ExpansionBase = OpenLibrary("expansion.library", 0);
	if (VDBase->vd_ExpansionBase == NULL) {
		DPrintF("virtio_blk_InitDev: Cant open expansion.library\n");
		return NULL;
	}

	VDBase->vd_VirtIOBase = OpenLibrary("virtio.library", 0);
	if (VDBase->vd_VirtIOBase == NULL) {
		DPrintF("virtio_blk_InitDev: Cant open virtio.library\n");
		return NULL;
	}

	for (UINT32 unitNum = 0; unitNum < MAX_VBLK; unitNum++)
	{
		VirtioBlk *vb = &((VDBase->vd_Unit[unitNum]).vbu_vb);
		VirtioDevice *vd = &(vb->vd);
		
		int res = vd_VirtioBlk_setup(VDBase, vb, unitNum);
		if (res != 1) break;

		// Reset the device
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_RESET);

		// Ack the device
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_ACK);

		vd->features = &blk_feature[unitNum][0];
		vd->num_features = MAX_VIRTIO_BLK_FEATURES;
		memcpy(vd->features, blkf, sizeof(blkf));

		//exchange features
		VirtioExchangeFeatures(vd);
		// We know how to drive the device...
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV);

		//3. be ready to go.
		// virtio blk has only 1 queue
		VirtioAllocateQueues(vd, VIRTIO_BLK_NUM_QUEUES);
		//init queues
		VirtioInitQueues(vd);
		//Allocate memory for headers and status
		vd_VirtioBlk_alloc_phys_requests(VDBase, vb);
		//collect configuration
		vd_VirtioBlk_configuration(VDBase, vb);
		//Driver is ready to go!
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV_OK);
		//collect device info for future uses
		vd_VirtioBlk_configuration(VDBase, vb);
		VDBase->vd_MaxUnit++;
	}

	DPrintF("Creating Worker Task\n");
	VDBase->vd_Task = TaskCreate("Virtio_BLK WorkerTask", vd_WorkerTask, VDBase, 4096, 10);
	Wait(SIGF_SINGLE);
	
	DPrintF("VirtioBlk: MaxUnits: %d\n", VDBase->vd_MaxUnit);
	return VDBase;
}

static const char EndResident = 0;
