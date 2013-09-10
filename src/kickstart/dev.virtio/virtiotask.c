/*
** Virtio Worker Task
** 
** 
*/

#include "virtiodev.h"
#if 0
static INT8 id_InitMsgPort(VDBase *VDBase, struct MsgPort *mport)
{
	INT8 sb;
	sb = AllocSignal(-1);
	mport->mp_SigBit = sb;
	mport->mp_Flags  = PA_SIGNAL;
	mport->mp_Node.ln_Type = NT_MSGPORT;
	mport->mp_SigTask = (struct Task *)FindTask(NULL);
	//DPrintF("MsgPort Task: %x [%s]\n",ret->mp_SigTask, ret->mp_SigTask->Node.ln_Name);
	NewListType(&mport->mp_MsgList, NT_MSGPORT);
	return sb;
}

UINT32 vdev_WorkerTask(VDBase *VDBase, APTR SysBase)
{
	INT8	signalUnit;
	UINT32	sigUnit;
	
	signalUnit = id_InitMsgPort(VDBase, &VDBase->vd_MsgPort);
	for (int i=0; i< VDBase->vd_MaxUnit; i++) VDBase->vd_Unit[i].unit_MsgPort = &VDBase->vd_MsgPort;
	Signal(VDBase->vd_BootTask, SIGF_SINGLE);
	sigUnit = (1<<signalUnit);
	Wait(sigUnit);

	while(1)
	{
		if (CheckForMsg(VDBase->vd_MsgPort))
		{
			struct IOStdReq *tmp = (struct IOStdReq *)GetHead(VDBase->vd_MsgPort.mp_MsgList);
			tmp->io_Flags &= ~IOB_QUICK;
			switch(tmp->io_Command)
			{
				//case VD_READ:
				//	break;
				default:
					break;
			}
		}
		Wait(sigUnit);
	}
	
	for(;;);
	return 0;
}

#endif
