/**
 * @file dosbase_private.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#ifndef dosbase_h
#define dosbase_h

#define DOS_INTERNAL

#include "types.h"
#include "lists.h"
//#include "tasks.h"
//#include "interrupts.h"
//#include "devices.h"

#include "libraries.h"
#include "residents.h"
#include "semaphores.h"
#include "memory.h"
#include "ports.h"
//#include "utility.h"
#include "timer.h"

#include "dos.h"
#include "dos_io.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_tags.h"

#include "exec_interface.h"
#include "utility_interface.h"
#include "dos_interface.h"
#define SAME 0

#define MAX_FILENAME	255
#define MAX_DEVICENAME	30

/* Flags to be passed to LockDosList(), etc */
#define LDB_DEVICES	2
#define LDF_DEVICES	(1L << LDB_DEVICES)
#define LDB_VOLUMES	3
#define LDF_VOLUMES	(1L << LDB_VOLUMES)
#define LDB_ASSIGNS	4
#define LDF_ASSIGNS	(1L << LDB_ASSIGNS)
#define LDB_ENTRY	5
#define LDF_ENTRY	(1L << LDB_ENTRY)
#define LDB_DELETE	6
#define LDF_DELETE	(1L << LDB_DELETE)

typedef struct CliProcList {
	MinNode		cpl_Node;
	UINT32		cpl_Number;
	pComLinInt	cpl_CLI;
} CliProcList, *pCliProcList;

typedef struct ErrorString 
{
	INT32*	estr_Nums;
	UINT8*	estr_Strings;
}ErrorString, *pErrorString;

typedef struct DOSBase {
	Library				dos_LibNode;
	pSysBase			dos_SysBase;
	pUtilBase			dos_UtilBase;
	MinList				dos_DosList;
	List				dos_SegList;
	MinList				dos_CliList;
	UINT32				dos_CliNum;
	SignalSemaphore		dos_DosListLock;
	SignalSemaphore 	dos_EntryLock;
	SignalSemaphore		dos_DeleteLock;

	SignalSemaphore		dos_SegLock;	// Read/Write Access to SegList
	SignalSemaphore		dos_CliLock;	// Read/Write Access to CliList
	struct TimeRequest	dos_TimerIO;
	struct DateStamp	dos_Time;
	pErrorString		dos_Errors;
	Segment				dos_Shell;
	Segment				dos_Console;
	Segment				dos_ConTTY;
	Segment				dos_FileSystem;
	Segment				dos_RAMHandler;
}DOSBase, *pDOSBase;

#define SysBase DOSBase->dos_SysBase
#define UtilBase DOSBase->dos_UtilBase

#endif
