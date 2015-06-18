/**
 * @file mouse_device.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "vioblk_device.h"
#include "residents.h"

#define DEVICE_VERSION_STRING "\0$VER: virtio_blk.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static char DevName[] = "virtio_blk.device";
static char Version[] = "\0$VER: virtio_blk.device 0.1 ("__DATE__")\r\n";

// TEMP FIX GCC
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/*******************

Prototypes

********************/

static pVIOBlkBase dev_InitDev(pVIOBlkBase devBase, UINT32 *segList, pSysBase execBase);
static pVIOBlkBase dev_OpenDev(pVIOBlkBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags);
static pVIOBlkBase dev_CloseDev(pVIOBlkBase devBase, struct IOStdReq *ioreq);
static pVIOBlkBase dev_ExpungeDev(pVIOBlkBase devBase);
static pVIOBlkBase dev_ExtFuncDev(pVIOBlkBase devBase);
static void dev_BeginIO(pVIOBlkBase devBase, pIOStdReq io);
static void dev_AbortIO(pVIOBlkBase devBase, pIOStdReq io);

void vioblk_QueueCommand(struct IOStdReq *io);
void vioblk_EndCommand(struct IOStdReq *io, UINT32 error);
extern void (*vioblk_CmdVector[])(pIOStdReq);
extern INT8 vioblk_CmdQuick[];

/*******************

Function Table

********************/

static APTR volatile FuncTab[] =
{
	(void(*)) dev_OpenDev,
	(void(*)) dev_CloseDev,
	(void(*)) dev_ExpungeDev,
	(void(*)) dev_ExtFuncDev,

	(void(*)) dev_BeginIO,
	(void(*)) dev_AbortIO,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/

static const struct VIOBlkBase DevData =
{
	.dev_Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.dev_Device.dev_Node.ln_Type = NT_DEVICE,
	.dev_Device.dev_Node.ln_Pri = 60,
	.dev_Device.dev_OpenCnt = 0,
	.dev_Device.dev_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.dev_Device.dev_NegSize = 0,
	.dev_Device.dev_PosSize = 0,
	.dev_Device.dev_Version = DEVICE_VERSION,
	.dev_Device.dev_Revision = DEVICE_REVISION,
	.dev_Device.dev_Sum = 0,
	.dev_Device.dev_IDString = (APTR)&Version[7],
};

/*******************

ROMTAG RESIDENT

********************/

static struct InitTable
{
	UINT32	LibBaseSize;
	APTR	volatile FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} InitTab =
{
	sizeof(struct VIOBlkBase),
	FuncTab,
	(APTR)&DevData,
	dev_InitDev
};

static APTR EndResident;

volatile static RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	50,
	DevName,
	Version,
	&InitTab
};

/*******************

Virtio priv. structures

*********************/
#define MAX_VIRTIO_BLK_FEATURES 10

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

static virtio_feature blk_feature[VB_UNIT_MAX][MAX_VIRTIO_BLK_FEATURES];

/*******************

DEVICE FUNCTIONS

********************/
int VirtioBlk_alloc_phys_requests(pVIOBlkBase VirtioBlkBase, VirtioBlk *vb);
int VirtioBlk_configuration(pVIOBlkBase VirtioBlkBase, VirtioBlk *vb);
int VirtioBlk_setup(pVIOBlkBase VirtioBlkBase, VirtioBlk *vb, INT32 unit_num);
__attribute__((no_instrument_function)) BOOL VIOBlk_IRQ_Handler(UINT32 number, pVIOBlkBase devBase, APTR SysBase);

#define ExpansionBase devBase->dev_ExpansionBase
#define VIOBase devBase->dev_VIOBase

UINT32 dev_VirtioBlkTask(APTR SysBase, pVIOBlkBase devBase);
void vio_EndCommand(struct IOStdReq *io, UINT32 error);

static pVIOBlkBase dev_InitDev(pVIOBlkBase devBase, UINT32 *segList, pSysBase SysBase)
{
	(void)segList;
	devBase->dev_SysBase = SysBase;

	devBase->dev_ExpansionBase = (pExpansionBase)OpenLibrary("expansion.library", 0);
	if (!devBase->dev_ExpansionBase)
	{
		KPrintF("InitDev VirtioBlk Device: Expansion.library\n");
		return NULL;
	}
	
	devBase->dev_VIOBase = (pVIOBase)OpenLibrary("virtio.library",0);
	if (!devBase->dev_VIOBase)
	{
		CloseLibrary(devBase->dev_ExpansionBase);
		KPrintF("InitDev VirtioBlk Device: virtio.library\n");
		return NULL;
	}

	VirtioBlk 		*vb;
	pVirtIODevice	vd;
	pVIOUnit		unit;
	
	devBase->dev_MaxUnits = VB_UNIT_MAX;
	
	for (int32_t unit_num = 0; unit_num < VB_UNIT_MAX; unit_num++)
	{
		unit= &devBase->dev_Unit[unit_num];
		vb	= &unit->vb;
		vd	= &vb->vd;
		
		// 1. Setup the device
		int32_t res = VirtioBlk_setup(devBase, vb, unit_num);
		
		if (res != 1) 
		{
			KPrintF("Error setting up device\n");
			break;
		}
		
		// Reset the device
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_RESET);
		// Ack the device
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_ACK);

		//2. check if you can drive the device.
		//driver supports these features
		vd->features = &blk_feature[unit_num][0];
		vd->num_features = MAX_VIRTIO_BLK_FEATURES;
		CopyMem(blkf, vd->features, sizeof(blkf));

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
		VirtioBlk_alloc_phys_requests(devBase, vb);

		//collect configuration
		VirtioBlk_configuration(devBase, vb);

		//Driver is ready to go!
		VirtioWrite8(vd->io_addr, VIRTIO_DEV_STATUS_OFFSET, VIRTIO_STATUS_DRV_OK);
		KPrintF("IRQLine: %d/%d\n", unit_num, vd->intLine );
		
		devBase->IntHandler = CreateIntServer("IRQ VirtIO", 100, VIOBlk_IRQ_Handler, devBase);
		AddIntServer(vd->intLine, devBase->IntHandler);
	}
	
	for (int32_t unit_num = 0; unit_num < VB_UNIT_MAX; unit_num++)
	{
		NewList((struct List *)&devBase->dev_Unit[unit_num].unit.unit_MsgPort.mp_MsgList);
		devBase->dev_Unit[unit_num].unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)unit_num;//(STRPTR)DevName;
		devBase->dev_Unit[unit_num].unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
		devBase->dev_Unit[unit_num].unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling
		devBase->dev_Unit[unit_num].unit.unit_Flags &= ~DUB_STOPPED;
	}

	devBase->dev_BootTask	= FindTask(NULL);
	devBase->dev_Task = CreateTask("VirtIO-Blk-Task", (Task_Function)dev_VirtioBlkTask, devBase, 8192, 0);
	if (devBase->dev_Task == NULL) KPrintF("ERROR: virtioblk.device Task not created\n");
	ReadyTask(devBase->dev_Task, TRUE);
	WaitSignal(SIGF_SINGLE);

	return devBase;
}

#define SysBase devBase->dev_SysBase

static pVIOBlkBase dev_OpenDev(pVIOBlkBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags)
{
	ioreq->io_Error = IOERR_OPENFAIL;
	
	if (unitNum < VB_UNIT_MAX) 
	{
		devBase->dev_Device.dev_OpenCnt++;
		devBase->dev_Device.dev_Flags &= ~LIBF_DELEXP;
		ioreq->io_Flags = flags;
		ioreq->io_Error = 0;
		ioreq->io_Unit	= (pUnit)&devBase->dev_Unit[unitNum];
		ioreq->io_Device= (Device *)devBase;
	} else
	{
		ioreq->io_Error = BLK_ERR_BadUnitNum;
	}
	return devBase;
}

static pVIOBlkBase dev_CloseDev(pVIOBlkBase devBase, struct IOStdReq *ioreq)
{
	devBase->dev_Device.dev_OpenCnt--;
	if(!devBase->dev_Device.dev_OpenCnt)
	{
		// Should we "expunge" the device?
	}
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return (devBase);
}

static pVIOBlkBase dev_ExpungeDev(pVIOBlkBase devBase)
{
	(void)devBase;
	return (NULL);
}

static pVIOBlkBase dev_ExtFuncDev(pVIOBlkBase devBase)
{
	(void)devBase;
	return (NULL);
}

static void dev_BeginIO(pVIOBlkBase devBase, pIOStdReq io)
{
	(void)devBase;
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > MAX_CMD) cmd = 0; // Invalidate the command.

#if 0
	if (vioblk_CmdQuick[cmd] >= 0)
	{
		vioblk_QueueCommand((struct IOStdReq*)io);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		
		if ((io->io_Unit->unit_Flags & DUB_STOPPED) != 0)
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
	}
#endif
//KPrintF("Calling CommandVector with %d\n", cmd);
	vioblk_CmdVector[cmd](io);
}

static void dev_AbortIO(pVIOBlkBase devBase, pIOStdReq io)
{
	(void)devBase;
	vio_EndCommand(io, IOERR_ABORTED);
}
