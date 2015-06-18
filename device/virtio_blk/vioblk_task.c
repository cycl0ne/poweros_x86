#include "vioblk_device.h"

#define SysBase ((pVIOBlkBase)io->io_Device)->dev_SysBase
#define DEVBase ((pVIOBlkBase)io->io_Device)
void VirtioBlk_transfer(pVIOBlkBase VirtioBlkBase, VirtioBlk* vb, UINT32 sector_start, UINT32 num_sectors, UINT8 write, UINT8* buf);

void vio_EndCommand(struct IOStdReq *io, UINT32 error)
{
	io->io_Error = error;
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
	ReplyMsg(&io->io_Message);
	return;	
}

static void Invalid(struct IOStdReq *io)
{
    vio_EndCommand(io, IOERR_NOCMD);
}

// TODO: Check ALL! (sector, length, diskgeometry, ....)

static void SendToTask(struct IOStdReq *io)
{
	io->io_Flags &= ~IOF_QUICK;
	io->io_Actual = io->io_Error = 0;
	//KPrintF("PutMsg\n");
	PutMsg(DEVBase->dev_Port, (pMessage) io);
}

void (*vioblk_CmdVector[])(struct IOStdReq *) = 
{
	Invalid,
	Invalid,//	Reset, 
	SendToTask,//	Read, 
	SendToTask,//	Write, 
	Invalid,//	Update, 
	Invalid,//	Clear,
	Invalid,//	StopCmd, 
	Invalid,//	Start, 
	Invalid,//	Flush, 
	Invalid,// getDeviceInfo
	Invalid,// GetDiskChangeCount
	Invalid,// 	GetDiskPresenceStatus
	Invalid,// 	VB_EJECT
	Invalid,// 	VB_FORMAT
	Invalid,// 	VB_GETNUMTRACKS
	Invalid,// 	VB_WRITEPROTECTIONSTATUS
	Invalid,// 	TD_ADDCHANGEINT
	Invalid,// 	TD_REMCHANGEINT
};

INT8 vioblk_CmdQuick[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1
};


static inline void QueueCommand(struct IOStdReq *io)
{
    pVIOUnit vioUnit = (pVIOUnit)io->io_Unit;
	struct Unit	*unit = &vioUnit->unit;
	
	// Clear Flags needed
	io->io_Flags &= ~(IOF_CURRENT|IOF_QUEUED);
//    KPrintF("Unit#: %d\n", unit->unit_MsgPort.mp_Node.ln_Name);
	if (io->io_Error == 0) 
	{
		//PutMsg(&unit->unit_MsgPort, &io->io_Message);
        AddTail(&unit->unit_MsgPort.mp_MsgList, (pNode)&io->io_Message);

		if (GetHead(&unit->unit_MsgPort.mp_MsgList) != &io->io_Message.mn_Node) 
		{
			SET_BITS(io->io_Flags, IOF_QUEUED);
		} else {
			SET_BITS(io->io_Flags, IOF_CURRENT);
		}
	}
}

static void vio_Read(struct IOStdReq *io)
{
    // Queue our command
    QueueCommand(io);

    // if we are the first, then we issue a read to device
    if (io->io_Flags & IOF_CURRENT)
    {
        //do a virtio call
        VirtioBlk_transfer((pVIOBlkBase)io->io_Device, &((pVIOUnit)(io->io_Unit))->vb, io->io_Offset, io->io_Length, VB_READ, io->io_Data);
        return;
    }
    // We are not first, so we just return we will be picked up by the IRQ.
    return;
}

static void vio_Write(struct IOStdReq *io)
{
    // Queue our command
    QueueCommand(io);

    // if we are the first, then we issue a read to device
    if (io->io_Flags & IOF_CURRENT)
    {
        //do a virtio call
        VirtioBlk_transfer((pVIOBlkBase)io->io_Device, &((pVIOUnit)(io->io_Unit))->vb,io->io_Offset, io->io_Length, VB_WRITE, io->io_Data);
        return;
    }
    // We are not first, so we just return
    return;
}

#undef SysBase
#define SysBase devBase->dev_SysBase

static void vio_HandleIRQ(pVIOBlkBase devBase)
{
    pVIOUnit    unit= NULL;
    pIOStdReq   io  = NULL;
    
    // Check which unit got the IRQ
    for (uint32_t i=0; i < VB_UNIT_MAX; i++)  // we could change this to just the devices mounted!
    {
        unit = &devBase->dev_Unit[i];
        
        // we have the IRQ? then handle it
        if (unit->gotIRQ)
        {
            io = (pIOStdReq)RemHead(&unit->unit.unit_MsgPort.mp_MsgList);
            if (io != NULL)
            {
                if (io->io_Flags & IOF_CURRENT)
                {
                    io->io_Flags &= ~IOF_CURRENT;
                    io->io_Actual = io->io_Length; // This might be wrong!
                    vio_EndCommand(io, 0);
                    
                    // Check for other requests
                    io = (pIOStdReq) RemHead(&unit->unit.unit_MsgPort.mp_MsgList);
                    if (io)
                    {
                        if (io->io_Command == CMD_READ)
                        {
                            vio_Read(io);
                        } else if (io->io_Command == CMD_WRITE)
                        {
                            vio_Write(io);
                        } else
                        {
                            KPrintF("Error: Wrong command. Device might be broken now\n");
                            QueueCommand(io);
                        }
                    }
                }
            }
            unit->gotIRQ = FALSE;
        }
    }
}

#undef SysBase
UINT32 dev_VirtioBlkTask(APTR SysBase, pVIOBlkBase devBase)
{
	pIOStdReq	msg = NULL;
	
	devBase->dev_Port = CreateMsgPort(NULL);
	if (devBase->dev_Port == NULL)
	{
		KPrintF("PANIC: VirtioTask Error\n");
		for(;;);
	}
	
	uint8_t sig = AllocSignal(-1);
    devBase->dev_signalTask = (1<<sig);
    
    uint32_t signalPort = (1<<devBase->dev_Port->mp_SigBit);    // cache it here
    uint32_t signalIRQ  = devBase->dev_signalTask;              // just cache it here
    uint32_t signalRcvd = 0;                                    // Our signal received
    
	SignalTask(devBase->dev_BootTask, SIGF_SINGLE);
//	KPrintF("%x SysBase", devBase->dev_SysBase);
//	KPrintF("%x SysBase2", SysBase);
	
	for (;;)
	{
        signalRcvd = WaitSignal(signalPort|signalIRQ);
//        KPrintF("VTask: got Signal\n");
        // Message on our port?
        if (signalRcvd & signalPort)
        {
//            KPrintF("VTask: PortMsg-");
            while ( (msg = (pIOStdReq)GetMsg(devBase->dev_Port)))
	    	{
    			switch (msg->io_Command)
    			{
    				case CMD_READ:
//    				    KPrintF("Read\n");
    					vio_Read(msg);
    					break;
    				case CMD_WRITE:
//    				    KPrintF("Write\n");
    					vio_Write(msg);
    					break;
    				default:
    					vio_EndCommand(msg, -1);
        				break;
    			}
    		}
        } else if (signalRcvd & signalIRQ)
        {
            // coming from IRQ
            vio_HandleIRQ(devBase);
        } else
        {
            KPrintF("Got strange signal in virtio task\n");
        }
//        KPrintF("going to sleep\n");
    }
	return 0;
}

