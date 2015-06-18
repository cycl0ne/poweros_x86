#include "vioblk_device.h"

#define VIOBase devBase->dev_VIOBase

__attribute__((no_instrument_function)) BOOL VIOBlk_IRQ_Handler(UINT32 number, pVIOBlkBase devBase, APTR SysBase)
{
	(void)number;
	VirtioBlk 		*vb;
	pVirtIODevice	vd;
	pVIOUnit		unit;
	int32_t			unitNum;
//	KPrintF("VIO - IRQ!\n");
	
	for (unitNum = 0; unitNum < devBase->dev_MaxUnits; unitNum++)
	{
		unit = &devBase->dev_Unit[unitNum];
		vb = &unit->vb;
		vd = &vb->vd;
			
		BOOL isr = VirtioRead8(vd->io_addr, VIRTIO_ISR_STATUS_OFFSET);
		if (isr == 1)
		{
			unit->gotIRQ = TRUE;
			SignalTask(devBase->dev_Task, devBase->dev_signalTask);
			return TRUE;
		}
	}
	return FALSE;
}

