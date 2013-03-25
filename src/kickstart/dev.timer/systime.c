#include "exec_funcs.h"
#include "timer_intern.h"

void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src)
{
	UINT32 ipl = Disable();
	src->tv_micro = TimerBase->CurrentTime.tv_micro;
	src->tv_secs  = TimerBase->CurrentTime.tv_secs;
	Enable(ipl);
}

UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src)
{
	return 0;
//	src->ev_lo = READ32(ST_BASE+0x04);
//	src->ev_hi = READ32(ST_BASE+0x08);	
//	return STC_FREQ_HZ;
}

