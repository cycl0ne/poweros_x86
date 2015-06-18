#ifndef funcs_h
#define funcs_h

#include "types.h"
#include "devices.h"

/* Units */
#define UNIT_MICROHZ    0
#define UNIT_VBLANK     1
#define UNIT_ECLOCK     2
#define UNIT_WAITUNTIL  3
#define UNIT_WAITECLOCK 4

/* IO-Commands */
#define TR_ADDREQUEST (CMD_NONSTD+0)
#define TR_GETSYSTIME (CMD_NONSTD+1)
#define TR_SETSYSTIME (CMD_NONSTD+2)

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
    struct IORequest tr_node;
    struct TimeVal   tr_time;
};

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

// Timer Device Functions

void AddTime(struct TimeVal *src, struct TimeVal *dst);
INT32 CmpTime(struct TimeVal *src, struct TimeVal *dest);
void SubTime(struct TimeVal *src, struct TimeVal *dest);
void GetSysTime(struct TimeVal *src);
UINT32 ReadEClock(struct EClockVal *src);

#define AddTime(a,b)	(((VOID(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,7))(TimerBase, a, b))
#define CmpTime(a,b)	(((INT32(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,8))(TimerBase, a, b))
#define SubTime(a,b)	(((VOID(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,9))(TimerBase, a, b))
#define GetSysTime(a)	(((VOID(*)(APTR, struct TimeVal *)) _GETVECADDR(TimerBase,10))(TimerBase, a))
#define ReadEClock(a)	(((UINT32(*)(APTR, struct EClockVal *)) _GETVECADDR(TimerBase,11))(TimerBase, a))


#endif
