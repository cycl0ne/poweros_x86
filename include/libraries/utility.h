#ifndef UTILITY_H
#define UTILITY_H

#include "types.h"
#include "tagitem.h"
#include "name.h"

// for String compare
#define SAME 0

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
	APTR			RootSpace;
	UINT32			SeedNext;
	UINT32			RandSeed;
} UilityBase, *pUtilBase;

#endif
