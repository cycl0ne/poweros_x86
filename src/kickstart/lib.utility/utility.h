#ifndef UTILITY_H
#define UTILITY_H

#include "types.h"
#include "tagitem.h"
#include "name.h"
#include "exec_funcs.h"

struct ClockData
{
	UINT16	sec;
	UINT16	min;
	UINT16	hour;
	UINT16	day;
	UINT16	month;
	UINT16	year;
	UINT16	wday;
};

typedef struct UtilityBase {
	struct Library	Library;
	APTR			SysBase;
	APTR			MasterSpace;
} Uility, *pUtility;

#endif
