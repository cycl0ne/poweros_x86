#ifndef funcs_h
#define funcs_h

#include "timer.h"

void AddTime(struct TimeVal *src, struct TimeVal *dst);
INT32 CmpTime(struct TimeVal *src, struct TimeVal *dest);
void SubTime(struct TimeVal *src, struct TimeVal *dest);

void GetSysTime(struct TimeVal *src);
UINT32 ReadEClock(struct EClockVal *src);

#define AddTime(a,b)	(((VOID(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,5))(TimerBase, a, b))
#define CmpTime(a,b)	(((INT32(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,6))(TimerBase, a, b))
#define SubTime(a,b)	(((VOID(*)(APTR, struct TimeVal *, struct TimeVal *)) _GETVECADDR(TimerBase,7))(TimerBase, a, b))

#define GetSysTime(a)	(((VOID(*)(APTR, struct TimeVal *)) _GETVECADDR(TimerBase,8))(TimerBase, a))
#define ReadEClock(a)	(((UINT32(*)(APTR, struct EClockVal *)) _GETVECADDR(TimerBase,9))(TimerBase, a))


#endif
