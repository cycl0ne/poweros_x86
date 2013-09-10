#include "virtiodev.h"

#define SysBase ((struct VDBase *)io->io_Device)->vd_SysBase

static void vdBadIO(struct IOStdReq *io)
{
	vd_EndCommand(io, IOERR_NOCMD);
}

static void vdNoIO(struct IOStdReq *io)
{
	io->io_Actual = 0;
	vd_EndCommand(io, IOERR_OK);
}

static void vdRemChangeInt(struct IOStdReq *io)
{
	Forbid();	
	Remove((struct Node *)io);
	Permit();
	vd_EndCommand(io, IOERR_OK);
}

static void vdChangeState(struct IOStdReq *io)
{
	io->io_Actual = ((struct VirtioBlkUnit *)io->io_Unit)->vbu_ChangeState;
	vd_EndCommand(io, IOERR_OK);
}

static void vdSendToTask(struct IOStdReq *io)
{
	io->io_Flags	&= ~IOB_QUICK;
	io->io_Actual	= 0;
	io->io_Error	= 0;
	PutMsg(((struct VirtioBlkUnit *)io->io_Unit)->vbu_MsgPort, (struct Message *) io);
}

void (*virtioBlkCmdVector[])(struct IOStdReq *) = 
{
	vdBadIO,
	vdNoIO,
	vdSendToTask,
	vdSendToTask,
	vdSendToTask,
	vdNoIO,
	vdSendToTask,
	vdSendToTask,
	vdNoIO,
	vdNoIO,
	vdSendToTask,
	vdSendToTask,
	vdSendToTask,
	vdNoIO,
	vdChangeState,
	vdSendToTask,
	vdBadIO,
	vdBadIO,
	vdBadIO,
	vdBadIO,
	vdSendToTask,
	vdRemChangeInt,
	vdSendToTask,
	vdSendToTask,
	vdBadIO,
	vdSendToTask,
};	

#if 0
// Not Used anymore
// -1 quick, 0 noquick handled by task
INT8 virtioBlkCmdQuick[] =
{
	-1, //CMD_INVALID
	-1, //CMD_RESET
	 0, //CMD_READ
	 0, //CMD_WRITE
	 0, //CMD_UPDATE
	-1, //CMD_CLEAR
	 0, //CMD_STOP
	 0, //CMD_START
	-1, //CMD_FLUSH
	-1, //HD_MOTOR
	 0, //HD_SEEK
	 0, //HD_FORMAT
	 0, //HD_REMOVE
	-1, //HD_CHANGENUM
	-1, //HD_CHANGESTATE
	 0, //HD_PROTSTATUS
	-1, //HD_RAWREAD
	-1, //HD_RAWWRITE
	-1, //HD_GETDRIVETYPE
	-1, //HD_GETNUMTRACKS
	 0, //HD_ADDCHANGEINT
	-1, //HD_REMCHANGEINT
	 0, //HD_GETGEOMETRY
	 0, //HD_EJECT
	-1, //HD_LASTCOMM
	 0  //HD_SCSICMD	 	 
};
#endif
