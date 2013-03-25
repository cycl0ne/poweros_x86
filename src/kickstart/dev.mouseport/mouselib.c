#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"

APTR mdev_OpenDev(struct MDBase *MDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags)
{
	// We have only one Unit. so we dont care about unit.
	ioreq->io_Error = 0;
	MDBase->Device.dd_Library.lib_OpenCnt++;
	MDBase->Device.dd_Library.lib_Flags &= ~LIBF_DELEXP;
	ioreq->io_Unit = &MDBase->Unit;
	return MDBase;		
}

APTR mdev_CloseDev(struct MDBase *MDBase, struct IORequest *ioreq)
{
	MDBase->Device.dd_Library.lib_OpenCnt--;
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return MDBase;
}

APTR mdev_ExpungeDev(struct MDBase *MDBase)
{
	return NULL;
}

APTR mdev_ExtFuncDev(struct MDBase *MDBase)
{
	return NULL;
}

