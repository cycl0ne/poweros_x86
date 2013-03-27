#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "exec_funcs.h"

#include "vgagfx.h"

#define SysBase VgaGfxBase->SysBase

static void VGDStart(struct IOStdReq *io, VgaGfxBase *VgaGfxBase);
static void VGDStopCmd(struct IOStdReq *io, VgaGfxBase *VgaGfxBase);
static void VGDFlush(struct IOStdReq *io, VgaGfxBase *VgaGfxBase);

static void VGDInvalid(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);	
}

static void VGDReset(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	VGDStopCmd(io, VgaGfxBase);
	VGDFlush(io, VgaGfxBase);
	VGDStart(io, VgaGfxBase);
}

static void VGDRead(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void VGDWrite(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void VGDUpdate(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	EndCommand(IOERR_NOCMD, io, SysBase);
}

static void VGDClear(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	UINT32 ipl = Disable();
	//VgaGfxBase->BufHead = VgaGfxBase->BufTail = 0;
	Enable(ipl);
	EndCommand(0, io, SysBase);
}

static void VGDStopCmd(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	EndCommand(0, io, SysBase);
}

static void VGDStart(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)io->io_Unit->unit_MsgPort.mp_MsgList.lh_Head;
	if (new != NULL)
	{
		gfxCmdVector[new->io_Command](new, VgaGfxBase);
	}
	EndCommand(0, io, SysBase);
}
 
static void VGDFlush(struct IOStdReq *io, VgaGfxBase *VgaGfxBase)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&io->io_Unit->unit_MsgPort.mp_MsgList, node, nextnode)
	{
		EndCommand(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}
	EndCommand(0, io, SysBase);
}


void (*gfxCmdVector[])(struct IOStdReq *, VgaGfxBase *) = 
{
	VGDInvalid, VGDReset, VGDRead, VGDWrite, VGDUpdate, 
	VGDClear, VGDStopCmd, VGDStart, VGDFlush
};

INT8 gfxCmdQuick[] =
{
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 
	 0, -1, -1, -1, -1
};

