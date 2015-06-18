#ifndef UTILITY_H
#define UTILITY_H

#include "types.h"
#include "libraries.h"
#include "tagitem.h"
#include "name.h"
#include "exec_interface.h"
#include "utility_interface.h"

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
} Uility, *pUtility;

//return TRUE if node 'a' is less than or equal to node 'b'
typedef BOOL(*pNodeCmpLe)(Node* a, Node* b);

#endif
