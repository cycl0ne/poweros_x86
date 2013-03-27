#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"

#include "vgagfx.h"
//(struct IOStdReq*) 
#define SysBase VgaGfxBase->SysBase

void gfxdev_BeginIO(VgaGfxBase *VgaGfxBase, struct IORequest *io)
{
	
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;
	
	if (cmd > CMD_NONSTD) cmd = 0; // Invalidate the command.

	if (gfxCmdQuick[cmd] >= 0)
	{
		QueueCommand((struct IOStdReq*) io, SysBase);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		// If Device is stopped, just return
		if (TEST_BITS(VgaGfxBase->Unit.unit_Flags, DUB_STOPPED))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;	
		}
		// we are first in Queue, now we are Quick, otherwise we come from the IS Routine
	}
	gfxCmdVector[cmd]((struct IOStdReq*) io, VgaGfxBase);
}

void gfxdev_AbortIO(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq)
{
	EndCommand(IOERR_ABORTED, (struct IOStdReq*) ioreq, SysBase);
}


APTR gfxdev_OpenDev(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags)
{
	// We have only one Unit. so we dont care about unit.
	ioreq->io_Error = 0;
	VgaGfxBase->Device.dd_Library.lib_OpenCnt++;
	VgaGfxBase->Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
	ioreq->io_Unit = &VgaGfxBase->Unit;
	return VgaGfxBase;		
}

APTR gfxdev_CloseDev(VgaGfxBase *VgaGfxBase, struct IORequest *ioreq)
{
	VgaGfxBase->Device.dd_Library.lib_OpenCnt--;
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return VgaGfxBase;
}

APTR gfxdev_ExpungeDev(VgaGfxBase *VgaGfxBase)
{
	return NULL;
}

APTR gfxdev_ExtFuncDev(VgaGfxBase *VgaGfxBase)
{
	return NULL;
}

