#ifndef timer_h
#define timer_h

#include "types.h"
#include "devices.h"

/* Units */
#define UNIT_MICROHZ    0
#define UNIT_VBLANK     1
#define UNIT_ECLOCK     2
#define UNIT_WAITUNTIL  3
#define UNIT_WAITECLOCK 4
#define UNIT_MAX		5

/* IO-Commands */
#define TR_ADDREQUEST (CMD_NONSTD+0)
#define TR_GETSYSTIME (CMD_NONSTD+1)
#define TR_SETSYSTIME (CMD_NONSTD+2)

#define VERSION  0
#define REVISION 2

#define DEVNAME "timer"
#define DEVVER  " 0.2 __DATE__"


struct EClockVal
{
    UINT32 ev_hi;
    UINT32 ev_lo;
};

struct TimeVal
{
	UINT32	tv_secs;
	UINT32	tv_micro;
};

struct TimeRequest
{
    IOStdReq tr_node;
    struct TimeVal   tr_time;
};

struct TimerUnit
{
	struct Unit tu_unit;
	APTR AddRequest;
};

typedef struct TimerBase
{
	struct Device		Device;
	APTR				Timer_SysBase;
	UINT32				MiscFlags;
	struct TimeVal		CurrentTime;
	struct TimeVal		VBlankTime;
	struct TimeVal		Elapsed;


	UINT32				TimerIRQ;
	struct Interrupt	*TimerVBLIntServer;
	struct Interrupt	*TimerMICROHZIntServer;

	UINT16				TimerCounter; // used in AddMHZDelay to know how much time the timer has already spent when we see a "new request"
	struct TimeVal		TimerIntTime; // used in TimerMICROHZIRQServer to subtract this time from all delay requests
	struct TimeVal		RequestedMinDelay; // used in AddMHZDelay to compare "newly requested delay" with "minimum delay requested till now"

	struct TimerUnit	TimerUnit[UNIT_MAX];

} TimerBase;

struct rtc_time
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

extern void (*TimerCmdVector[])(TimerBase *, pIOStdReq);
//extern INT8 TimerCmdQuick[];

void INTERN_EndCommand(TimerBase *TimerBase, UINT32 error, pIOStdReq ioreq);
void INTERN_QueueRequest(TimerBase *TimerBase, pIOStdReq ioreq);

void timer_BeginIO(TimerBase *TimerBase, pIOStdReq ioreq);
void timer_AbortIO(TimerBase *TimerBase, pIOStdReq ioreq);
INT32 timer_CmpTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_AddTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_SubTime(TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src);
UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src);

void TimerInvalid(TimerBase *TimerBase, pIOStdReq ioreq);
void TimerAddRequest(TimerBase *TimerBase, pIOStdReq ioreq);
void TimerGetSysTime(TimerBase *TimerBase, pIOStdReq ioreq);
void TimerSetSysTime(TimerBase *TimerBase, pIOStdReq ioreq);

void AddAlarm(TimerBase *TimerBase, pIOStdReq ioreq);
void AddDelay(TimerBase *TimerBase, pIOStdReq ioreq);
void AddMHZDelay(TimerBase *TimerBase, pIOStdReq ioreq);

void update_counter_value(TimerBase *TimerBase, struct TimeVal delay);
void update_interval_time(TimerBase *TimerBase, struct TimeVal delay);
UINT16 convert_microsec_to_counterval(TimerBase *TimerBase, UINT32 micro_sec);
void convert_counterval_to_microsec(TimerBase *TimerBase, UINT16 count, struct TimeVal *tval);

#endif


