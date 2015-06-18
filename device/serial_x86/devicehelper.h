#ifndef devicehelper_h
#define devicehelper_h
/**
 * @file devicehelper.h
 *
 * Useful things used by some devices
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "ports.h"
#include "devices.h"
#include "exec_interface.h"

static inline void QueueCommandQueue(struct IOStdReq *io, pSysBase SysBase)
{
	pSerUnit unit = (pSerUnit)io->io_Unit;
	pMsgPort port = &unit->su_OutputQueue;
	if (io->io_Command == CMD_READ) port = &unit->su_InputQueue;
	// Clear Flags needed
	io->io_Flags &= ~(IOF_CURRENT|IOF_QUEUED);

	UINT32 ipl = Disable();
	
	if (io->io_Error == 0) 
	{
		//PutMsg(port, &io->io_Message);
		//io->io_Message.mn_Node.ln_Type = NT_MESSAGE;
		//if (IsListEmpty(&port->mp_MsgList)) KPrintF("Empty port\n");
		//KPrintF("addingtail\n");
		AddTail(&port->mp_MsgList, &io->io_Message.mn_Node);
		//KPrintF("error = %d\n",err);
		//KPrintF("DEBUG| Head %x, Adress Node %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head, &io->io_Message.mn_Node);

		if (GetHead(&port->mp_MsgList) != &io->io_Message.mn_Node) 
		{
			SET_BITS(io->io_Flags, IOF_QUEUED);
		} else {
			SET_BITS(io->io_Flags, IOF_CURRENT);
		}
	}
	Restore(ipl);
	return;			
}


static inline void QueueCommand(struct IOStdReq *io, pSysBase SysBase)
{
	struct Unit	*unit = io->io_Unit;
	// Clear Flags needed
	io->io_Flags &= ~(IOF_CURRENT|IOF_QUEUED);

	UINT32 ipl = Disable();
	
	if (io->io_Error == 0) 
	{
		PutMsg(&unit->unit_MsgPort, &io->io_Message);
		//KPrintF("error = %d\n",err);
		//KPrintF("DEBUG| Head %x, Adress Node %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head, &io->io_Message.mn_Node);

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

static inline void EndCommandQueue(UINT32 error, struct IOStdReq *io, pSysBase SysBase)
{
//	KPrintF("EndCommandQueue error [%d]\n", error);
	pSerUnit unit = (pSerUnit)io->io_Unit;
	pMsgPort	port= &unit->su_OutputQueue;
	if (io->io_Command == CMD_READ) port = &unit->su_InputQueue;
	UINT32 ipl = Disable();

	if (TEST_BITS(io->io_Flags, IOF_DONE)) {
//		KPrintF("iof_done\n");
		Restore(ipl);
		return;		
	}
	SET_BITS(io->io_Flags, IOF_DONE);

	io->io_Error = error;

	if ((io->io_Flags & IOF_CURRENT) == 0) {
		if ((io->io_Flags & IOF_QUEUED) == 0)  {
			Restore(ipl);
			//Still quick? Return
//			KPrintF("we are quick and return1\n");
			if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//			KPrintF("EndCommand ReplyMsg1\n");
			ReplyMsg(&io->io_Message);
			return;
		}
	}

	Remove(&io->io_Message.mn_Node);
	if (IsMsgPortEmpty(port)) {
		Restore(ipl);
//		KPrintF("we are quick and return2\n");
		if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//		KPrintF("EndCommand ReplyMsg2 %x Port: %x\n", io, io->io_Message.mn_ReplyPort);
		ReplyMsg(&io->io_Message);
		return;
	}
	struct IOStdReq *tmp = GetHead(&port->mp_MsgList);
	if (tmp!=NULL) SET_BITS(tmp->io_Flags, IOF_CURRENT);
	Restore(ipl);
//	KPrintF("we are quick and return3\n");
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//	KPrintF("EndCommand ReplyMsg3 %x\n", io);
	ReplyMsg(&io->io_Message);
	return;		
}


static inline void EndCommand(UINT32 error, struct IOStdReq *io, pSysBase SysBase)
{
//	KPrintF("EndCommand\n");
	UINT32 ipl = Disable();
	if (TEST_BITS(io->io_Flags, IOF_DONE)) {
//		KPrintF("iof_done\n");
		Restore(ipl);
		return;		
	}
	SET_BITS(io->io_Flags, IOF_DONE);

	io->io_Error = error;

	if ((io->io_Flags & IOF_CURRENT) == 0) {
		if ((io->io_Flags & IOF_QUEUED) == 0)  {
			Restore(ipl);
			//Still quick? Return
			if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//			KPrintF("EndCommand ReplyMsg1\n");
			ReplyMsg(&io->io_Message);
			return;
		}
	}

	Remove(&io->io_Message.mn_Node);
	if (IsMsgPortEmpty(&io->io_Unit->unit_MsgPort)) {
		Restore(ipl);
		if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//		KPrintF("EndCommand ReplyMsg2 %x\n", io);
		ReplyMsg(&io->io_Message);
		return;
	}
	struct IOStdReq *tmp = GetHead(&io->io_Unit->unit_MsgPort.mp_MsgList);
	if (tmp!=NULL) SET_BITS(tmp->io_Flags, IOF_CURRENT);
	Restore(ipl);
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
//	KPrintF("EndCommand ReplyMsg3 %x\n", io);
	ReplyMsg(&io->io_Message);
	return;		
}

#endif
