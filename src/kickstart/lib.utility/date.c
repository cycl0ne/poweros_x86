#include "utility.h"
#include "utility_funcs.h"

#define CALC_DATE(YEAR,MONTH,DAY) \
  ( \
  ((YEAR)-1+((MONTH)+9)/12)*365L \
  +((YEAR)-1+((MONTH)+9)/12)/4 \
  -((YEAR)-1+((MONTH)+9)/12)/100 \
  +((YEAR)-1+((MONTH)+9)/12)/400 \
  +((((MONTH)+9)%12)*306+5)/10 \
  +(DAY) \
  -1 \
 )

UINT32 util_CalcDate(pUtility UtilBase, UINT32 year, UINT32 month, UINT32 day)
{
	month += 9;
	year = year-1 + (month/12);
	month %= 12;
	month = month*306 +5;
	return (year*365) + (year/4) - (year/100) + (year/400) + (month/10) + day -1;
}

void util_Os2Date(pUtility UtilBase, UINT32 amiga, struct ClockData *cd)
{
	INT32 year;
	INT32 month;
	INT32 day;
	INT32 hour;
	INT32 minute;
	INT32 second;
	INT32 absday;
	INT32 work;

	second  = amiga % 60;
	minute  = amiga / 60;
	hour    = minute / 60;
	minute %= 60;
	hour   %= 24;

	absday  = amiga / (60 * 60 * 24);
	absday += CALC_DATE(1978,1,1);

	cd->wday  = (absday+3)%7;

	work = absday;
	work -= (absday + 1) / 146097;
	work += work / 36524;
	work -= (work + 1) / 1461;
	year = work / 365;

	work = absday;
	work -= year * 365;
	work -= year / 4;
	work += year / 100;
	work -= year / 400;

	month = work / 153;
	month *= 5;
	month += 10 * (work % 153) / 305;

	day = 1 + work;
	day -= (long)((month * 306 + 5) / 10);

	month += 2;
	year += month / 12;
	month %= 12;
	month++;

	cd->sec   = second;
	cd->min   = minute;
	cd->hour  = hour;
	cd->day   = day;
	cd->month = month;
	cd->year  = year;
}

UINT32 util_Date2Os(pUtility UtilBase, struct ClockData *cd)
{
	return (((CalcDate(cd->year,cd->month,cd->day) - CALC_DATE(1978,1,1)) * 24 + cd->hour) * 60 + cd->min) * 60 + cd->sec;
}

UINT32 util_CheckDate(pUtility UtilBase, struct ClockData *date)
{
	struct ClockData date2;
	UINT32 date1 = Date2Amiga(date);
	Amiga2Date(date1, &date2);
	
	if ((date->sec	== date2.sec) &&
		(date->min	== date2.min) &&
		(date->hour	== date2.hour) &&
		(date->day	== date2.day) &&
		(date->month== date2.month) &&
		(date->year	== date2.year)) return date1;
	return NULL;
}
