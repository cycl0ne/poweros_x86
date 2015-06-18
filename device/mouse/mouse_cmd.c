
#include "mouse_device.h"

#define SysBase ((pMouseBase)io->io_Device)->dev_SysBase
#define devBase ((pMouseBase)io->io_Device)

void (*mouse_CmdVector[])(pIOStdReq);

void mouse_QueueCommand(struct IOStdReq *io)
{
	UINT32 ipl = Disable();
	
	struct Unit	*unit = io->io_Unit;
	// Clear Flags needed
	io->io_Flags &= ~(IOF_CURRENT|IOF_QUEUED);
	
	if (io->io_Error == 0) {
		PutMsg(&unit->unit_MsgPort, &io->io_Message);
		//DPrintF("DEBUG| Head %x, Adress Node %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head, &io->io_Message.mn_Node);

		if (GetHead(&unit->unit_MsgPort.mp_MsgList) != &io->io_Message.mn_Node) 
		{
			SET_BITS(io->io_Flags, IOF_QUEUED);
		} else {
			SET_BITS(io->io_Flags, IOF_CURRENT);
		}
	}
	Restore(ipl);
	return;			
}

void mouse_EndCommand(UINT32 error, struct IOStdReq *io)
{
	//DPrintF("EndCommand\n");
	UINT32 ipl = Disable();
	if (TEST_BITS(io->io_Flags, IOF_DONE)) 
	{
		Restore(ipl);
		return;		
	}
	SET_BITS(io->io_Flags, IOF_DONE);

	io->io_Error = error;

	if ((io->io_Flags & IOF_CURRENT) == 0) 
	{
		if ((io->io_Flags & IOF_QUEUED) == 0)  
		{
			Restore(ipl);
			//Still quick? Return
			if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
			//DPrintF("EndCommand ReplyMsg1\n");
			ReplyMsg(&io->io_Message);
			return;
		}
	}

	Remove(&io->io_Message.mn_Node);
	if (IsMsgPortEmpty(&io->io_Unit->unit_MsgPort)) {
		Restore(ipl);
		if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
		//DPrintF("EndCommand ReplyMsg2 %x\n", io);
		ReplyMsg(&io->io_Message);
		return;
	}
	struct IOStdReq *tmp = GetHead(&io->io_Unit->unit_MsgPort.mp_MsgList);
	if (tmp!=NULL) SET_BITS(tmp->io_Flags, IOF_CURRENT);
	Restore(ipl);
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
	//DPrintF("EndCommand ReplyMsg3 %x\n", io);
	ReplyMsg(&io->io_Message);
	return;		
}

static void MDStart(struct IOStdReq *io);
static void MDStopCmd(struct IOStdReq *io);
static void MDFlush(struct IOStdReq *io);

static void MDInvalid(struct IOStdReq *io)
{
	mouse_EndCommand(IOERR_NOCMD, io);	
}

static void MDReset(struct IOStdReq *io)
{
	MDStopCmd(io);
	MDFlush(io);
	MDStart(io);
}

static void MDRead(struct IOStdReq *io)
{
	mouse_EndCommand(IOERR_NOCMD, io);
}

static void MDWrite(struct IOStdReq *io)
{
	mouse_EndCommand(IOERR_NOCMD, io);
}

static void MDUpdate(struct IOStdReq *io)
{
	mouse_EndCommand(IOERR_NOCMD, io);
}

static void MDClear(struct IOStdReq *io)
{
	UINT32 ipl = Disable();
	devBase->BufHead = devBase->BufTail = 0;
	Restore(ipl);
	mouse_EndCommand(0, io);
}

static void MDStopCmd(struct IOStdReq *io)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	mouse_EndCommand(0, io);
}

static void MDStart(struct IOStdReq *io)
{
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)io->io_Unit->unit_MsgPort.mp_MsgList.lh_Head;
	if (new != NULL)
	{
		mouse_CmdVector[new->io_Command](new);
	}
	mouse_EndCommand(0, io);
}
 
static void MDFlush(struct IOStdReq *io)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&io->io_Unit->unit_MsgPort.mp_MsgList, node, nextnode)
	{
		mouse_EndCommand(IOERR_ABORTED, (struct IOStdReq *)node);
	}
	mouse_EndCommand(0, io);
}

#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }

static void MDReadEvent(struct IOStdReq *io)
{
	struct  IOStdReq *ioloop = io;

	while(1)
	{
		//DPrintF("[io %x]", ioloop->io_Message.mn_Node.ln_Name);
		struct Unit *unit = ioloop->io_Unit;
		UINT32	ipl = Disable();
		if (TEST_BITS(ioloop->io_Flags, IOF_SERVICING)) {
			KPrintF("IOF_Servicing\n");
			Restore(ipl);
			break;
		}
		
		ioloop->io_Flags |= IOF_SERVICING;

		if (ioloop->io_Length < sizeof(struct InputEvent)) {
			KPrintF("Error Length\n");
			Restore(ipl);
			mouse_EndCommand(IOERR_BADLENGTH, ioloop);
			if (IsMsgPortEmpty(&unit->unit_MsgPort)) break; // leave loop
			ioloop = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
			continue;
		}
		
		if (devBase->BufHead == devBase->BufTail) {
			//DPrintF("Buffer Empty ioloop %x (ID %d)\n", ioloop->io_Message.mn_Node.ln_Name, SysBase->IDNestCnt);
			ioloop->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
			Restore(ipl);
			break;
		}
		
		struct InputEvent *ie = (struct InputEvent *)ioloop->io_Data;
//		DPrintF("io_Data: %x ---", ioloop->io_Data);
		uint32_t ieSize = ioloop->io_Length;
		ieSize /= sizeof(struct InputEvent);
		uint32_t i = 0;
		do {
			ie[i].ie_Code		= devBase->BufQueue[devBase->BufHead];
			ie[i].ie_Qualifier	= devBase->BufQueue[devBase->BufHead+1];
			ie[i].ie_X			= devBase->BufQueue[devBase->BufHead+2];
			ie[i].ie_Y			= devBase->BufQueue[devBase->BufHead+3];
//			ie[i].ie_TimeStamp.tv_micro = devBase->BufQueue[devBase->BufHead+4];

			devBase->BufHead += 4;
			devBase->BufHead &= MDBUFSIZE-1;
			Restore(ipl);
			//DPrintF("bh %x, bt %x\n",devBase->BufHead, devBase->BufTail);
			ie[i].ie_Class		= IECLASS_RAWMOUSE;
			ie[i].ie_SubClass	= 0;
			ie[i].ie_NextEvent	= &ie[i+1];
			ie[i].ie_TimeStamp.tv_secs  = 0;
			i++;
			if (i+1 > ieSize) 
			{
				ipl = Disable();
				break;
			}
			ipl = Disable();
		}  while (devBase->BufHead != devBase->BufTail);
		i--;
		Restore(ipl);
		ie[i].ie_NextEvent = NULL;
		ioloop->io_Actual = i * sizeof(struct InputEvent);
		mouse_EndCommand(0, ioloop);
		
		if (IsMsgPortEmpty(&unit->unit_MsgPort)) {
			//DPrintF("No Waiters [%x][%x]\n", devBase->BufTail, devBase->BufHead);
			break; // leave loop
		}
		//struct Task *task = FindTask(NULL);
		//DPrintF("Waiter found. Task currently running[%s]\n", task->Node.ln_Name);
		ioloop = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
		continue;
	}
}

void (*mouse_CmdVector[])(struct IOStdReq *) = 
{
	MDInvalid, MDReset, MDRead, MDWrite, MDUpdate, 
	MDClear, MDStopCmd, MDStart, MDFlush, 
	MDReadEvent, //MDAskCType, MDSetCType, MDAskTrigger, MDSetTrigger
	MDInvalid, MDInvalid, MDInvalid, MDInvalid
};

INT8 mouseCmdQuick[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	 0, -1, -1, -1, -1
};
