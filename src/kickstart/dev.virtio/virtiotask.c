/*
** Virtio Worker Task
** 
** 
*/

#include "virtiodev.h"

#define SysBase VDBase->vd_SysBase

static INT8 vd_InitMsgPort(VDBase *VDBase, struct MsgPort *mport)
{
	mport->mp_SigBit		= AllocSignal(-1);
	mport->mp_Flags			= PA_SIGNAL;
	mport->mp_Node.ln_Type	= NT_MSGPORT;
	mport->mp_SigTask		= (struct Task *)FindTask(NULL);
	
	NewListType(&mport->mp_MsgList, NT_MSGPORT);
	return mport->mp_SigBit;
}

void vd_VIOBLK_Transfer(VDBase *VDBase, VirtioBlk* vb, UINT32 sector_start, UINT32 num_sectors, UINT8 write, UINT8* buf)
{
	VirtioDevice* vd = &(vb->vd);

	//prepare first out_hdr, since we have only one we are using 0,
	//otherwise replace 0 by a variable
	memset(&(vb->hdrs[0]), 0, sizeof(vb->hdrs[0]));

	//for reading from disk
	vb->hdrs[0].type = VIRTIO_BLK_T_IN;

	if(write == 1)
	{
		//for writing to disk
		vb->hdrs[0].type = VIRTIO_BLK_T_OUT;
	}

	//fill up sector
	vb->hdrs[0].ioprio = 0;
	vb->hdrs[0].sector = sector_start;

	//clear status
	vb->status[0] = 1; //0 means success, 1 means error, 2 means unsupported

	//DPrintF("\n\n\nsector = %d\n", sector_start);
	//DPrintF("idx = %d\n", (vd->queues[0]).vring.avail->idx);



	//fill into descriptor table
	(vd->queues[0]).vring.desc[0].addr = (UINT32)&(vb->hdrs[0]);
	(vd->queues[0]).vring.desc[0].len = sizeof(vb->hdrs[0]);
	(vd->queues[0]).vring.desc[0].flags = VRING_DESC_F_NEXT;
	(vd->queues[0]).vring.desc[0].next = 1;



	//fillup buffer info
	(vd->queues[0]).vring.desc[1].addr = (UINT32)&buf[0];
	(vd->queues[0]).vring.desc[1].len = 512*num_sectors;
	(vd->queues[0]).vring.desc[1].flags = VRING_DESC_F_NEXT;
	//for reading sector, say that, this buffer is empty and writable
	if(write == 0)
	{
		(vd->queues[0]).vring.desc[1].flags |= VRING_DESC_F_WRITE;
	}
	(vd->queues[0]).vring.desc[1].next = 2;



	//fill up status header
	(vd->queues[0]).vring.desc[2].addr = (UINT32)&(vb->status[0]);
	(vd->queues[0]).vring.desc[2].len = sizeof(vb->status[0]);
	(vd->queues[0]).vring.desc[2].flags = VRING_DESC_F_WRITE;
	(vd->queues[0]).vring.desc[2].next = 0;



	//fill in available ring
	(vd->queues[0]).vring.avail->flags = 0; //1 mean no interrupt needed, 0 means interrupt needed
	(vd->queues[0]).vring.avail->ring[0] = 0; // 0 is the head of above request descriptor chain
	(vd->queues[0]).vring.avail->idx = (vd->queues[0]).vring.avail->idx + 3; //next available descriptor

	//notify
	VirtioWrite16(vd->io_addr, VIRTIO_QNOTFIY_OFFSET, 0); //notify that 1st queue (0) of this device has been updated

}

void vd_VIOBLK_DoIO(VDBase *VDBase, struct IOStdReq *io)
{
	VirtioBlkUnit 	*vbu = (struct VirtioBlkUnit *)io->io_Unit;
	VirtioBlk		*vb  = vbu->vbu_vb;
	UINT8			*buf = io->io_Data;
	UINT8			write = 0;

	if (io->io_Command == CMD_WRITE) write = 1;	
	vd_VIOBLK_Transfer(VDBase, vb, io->io_Offset, io->io_Length, write, buf);
}

#undef SysBase

UINT32 vd_WorkerTask(VDBase *VDBase, APTR SysBase)
{
	INT8	SigTask = vd_InitMsgPort(VDBase, &VDBase->vd_MsgPort);
	VDBase->vd_SigTask = SigTask;
	
	for (int i=0; i< VDBase->vd_MaxUnit; i++) VDBase->vd_Unit[i].vbu_MsgPort = &VDBase->vd_MsgPort;

	Signal(VDBase->vd_BootTask, SIGF_SINGLE);
	Wait(1<<SigTask);

	while(1)
	{
		if (CheckForMsg(VDBase->vd_MsgPort))
		{
		}
	}
	for(;;);
	return 0;
}
