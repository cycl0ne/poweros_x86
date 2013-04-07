#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"

#include "inputdev.h"

#include "sysbase.h"
#include "exec_funcs.h"

APTR idev_OpenDev(struct IDBase *IDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags)
{
	// We have only one Unit. so we dont care about unit.
	ioreq->io_Error = 0;
	IDBase->Device.dd_Library.lib_OpenCnt++;
	IDBase->Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
	ioreq->io_Unit = &IDBase->Unit;
	return IDBase;		
}

APTR idev_CloseDev(struct IDBase *IDBase, struct IORequest *ioreq)
{
	IDBase->Device.dd_Library.lib_OpenCnt--;
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return IDBase;
}

APTR idev_ExpungeDev(struct IDBase *IDBase)
{
	return NULL;
}

APTR idev_ExtFuncDev(struct IDBase *IDBase)
{
	return NULL;
}

