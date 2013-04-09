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
//	DPrintF("IDAddHandler\n");
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
	struct TimeRequest *io	= &IDBase->id_TIOR;
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
//		DPrintF("ActiveMask ff\n");
		io = &IDBase->id_K2IOR;
		ie = &IDBase->id_K2Data;
		IDBase->id_KRActiveMask = 0x00;
		io->io_Message.mn_Node.ln_Pad = 0x00;
	} else
	{
//		DPrintF("ActiveMask 00 %x\n", &IDBase->id_K1IOR);
		io = &IDBase->id_K1IOR;
		ie = &IDBase->id_K1Data;
		IDBase->id_KRActiveMask = 0xff;
		io->io_Message.mn_Node.ln_Pad = 0xff;
	}
	io->io_Command	= KBD_READEVENT;
	io->io_Flags	= 0;
	io->io_Length	= sizeof(struct InputEvent);
	io->io_Data		= ie;
//	DPrintF("SendIO KB: %x\n",io);
	SendIO((struct IORequest *)io);
}

static void gotKeyboard(IDBase *IDBase)
{
	if (IDBase->id_KRActiveMask)
	{
		DispatchEvents(IDBase, &IDBase->id_K2Data);
	} else
	{
		DispatchEvents(IDBase, &IDBase->id_K1Data);
	}
	readKeyboard(IDBase);
}


static void ReadMouse(IDBase *IDBase)
{
	if (NULL == IDBase->id_MIOR.io_Device) return;
	IDBase->id_MIOR.io_Command 	= MD_READEVENT;
	IDBase->id_MIOR.io_Length	= sizeof(struct InputEvent) * MOUSEAHEAD;
	IDBase->id_MIOR.io_Data		= &IDBase->id_MData;
	IDBase->id_MIOR.io_Flags	= 0;
	SendIO((struct IORequest *)&IDBase->id_MIOR);
}

#define CheckForMsg(a) (((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ != NULL)
#define GetMessage(a) (struct IOStdRequest *)(((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ)

static void TypeMouse(IDBase *IDBase)
{
	if (NULL == IDBase->id_MIOR.io_Device) return;
	IDBase->id_MIOR.io_Command 	= MD_SETCTYPE;
	IDBase->id_MIOR.io_Length	= 1;
	IDBase->id_MIOR.io_Data		= &IDBase->id_MType;
	IDBase->id_MIOR.io_Flags	&= IOF_QUICK;
	SendIO((struct IORequest *)&IDBase->id_MIOR);
	WaitIO((struct IORequest *)&IDBase->id_MIOR);
}

static void TrigMouse(IDBase *IDBase)
{
	if (NULL == IDBase->id_MIOR.io_Device) return;
	IDBase->id_MIOR.io_Command 	= MD_SETTRIGGER;
	IDBase->id_MIOR.io_Length	= 8;
	IDBase->id_MIOR.io_Data		= &IDBase->id_MTrig;
	IDBase->id_MIOR.io_Flags	&= IOF_QUICK;
	SendIO((struct IORequest *)&IDBase->id_MIOR);
	WaitIO((struct IORequest *)&IDBase->id_MIOR);
}

static void openGameport(IDBase *IDBase)
{
//	DPrintF("Open Gameport\n");
	if (OpenDevice("mouseport.device", IDBase->id_MPort, (struct IORequest*)&IDBase->id_MIOR, 0) != 0)
	{
		IDBase->id_MIOR.io_Device = NULL;
		DPrintF("Mouse failed open\n");
	}
	TypeMouse(IDBase);
	TrigMouse(IDBase);
	ReadMouse(IDBase);
	return;	
}

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

static void gotMouse(IDBase *IDBase)
{
	INT32 size 	= IDBase->id_MIOR.io_Actual;
	INT32 i		= 0;
	while (1)
	{
		//DPrintF("-GotM(%d / %d)",i, size);
		DispatchEvents(IDBase, &IDBase->id_MData[i++]);
		size -= sizeof(struct InputEvent);
		if (size <= 0) break;
	}
	ReadMouse(IDBase);
#if 0
	INT32 d5 = IDBase->id_MIOR.io_Actual;
	INT32 d0 = IDBase->id_MData[0].ie_Qualifier;
	INT32 idQual = IDBase->id_Qualifier;
	INT32 old = idQual;
	d0 &= ID_MOUSEMASK;
	idQual &= ((~ID_MOUSEMASK)&0xffff);
	idQual |= d0;
	IDBase->id_MIOR[0].ie_Qualifier = idQual;
	IDBase->id_Qualifier = idQual;
#endif
}

static void CheckPort(UINT32 signalStart, UINT32 signalIE, IDBase *IDBase)
{
	UINT32 sig;
	while(1)
	{
		if (CheckForMsg(IDBase->Unit.unit_MsgPort))
		{
			struct IOStdReq *tmp = (struct IOStdReq *)GetHead(&IDBase->Unit.unit_MsgPort.mp_MsgList);
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
					case 2:
						gotMouse(IDBase);
						break;
					case 3:
						gotKeyboard(IDBase);
						break;
					case 4:
						//gotRepeat(IDBase);
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
	IDBase->id_K2IOR.io_Message.mn_Length 		= 3;
	IDBase->id_RIOR.tr_node.io_Message.mn_Length= 4;

	Signal(IDBase->id_BootTask, SIGF_SINGLE);
	//Wait for someone to use us.
	UINT32 rcvd = Wait((1<<signalUnit)|(1<<signalIE));

	openGameport(IDBase);
	readTimer(IDBase);
	readKeyboard(IDBase);
	readKeyboard(IDBase);
	CheckPort((1<<signalUnit), (1<<signalIE) , IDBase);
	for(;;);
	return 0;
}
