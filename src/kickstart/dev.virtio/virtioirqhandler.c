/*
** VirtIO IRQ Handler
** 
** 
*/

#include "virtiodev.h"

__attribute__((no_instrument_function)) BOOL vdIRQ(UINT32 number, VirtioBlkBase *VirtioBlkBase, APTR SysBase)
{


}
#if 0
	struct VirtioBase *VirtioBase = VirtioBlkBase->VirtioBase;

	VirtioBlk *vb;
	VirtioDevice* vd;
	struct  Unit *unit;
	struct IOStdReq *head_req;
	int unit_num;

	//find out which unit generated this irq
	for (unit_num=0; unit_num < VirtioBlkBase->NumAvailUnits; unit_num++)
	{
		vb = &((VirtioBlkBase->VirtioBlkUnit[unit_num]).vb);
		vd = &(vb->vd);

		//See if virtio device generated an interrupt(1) or not(0)
		UINT8 isr;
		isr=VirtioRead8(vd->io_addr, VIRTIO_ISR_STATUS_OFFSET);
		DPrintF("VirtioBlkIRQServer: isr= %d\n", isr);

		if (isr == 1)
		{
			//now use the found unit_num to continue processing
			unit = (struct  Unit *)&((VirtioBlkBase->VirtioBlkUnit[unit_num]).vb_unit);
			head_req = (struct IOStdReq *)GetHead(&(unit->unit_MsgPort.mp_MsgList));

			if(TEST_BITS(head_req->io_Flags, IOF_CURRENT))
			{
				CLEAR_BITS(head_req->io_Flags, IOF_CURRENT);
				SET_BITS(head_req->io_Flags, IOF_SERVICING);
			}

			DPrintF("VirtioBlkIRQServer: sent signal to unit %d\n", unit_num);
			Signal(VirtioBlkBase->VirtioBlkUnit[unit_num].VirtioBlk_WorkerTask, (1 << (VirtioBlkBase->VirtioBlkUnit[unit_num].taskWakeupSignal)));

			return 1; // was for us, dont proceed daisy chain
		}
	}
	return 0; // Not for us, proceed in daisy chain
}
#endif
