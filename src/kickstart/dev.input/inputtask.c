#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"
#include "mouseport.h"
#include "keyboard.h"
#include "inputdev.h"
#include "sysbase.h"
#include "exec_funcs.h"
#include "timer_funcs.h"

#define SysBase IDBase->SysBase

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

#   define GetHead(l)       (void *)(((struct List *)l)->lh_Head->ln_Succ \
				? ((struct List *)l)->lh_Head \
				: (struct Node *)0)
#   define GetTail(l)       (void *)(((struct List *)l)->lh_TailPred->ln_Pred \
				? ((struct List *)l)->lh_TailPred \
				: (struct Node *)0)
#   define GetSucc(n)       (void *)(((struct Node *)n)->ln_Succ->ln_Succ \
				? ((struct Node *)n)->ln_Succ \
				: (struct Node *)0)
#   define GetPred(n)       (void *)(((struct Node *)n)->ln_Pred->ln_Pred \
				? ((struct Node *)n)->ln_Pred \
				: (struct Node *)0)

void DispatchEvents(IDBase *IDBase, struct InputEvent *ie);

void IDSetMPort(struct IOStdReq *io, IDBase *IDBase)
{
	if (io->io_Length-1 == 0)
	{
		UINT8 *tmp = (UINT8*)io->io_Data;
		IDBase->id_MPort = tmp[0];
		io->io_Actual = 1;
		EndCommand(0, io, SysBase);			
	}
	EndCommand(IOERR_BADLENGTH, io, SysBase);
}

void IDSetMType(struct IOStdReq *io, IDBase *IDBase)
{
	if (io->io_Length-1 == 0)
	{
		UINT8 *tmp = (UINT8*)io->io_Data;
		IDBase->id_MType = tmp[0];
		io->io_Actual = 1;
		EndCommand(0, io, SysBase);			
	}
	EndCommand(IOERR_BADLENGTH, io, SysBase);
}

void IDSetMTrig(struct IOStdReq *io, IDBase *IDBase)
{
	if (io->io_Length != sizeof(struct MouseTrigger))
	{
		MouseTrigger *mt = (MouseTrigger *) io->io_Data;
		IDBase->id_MTrig.Keys = mt->Keys;
		IDBase->id_MTrig.Timeout = mt->Timeout;
		IDBase->id_MTrig.XDelta = mt->XDelta;
		IDBase->id_MTrig.YDelta = mt->YDelta;
		io->io_Actual = sizeof(MouseTrigger);
		EndCommand(0, io, SysBase);			
	}
	EndCommand(IOERR_BADLENGTH, io, SysBase);
}

void IDAddHandler(struct IOStdReq *io, IDBase *IDBase)
{
	Enqueue(&IDBase->id_HandlerList, (struct Node *)io->io_Data);
	EndCommand(0, io, SysBase);	
}

void IDRemHandler(struct IOStdReq *io, IDBase *IDBase)
{
	Remove((struct Node *)io->io_Data);
	EndCommand(0, io, SysBase);	
}

void IDWriteEvent(struct IOStdReq *io, IDBase *IDBase)
{
	DispatchEvents(IDBase, (struct InputEvent *)io->io_Data);
	EndCommand(0, io, SysBase);	
}

void DispatchEvents(IDBase *IDBase, struct InputEvent *ie)
{
	APTR TimerBase = IDBase->id_TIOR.tr_node.io_Device;
	GetSysTime(&ie->ie_TimeStamp);
	ie->ie_NextEvent = NULL;

	struct Interrupt *ihandler;
	struct InputEvent *newie = ie;
	ForeachNode(&IDBase->id_HandlerList, ihandler)
	{
		newie = ihandler->is_Code(newie, ihandler->is_Data);
		if (newie == NULL) break;
	}
}

static void readTimer(IDBase *IDBase)
{
	struct TimeRequest *io	= (struct TimeRequest *)&IDBase->id_TIOR;
	io->tr_node.io_Command	= TR_ADDREQUEST;
	io->tr_node.io_Flags	= 0;
	io->tr_time.tv_secs		= 0;
	io->tr_time.tv_micro	= 100000; // 10 times/sec
	SendIO((struct IORequest *)io);
}

static void readKeyboard(IDBase *IDBase)
{
	struct IOStdReq *io = NULL;
	struct InputEvent *ie = NULL;
	if (IDBase->id_KRActiveMask)
	{
		io = &IDBase->id_K2IOR;
		ie = &IDBase->id_K2Data;
		IDBase->id_KRActiveMask = 0x00;
	} else
	{
		io = &IDBase->id_K1IOR;
		ie = &IDBase->id_K1Data;
		IDBase->id_KRActiveMask = 0xff;
	}
	
	io->io_Command	= KBD_READEVENT;
	io->io_Flags	= 0;
	io->io_Length	= sizeof(struct InputEvent);
	io->io_Data		= ie;
	SendIO((struct IORequest *)io);
}

#define CheckForMsg(a) (((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ != NULL)
#define GetMessage(a) (struct IOStdRequest *)(((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ)

static void gotTimer(IDBase *IDBase)
{
	IDBase->id_TData.ie_Class		= IECLASS_TIMER;
	IDBase->id_TData.ie_SubClass	= 0;
	IDBase->id_TData.ie_Code		= 0;
	IDBase->id_TData.ie_EventAddress = NULL;
	IDBase->id_TData.ie_Qualifier	= IDBase->id_Qualifier;
	DispatchEvents(IDBase, &IDBase->id_TData);
	readTimer(IDBase);
}

static void gotKeyboard(IDBase *IDBase)
{
	
}

static void CheckPort(UINT32 signalStart, UINT32 signalIE, IDBase *IDBase)
{
	UINT32 sig;
	while(1)
	{
		if (CheckForMsg(IDBase->Unit.unit_MsgPort))
		{
			struct IOStdReq *tmp = (struct IOStdReq *)GetHead(&IDBase->Unit.unit_MsgPort);
			tmp->io_Flags &= ~IOB_QUICK;
			switch(tmp->io_Command)
			{
				case IND_ADDHANDLER:
					IDAddHandler(tmp, IDBase);
					break;
				case IND_REMHANDLER:
					IDRemHandler(tmp, IDBase);
					break;
				case IND_WRITEEVENT:
					IDWriteEvent(tmp, IDBase);
					break;
				case IND_SETMPORT:
					IDSetMPort(tmp, IDBase);
					break;
				case IND_SETMTYPE:
					IDSetMType(tmp, IDBase);
					break;
				case IND_SETMTRIG:
					IDSetMTrig(tmp, IDBase);
					break;
				default:
					break;
			}
		}
		
		while(IDBase->Unit.unit_Flags & DUB_STOPPED != 0)
		{
			sig = Wait(signalStart);
			if (IDBase->Unit.unit_Flags & DUB_STOPPED == 0) 
			{
				if (sig & signalIE) SetSignal(signalIE, signalIE);
				break;
			}
		}
		sig = Wait(signalIE|signalStart);
		
		if (sig & (1<<(IDBase->id_IEPort.mp_SigBit)))
		{
			struct Message *msg = GetMsg(&IDBase->id_IEPort);
			while (msg != NULL)
			{
				switch(msg->mn_Length)
				{
					case 1:
						gotTimer(IDBase);
						break;
					default:
						DPrintF("ID-> wrong msg\n");
						break;
				}
				msg = GetMsg(&IDBase->id_IEPort);
			}
		}
	}
}


#undef SysBase
static INT8 id_InitMsgPort(struct MsgPort *mport, SysBase *SysBase)
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

UINT32 idev_InputTask(IDBase *IDBase, APTR SysBase)
{
	INT8 signalUnit;
	INT8 signalIE;
	signalUnit = id_InitMsgPort(&IDBase->Unit.unit_MsgPort, SysBase);

	struct MsgPort *port = &IDBase->id_IEPort;	
	IDBase->id_TIOR.tr_node.io_Message.mn_ReplyPort = port;
	IDBase->id_MIOR.io_Message.mn_ReplyPort			= port;
	IDBase->id_K1IOR.io_Message.mn_ReplyPort 		= port;
	IDBase->id_K2IOR.io_Message.mn_ReplyPort 		= port;
	IDBase->id_RIOR.tr_node.io_Message.mn_ReplyPort = port;

	signalIE = id_InitMsgPort(port, SysBase);	

	IDBase->id_TIOR.tr_node.io_Message.mn_Length= 1;
	IDBase->id_MIOR.io_Message.mn_Length  		= 2;
	IDBase->id_K1IOR.io_Message.mn_Length 		= 3;
	IDBase->id_K2IOR.io_Message.mn_Length 		= 4;
	IDBase->id_RIOR.tr_node.io_Message.mn_Length= 5;
DPrintF("Signal Init of input.device\n");
	Signal(IDBase->id_BootTask, SIGF_SINGLE);

DPrintF("wait for somebody\n");	
	UINT32 rcvd = Wait((1<<signalUnit)|(1<<signalIE));
DPrintF("Got one\n");	

	//openGameport(IDBase);	
	//lot of work still here

	readTimer(IDBase);
	readKeyboard(IDBase);
	readKeyboard(IDBase);
	CheckPort((1<<signalUnit), (1<<signalIE) , IDBase);
	for(;;);
	return 0;
}
