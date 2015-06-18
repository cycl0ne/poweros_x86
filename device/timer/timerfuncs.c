#include "exec_interface.h"
#include "timer_intern.h"

#define SysBase TimerBase->Timer_SysBase

void timer_BeginIO(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
//KPrintF("BeginIO Timer\n");
	UINT8 cmd = ioreq->io_Command;
	ioreq->io_Error = 0;

	if (cmd > TR_SETSYSTIME || cmd < TR_ADDREQUEST) cmd = 0; // Invalidate the command.

	TimerCmdVector[cmd](TimerBase, ioreq);
}

void timer_AbortIO(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	if(ioreq->io_Message.mn_Node.ln_Type != NT_REPLYMSG)
    {
		Remove((struct Node *)ioreq);
		INTERN_EndCommand(TimerBase, IOERR_ABORTED, ioreq);
	}
}

INT32 timer_CmpTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dest)
{
	//KPrintF("Inside timer_CmpTime!\n");
    INT32 diff;

    if (dest->tv_secs == src->tv_secs) diff = src->tv_micro - dest->tv_micro;
    else diff = src->tv_secs - dest->tv_secs;

    if (diff < 0) return -1; // first argument smaller
    else if (diff > 0) return 1; // first argument bigger
    else return 0; // both arguments equal
}

void timer_AddTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst)
{
	//KPrintF("Inside timer_AddTime!\n");
	dst->tv_micro += src->tv_micro;
	dst->tv_secs  += src->tv_secs;
	while(dst->tv_micro > 999999)
	{
		dst->tv_secs++;
		dst->tv_micro -= 1000000;
	}
}

void timer_SubTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dest)
{
	//KPrintF("Inside timer_SubTime!\n");
    while(src->tv_micro > 999999)
    {
		src->tv_secs++;
		src->tv_micro -= 1000000;
    }
    while(dest->tv_micro > 999999)
    {
		dest->tv_secs++;
		dest->tv_micro -= 1000000;
    }
    if(dest->tv_micro < src->tv_micro)
    {
		dest->tv_micro += 1000000;
		dest->tv_secs--;
    }

    dest->tv_micro -= src->tv_micro;
    dest->tv_secs -= src->tv_secs;
}


void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src)
{
	//KPrintF("Inside timer_GetSysTime!\n");
	UINT32 ipl = Disable();
	src->tv_micro = TimerBase->CurrentTime.tv_micro;
	src->tv_secs  = TimerBase->CurrentTime.tv_secs;
	Restore(ipl);
}

UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src)
{
	//KPrintF("Inside timer_ReadEClock!\n");
	return 0;
//	src->ev_lo = READ32(ST_BASE+0x04);
//	src->ev_hi = READ32(ST_BASE+0x08);
//	return STC_FREQ_HZ;
}



