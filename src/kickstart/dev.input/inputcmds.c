#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "inputevent.h"

#include "inputdev.h"

#include "sysbase.h"
#include "exec_funcs.h"

#if 0
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
#endif 
#define SysBase IDBase->SysBase

static void IDStart(struct IOStdReq *io, IDBase *IDBase);
static void IDStopCmd(struct IOStdReq *io, IDBase *IDBase);
static void IDFlush(struct IOStdReq *io, IDBase *IDBase);

static void IDInvalid(struct IOStdReq *io, IDBase *IDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);	
}

static void IDReset(struct IOStdReq *io, IDBase *IDBase)
{
	IDStopCmd(io, IDBase);
	IDFlush(io, IDBase);
	IDStart(io, IDBase);
}

static void IDRead(struct IOStdReq *io, IDBase *IDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void IDWrite(struct IOStdReq *io, IDBase *IDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void IDUpdate(struct IOStdReq *io, IDBase *IDBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void IDClear(struct IOStdReq *io, IDBase *IDBase)
{
	EndCommand(0, io, SysBase);
}

static void IDStopCmd(struct IOStdReq *io, IDBase *IDBase)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	EndCommand(0, io, SysBase);
}

static void IDStart(struct IOStdReq *io, IDBase *IDBase)
{
	if (io->io_Unit->unit_Flags & DUB_STOPPED == 0)
	{
		io->io_Unit->unit_Flags &= ~DUB_STOPPED;
		Signal(IDBase->id_Task, (1<<io->io_Unit->unit_MsgPort.mp_SigBit));
	}
	EndCommand(0, io, SysBase);
}
 
static void IDFlush(struct IOStdReq *io, IDBase *IDBase)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&io->io_Unit->unit_MsgPort.mp_MsgList, node, nextnode)
	{
		EndCommand(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}
	EndCommand(0, io, SysBase);
}

static void IDSetThresh(struct IOStdReq *io, IDBase *IDBase)
{
	struct TimeRequest *time = (struct TimeRequest*) io->io_Data;
	IDBase->id_Thresh.tv_secs	= time->tr_time.tv_secs;
	IDBase->id_Thresh.tv_micro	= time->tr_time.tv_micro;
	EndCommand(0, io, SysBase);	
}

static void IDSetPeriod(struct IOStdReq *io, IDBase *IDBase)
{
	struct TimeRequest *time = (struct TimeRequest *) io->io_Data;
	IDBase->id_Period.tv_secs	= time->tr_time.tv_secs;
	IDBase->id_Period.tv_micro	= time->tr_time.tv_micro;
	EndCommand(0, io, SysBase);	
}

static void IDTaskCommand(struct IOStdReq *io, IDBase *IDBase)
{
	io->io_Flags &= ~IOF_QUICK;
}

void (*inputCmdVector[])(struct IOStdReq *, IDBase *) = 
{
	IDInvalid, IDReset, IDRead, IDWrite, IDUpdate, 
	IDClear, IDStopCmd, IDStart, IDFlush, 
	IDTaskCommand, IDTaskCommand, IDTaskCommand,
	IDSetThresh, IDSetPeriod,
	IDTaskCommand, IDTaskCommand, IDTaskCommand
};

INT8 inputCmdQuick[] =
{
	-1, -1, -1, -1, -1, 
	-1, -1, -1, -1, 
	 0,  0,  0,
	-1, -1,
	 0,  0,  0
};
