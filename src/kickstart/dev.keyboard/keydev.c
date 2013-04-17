#include "types.h"
#include "sysbase.h"
#include "kbddev_intern.h"
#include "exec_funcs.h"
#include "io.h"
#include "inputevent.h"

void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

extern void (*commandVector[])(struct IORequest *, KbdBase *);
extern INT8 commandQuick[];

#define SysBase KbdBase->SysBase

APTR kdev_OpenDev(struct KbdBase *KbdBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags)
{
	// We have only one Unit. so we dont care about unit.
	ioreq->io_Error = 0;
	KbdBase->Device.dd_Library.lib_OpenCnt++;
	KbdBase->Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
	ioreq->io_Unit = &KbdBase->Unit;
	return KbdBase;		
}

APTR kdev_CloseDev(struct KbdBase *KbdBase, struct IORequest *ioreq)
{
	KbdBase->Device.dd_Library.lib_OpenCnt--;
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return KbdBase;
}

APTR kdev_ExpungeDev(struct KbdBase *KbdBase)
{
	return NULL;
}

APTR kdev_ExtFuncDev(struct KbdBase *KbdBase)
{
	return NULL;
}

void kdev_BeginIO(KbdBase *KbdBase, struct IORequest *io)
{
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;
	
	if (cmd > KBD_READEVENT) cmd = 0; // Invalidate the command.
	if (commandQuick[cmd] >= 0)
	{
		QueueCommand((struct IOStdReq*)io, SysBase);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		if ((KbdBase->Unit.unit_Flags & DUB_STOPPED) != 0)
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;	
		}
	}
	commandVector[cmd](io, KbdBase);
}

void kdev_AbortIO(struct IORequest *ioreq, KbdBase *KbdBase)
{
	EndCommand(IOERR_ABORTED, (struct IOStdReq*)ioreq, SysBase);
}
