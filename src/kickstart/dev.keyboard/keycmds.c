#include "types.h"
#include "sysbase.h"
#include "kbddev_intern.h"
#include "exec_funcs.h"
#include "io.h"
#include "inputevent.h"

extern void (*commandVector[])(struct IOStdReq *, KbdBase *);
void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

#define SysBase KbdBase->SysBase

static void KDClear(struct IOStdReq *io, KbdBase *KbdBase)
{
	UINT32 ipl = Disable();
	KbdBase->BufHead = KbdBase->BufTail = 0;
	Enable(ipl);
	EndCommand(0, io, SysBase);
}

//CMD_INVALID -1
static void KDInvalid(struct IOStdReq *io, KbdBase *KbdBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);	
}

//CMD_STOP -1
static void KDStop(struct IOStdReq *io, KbdBase *KbdBase)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	EndCommand(0, io, SysBase);
}

//CMD_START -1
static void KDStart(struct IOStdReq *io, KbdBase *KbdBase)
{
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)io->io_Unit->unit_MsgPort.mp_MsgList.lh_Head;
	if (new != NULL)
	{
		commandVector[new->io_Command](new, KbdBase);		
	}
	EndCommand(0, io, SysBase);
}

//CMD_FLUSH-1
static void KDFlush(struct IOStdReq *io, KbdBase *KbdBase)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&io->io_Unit->unit_MsgPort.mp_MsgList, node, nextnode)
	{
		EndCommand(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}
	EndCommand(0, io, SysBase);
}

//CMD_RESET -1
static void KDReset(struct IOStdReq *io, KbdBase *KbdBase)
{
	KDStop(io, KbdBase);
	KDFlush(io, KbdBase);
	KDStart(io, KbdBase);
}

//CMD_READEVENT 0
static void KDReadEvent(struct IOStdReq *io, KbdBase *KbdBase)
{
	struct  IOStdReq *ioloop = io;

	while(1)
	{
		//DPrintF("[KDRE: io %x]", ioloop);//ioloop->io_Message.mn_Node.ln_Name);
		struct Unit *unit = ioloop->io_Unit;
		UINT32	ipl = Disable();
		if (TEST_BITS(ioloop->io_Flags, IOF_SERVICING)) {
			DPrintF("KB_IOF_Servicing: %x\n", ioloop); //ioloop->io_Message.mn_Node.ln_Pad);
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

		if (KbdBase->BufHead == KbdBase->BufTail) {
			//DPrintF("[Kbd]Buffer Empty ioloop %x\n", ioloop);
			ioloop->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
			Enable(ipl);
			break;
		}

		struct InputEvent *ie = (struct InputEvent *)ioloop->io_Data;
		UINT32 ieSize = ioloop->io_Length / sizeof(struct InputEvent);
		UINT8 i = 0;


		do {
			ie[i].ie_Code = KbdBase->BufQueue[KbdBase->BufHead];
			ie[i].ie_Qualifier = KbdBase->BufQueue[KbdBase->BufHead+1];

			KbdBase->BufHead += 2;			
			KbdBase->BufHead &= KBBUFSIZE-1;
			Enable(ipl);

			ie[i].ie_Class = IECLASS_RAWKEY;
			ie[i].ie_SubClass = 0;
			ie[i].ie_X = 0;
			ie[i].ie_Y = 0;
			ie[i].ie_TimeStamp.tv_micro = 0;
			ie[i].ie_TimeStamp.tv_secs = 0;
			ie[i].ie_NextEvent = &ie[i+1];

			i++;
			if (i > ieSize) 
			{
				ipl = Disable();
				break;
			}
			ipl = Disable();
		}  while (KbdBase->BufHead != KbdBase->BufTail);
		Enable(ipl);
		i--;
		ie[i].ie_NextEvent = NULL;
		ioloop->io_Actual = i * sizeof(struct InputEvent);
		EndCommand(0, ioloop, SysBase);
		
		if (IsMsgPortEmpty(&unit->unit_MsgPort)) {
			//DPrintF("Empty MsgPort leaving\n");
			//DPrintF("[kd]No Waiters [%x][%x]\n", KbdBase->BufTail, KbdBase->BufHead);
			break; // leave loop
		}
		//DPrintF("DEBUG| Head %x\n", unit->unit_MsgPort.mp_MsgList.lh_Head);
		//struct Task *task = FindTask(NULL);
		//DPrintF("[kd]Waiter found. Task currently running[%s]\n", task->Node.ln_Name);
		ioloop = (struct IOStdReq *)GetHead(&unit->unit_MsgPort.mp_MsgList);
		//DPrintF("ioloop: %x\n", ioloop);
		//continue;
	}
}

//CMD_READMATRIX -1
static void KDReadMatrix(struct IOStdReq *io, KbdBase *KbdBase)
{
	io->io_Actual = MIN(MATRIX_BYTES, io->io_Length);
	CopyMem(KbdBase->Matrix, io->io_Data, io->io_Actual);
	EndCommand(0, io, SysBase);
}

//CMD_RESETHANDLER -1
static void KDAddResetHandler(struct IOStdReq *io, KbdBase *KbdBase)
{
	Enqueue(&KbdBase->HandlerList, (struct Node *) io->io_Data);
	EndCommand(0, io, SysBase);
}

//CMD_REMRESETHANDLER -1
static void KDRemResetHandler(struct IOStdReq *io, KbdBase *KbdBase)
{
	Remove((struct Node *)io->io_Data);
	EndCommand(0, io, SysBase);
}

//CMD_REMRESETHANDLERDONE -1
static void KDResetHandlerDone(struct IOStdReq *io, KbdBase *KbdBase)
{
	//KbdBase *KbdBase = io->io_Device;
	KbdBase->OutstandingResetHandlers--;
	if (KbdBase->OutstandingResetHandlers == 0) 
	{
		// RESET
	}
	EndCommand(0, io, SysBase);
}

//APTR commandVector[](void(*))
void (*commandVector[])(struct IOStdReq *, KbdBase *) = {
	KDInvalid, KDReset, KDInvalid /*READ*/, KDInvalid /*WRITE*/, KDInvalid /* Update */,
	KDClear, KDStop, KDStart, KDFlush, KDAddResetHandler, KDRemResetHandler, KDResetHandlerDone, KDReadMatrix, KDReadEvent
};

// All Commands are Quick, except ReadEvent
INT8 commandQuick[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0
};
