#include "virtiodev.h"

#define SysBase ((struct VDBase *)io->io_Device)->vd_SysBase

void vd_EndCommand(struct IOStdReq *io, UINT32 error)
{
	io->io_Error = error;
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return;
	ReplyMsg(&io->io_Message);
	return;	
}
