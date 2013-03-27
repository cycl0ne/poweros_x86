#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "inputevent.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"
	
void QueueCommand(struct IOStdReq *io, SysBase *SysBase)
{
	UINT32 ipl = Disable();
	
	struct Unit	*unit = io->io_Unit;
	// Clear Flags needed
	io->io_Flags &= ~(IOF_CURRENT|IOF_QUEUED);
	
	if (io->io_Error == 0) {
		PutMsg(&unit->unit_MsgPort, &io->io_Message);
		//DPrintF("DEBUG| Head %x, Adress Node %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head, &io->io_Message.mn_Node);

		if (GetHead(&unit->unit_MsgPort.mp_MsgList) != &io->io_Message.mn_Node) {
			SET_BITS(io->io_Flags, IOF_QUEUED);
		} else {
			SET_BITS(io->io_Flags, IOF_CURRENT);
		}
	}
	Enable(ipl);
	return;			
}

void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase)
{
	UINT32 ipl = Disable();
	if (TEST_BITS(io->io_Flags, IOF_DONE)) {
		Enable(ipl);
		return;		
	}
	SET_BITS(io->io_Flags, IOF_DONE);

	io->io_Error = error;

	if ((io->io_Flags & IOF_CURRENT) == 0) {
		if ((io->io_Flags & IOF_QUEUED) == 0)  {
			Enable(ipl);
			//Still quick? Return
			if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
			ReplyMsg(&io->io_Message);
			return;
		}
	}
	
	Remove(&io->io_Message.mn_Node);
	if (IsMsgPortEmpty(&io->io_Unit->unit_MsgPort)) {
		Enable(ipl);
		if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
		ReplyMsg(&io->io_Message);
		return;
	}
	struct IOStdReq *tmp = GetHead(&io->io_Unit->unit_MsgPort.mp_MsgList);
	if (tmp!=NULL) SET_BITS(tmp->io_Flags, IOF_CURRENT);
	Enable(ipl);
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
	ReplyMsg(&io->io_Message);
	return;		
}

#define SysBase MDBase->SysBase

static void MDReadEvent(struct IOStdReq *io, MDBase *MDBase)
{
	struct  IOStdReq *ioloop = io;

	while(1)
	{
		//DPrintF("[io %x]", ioloop->io_Message.mn_Node.ln_Name);
		struct Unit *unit = ioloop->io_Unit;
		UINT32	ipl = Disable();
		if (TEST_BITS(ioloop->io_Flags, IOF_SERVICING)) {
			DPrintF("IOF_Servicing\n");
			Enable(ipl);
			break;
		}
		
		ioloop->io_Flags |= IOF_SERVICING;

		if (ioloop->io_Length < sizeof(struct InputEvent)) {
			DPrintF("Error Length\n");
			Enable(ipl);
			EndCommand(IOERR_BADLENGTH, ioloop, SysBase);
			if (IsMsgPortEmpty(&unit->unit_MsgPort)) break; // leave loop
			ioloop = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
			continue;
		}
		
		if (MDBase->BufHead == MDBase->BufTail) {
			//DPrintF("Buffer Empty ioloop %x (ID %d)\n", ioloop->io_Message.mn_Node.ln_Name, SysBase->IDNestCnt);
			ioloop->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
			Enable(ipl);
			break;
		}
		
		struct InputEvent *ie = (struct InputEvent *)ioloop->io_Data;
		UINT32 ieSize = ioloop->io_Length / sizeof(struct InputEvent);
		UINT8 i = 0;

		do {
			ie[i].ie_Code		= MDBase->BufQueue[MDBase->BufHead];
			ie[i].ie_Qualifier	= MDBase->BufQueue[MDBase->BufHead+1];
			ie[i].ie_X			= MDBase->BufQueue[MDBase->BufHead+2];
			ie[i].ie_Y			= MDBase->BufQueue[MDBase->BufHead+3];
			ie[i].ie_TimeStamp.tv_micro = MDBase->BufQueue[MDBase->BufHead+4];

			MDBase->BufHead += 4;
			MDBase->BufHead &= MDBUFSIZE-1;
			Enable(ipl);
			//DPrintF("bh %x, bt %x\n",MDBase->BufHead, MDBase->BufTail);
			ie[i].ie_Class		= IECLASS_RAWMOUSE;
			ie[i].ie_SubClass	= 0;
			ie[i].ie_NextEvent	= &ie[i+1];
			ie[i].ie_TimeStamp.tv_secs  = 0;
			if (i+1 > ieSize) 
			{
				ipl = Disable();
				break;
			}
			i++;
			ipl = Disable();
		}  while (MDBase->BufHead != MDBase->BufTail);
		
		Enable(ipl);
		ie[i].ie_NextEvent = NULL;
		ioloop->io_Actual = ieSize * sizeof(struct InputEvent);
		EndCommand(0, ioloop, SysBase);
		
		if (IsMsgPortEmpty(&unit->unit_MsgPort)) {
			//DPrintF("No Waiters [%x][%x]\n", MDBase->BufTail, MDBase->BufHead);
			break; // leave loop
		}
		//struct Task *task = FindTask(NULL);
		//DPrintF("Waiter found. Task currently running[%s]\n", task->Node.ln_Name);
		ioloop = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
		continue;
	}
}

static void MDStart(struct IOStdReq *io, MDBase *MDBase);
static void MDStopCmd(struct IOStdReq *io, MDBase *MDBase);
static void MDFlush(struct IOStdReq *io, MDBase *MDBase);

static void MDInvalid(struct IOStdReq *io, MDBase *MDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);	
}

static void MDReset(struct IOStdReq *io, MDBase *MDBase)
{
	MDStopCmd(io, MDBase);
	MDFlush(io, MDBase);
	MDStart(io, MDBase);
}

static void MDRead(struct IOStdReq *io, MDBase *MDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void MDWrite(struct IOStdReq *io, MDBase *MDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void MDUpdate(struct IOStdReq *io, MDBase *MDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void MDClear(struct IOStdReq *io, MDBase *MDBase)
{
	UINT32 ipl = Disable();
	MDBase->BufHead = MDBase->BufTail = 0;
	Enable(ipl);
	EndCommand(0, io, SysBase);
}

static void MDStopCmd(struct IOStdReq *io, MDBase *MDBase)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	EndCommand(0, io, SysBase);
}

static void MDStart(struct IOStdReq *io, MDBase *MDBase)
{
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)io->io_Unit->unit_MsgPort.mp_MsgList.lh_Head;
	if (new != NULL)
	{
		mouseCmdVector[new->io_Command](new, MDBase);
	}
	EndCommand(0, io, SysBase);
}
 
static void MDFlush(struct IOStdReq *io, MDBase *MDBase)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&io->io_Unit->unit_MsgPort.mp_MsgList, node, nextnode)
	{
		EndCommand(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}
	EndCommand(0, io, SysBase);
}

static void MDAskCType(struct IOStdReq *io, MDBase *MDBase)
{
	if (io->io_Length < 1) 
	{
		EndCommand(IOERR_BADLENGTH, io, SysBase);
		return;
	}
	UINT8 *tmp = io->io_Data;
	tmp[0]			= MDBase->Type;
	io->io_Actual	= 1;
	EndCommand(0, io, SysBase);	
}

static void MDSetCType(struct IOStdReq *io, MDBase *MDBase)
{
	// does nothing at the moment :)
	EndCommand(0, io, SysBase);	
}

static void MDAskTrigger(struct IOStdReq *io, MDBase *MDBase)
{
	if (io->io_Length < sizeof(struct MouseTrigger))
	{
		EndCommand(IOERR_BADLENGTH, io, SysBase);
		return;
	}
	MouseTrigger *tmp = (MouseTrigger *)io->io_Data;

	tmp->Keys		= MDBase->Timeout.Keys;
	tmp->Timeout	= MDBase->Timeout.Timeout;
	tmp->XDelta		= MDBase->Timeout.XDelta;
	tmp->YDelta		= MDBase->Timeout.YDelta;

	io->io_Actual = sizeof(MouseTrigger);
	EndCommand(0, io, SysBase);	
}

static void MDSetTrigger(struct IOStdReq *io, MDBase *MDBase)
{
	if (io->io_Length < sizeof(struct MouseTrigger))
	{
		EndCommand(IOERR_BADLENGTH, io, SysBase);
		return;
	}
	MouseTrigger *tmp = (MouseTrigger *)io->io_Data;

	MDBase->Timeout.Keys	= tmp->Keys;
	MDBase->Timeout.Timeout	= tmp->Timeout;
	MDBase->Timeout.XDelta	= tmp->XDelta;
	MDBase->Timeout.YDelta	= tmp->YDelta;
	io->io_Actual = sizeof(MouseTrigger);
	EndCommand(0, io, SysBase);	
}

void (*mouseCmdVector[])(struct IOStdReq *, MDBase *) = 
{
	MDInvalid, MDReset, MDRead, MDWrite, MDUpdate, 
	MDClear, MDStopCmd, MDStart, MDFlush, 
	MDReadEvent, MDAskCType, MDSetCType, MDAskTrigger, MDSetTrigger
};

INT8 mouseCmdQuick[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	 0, -1, -1, -1, -1
};
