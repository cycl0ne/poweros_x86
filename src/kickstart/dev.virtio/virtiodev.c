/*
** Virtio Std. Library Vectors
** 
** 
*/

#include "virtiodev.h"

APTR vdev_OpenDev(struct VDBase *VDBase, struct IORequest *ioreq, UINT32 unitNum, UINT32 flags)
{
	ioreq->io_Error = IOERR_OPENFAIL;
	if(unitNum < VDBase->vd_MaxUnit)
	{
		VDBase->vd_Device.dd_Library.lib_OpenCnt++;
		VDBase->vd_Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;

		ioreq->io_Error = 0;
		ioreq->io_Unit	= (struct  Unit *)&VDBase->vd_Unit[unitNum];
		ioreq->io_Device= (struct Device *)VDBase;
	}
	return VDBase;
}

APTR vdev_CloseDev(struct VDBase *VDBase, struct IORequest *ioreq)
{
	VDBase->vd_Device.dd_Library.lib_OpenCnt--;
	ioreq->io_Unit	= NULL;
	ioreq->io_Device= NULL;
	return VDBase;
}

APTR vdev_ExpungeDev(struct VDBase *VDBase)
{
	return NULL;
}

APTR vdev_ExtFuncDev(struct VDBase *VDBase)
{
	return NULL;
}

void vdev_BeginIO(VDBase *VDBase, struct IORequest *io)
{
	// The following code should be kept until exec/sendio error handling is fixed!
	if (io == NULL) return;
	if (io->io_Device == NULL) 
	{
		io->io_Error = IOERR_OPENFAIL;
		return;
	}
	
	UINT8 cmd = io->io_Command;
	if (cmd > HD_SCSICMD) cmd = 0; // Invalidate the command if it is bigger than HD_SCSICMD
	virtioBlkCmdVector[cmd]((struct IOStdReq*) io);
}

void vdev_AbortIO(VDBase *VDBase, struct IORequest *ioreq)
{
	vd_EndCommand((struct IOStdReq*)ioreq, IOERR_ABORTED);
}
