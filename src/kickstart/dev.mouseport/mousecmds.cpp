#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "inputevent.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"

/*
*/
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *);

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )
	
static void MDStart(struct IOStdReq *io, MDBase *MDBase);
static void MDStopCmd(struct IOStdReq *io, MDBase *MDBase);
static void MDFlush(struct IOStdReq *io, MDBase *MDBase);

#define SysBase MDBase->SysBase

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

#define GetListHeadNode(l)\
		(l) ? (((((struct List*)(l))->lh_Head) == ((struct Node *)&(((struct List*)(l))->lh_Tail))) ? (NULL) : ((((struct List*)(l))->lh_Head))) : NULL

void arch_irq_mask(UINT32);
void arch_irq_unmask(UINT32);
#define DisableMouse() arch_irq_mask(1<<12);
#define EnableMouse() arch_irq_unmask(1<<12);

static void MDReadEvent(struct IOStdReq *io, MDBase *MDBase)
{
	UINT32 ipl = Disable();
	//DisableMouse();
	if ((io->io_Flags & IOF_SERVICING) != 0) 
	{
		Enable(ipl);
		//EnableMouse();
		return;	
	}

	io->io_Flags |= IOF_SERVICING;

	if (io->io_Length < sizeof(struct InputEvent))
	{
//		EnableMouse();
		Enable(ipl);
		EndCommand(IOERR_BADLENGTH, io, SysBase);
//		Disable();
//		DisableMouse();
		if (!IsMsgPortEmpty(&io->io_Unit->unit_MsgPort))
		{
			struct IOStdReq *new = (struct IOStdReq *)io->io_Unit->unit_MsgPort.mp_MsgList.lh_Head;
			if (new != NULL) MDReadEvent(new, MDBase);
		}
		//EnableMouse();
		return;
	}

	if (MDBase->BufHead == MDBase->BufTail)
	{
		io->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
		//EnableMouse();
		Enable(ipl);
		return;
	}

	struct InputEvent *ie = (struct InputEvent *)io->io_Data;
	UINT32 ieSize = io->io_Length / sizeof(struct InputEvent);
	UINT8 i = 0;

	while (MDBase->BufHead != MDBase->BufTail)
	{
#if 0
		ie[i].ie_Code		= MDBase->BufQueue[MDBase->BufHead];
		ie[i].ie_Qualifier	= MDBase->BufQueue[MDBase->BufHead+1];
		ie[i].ie_X			= MDBase->BufQueue[MDBase->BufHead+2];
		ie[i].ie_Y			= MDBase->BufQueue[MDBase->BufHead+3];
		ie[i].ie_TimeStamp.tv_micro = MDBase->BufQueue[MDBase->BufHead+4];
		ie[i].ie_TimeStamp.tv_secs  = 0;
#endif
		MDBase->BufHead += 4;
		MDBase->BufHead &= MDBUFSIZE-1;
#if 0	
		ie[i].ie_Class		= IECLASS_RAWMOUSE;
		ie[i].ie_SubClass	= 0;
		ie[i].ie_NextEvent	= &ie[i+1];
#endif		
		if (i+1 > ieSize) break;
		i++;
	}
	
	ie[i].ie_NextEvent = NULL;
	io->io_Actual = ieSize * sizeof(struct InputEvent);
//	EnableMouse();
	Enable(ipl);

	EndCommand(0, io, SysBase);
//	DisableMouse();
	//ipl = Disable();
	
	// and again recursion :-/ Should fix this !
	BOOL mp = !IsMsgPortEmpty(&MDBase->Unit.unit_MsgPort);
	DPrintF("mp [%x]",mp);
	if (mp)
	{
		struct IOStdReq *new = (struct IOStdReq *)MDBase->Unit.unit_MsgPort.mp_MsgList.lh_Head;
		if (new != NULL) 
		{
			DPrintF("Waiters\n");
			MDReadEvent(new, MDBase);
		}
	}
	//EnableMouse();
	//Enable(ipl);
	return;
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
