#include "exec_interface.h"
#include "timer_intern.h"

#define SysBase TimerBase->Timer_SysBase
struct TimerBase *timer_OpenDev(struct TimerBase *TimerBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags)
{
	//DPrintF("[TimerDev] Open Unit: %d\n", unitNum);
    TimerBase->Device.dev_OpenCnt++;
    TimerBase->Device.dev_Flags &= ~LIBF_DELEXP;
    if (unitNum >= UNIT_MICROHZ && unitNum <= UNIT_WAITECLOCK)
    {
		ioreq->io_Error = 0;
	    ioreq->io_Unit = (struct  Unit *)&TimerBase->TimerUnit[unitNum];
	    ioreq->io_Device = (struct Device *)TimerBase;
	}
	else
	{
		ioreq->io_Error = IOERR_OPENFAIL;
	}
	//KPrintF("[TimerDev] Open Unit: %d\n", unitNum);
	return(TimerBase);
}

APTR timer_CloseDev(struct TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	TimerBase->Device.dev_OpenCnt--;
	if(!TimerBase->Device.dev_OpenCnt)
	{
		// Should we "expunge" the device?
	}
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return (TimerBase);
}

APTR timer_ExpungeDev(struct TimerBase *TimerBase)
{
	return (NULL);
}

APTR timer_ExtFuncDev(void)
{
	return (NULL);
}



