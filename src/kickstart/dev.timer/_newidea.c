#include "types.h"
#include "device.h"
#include "io.h"
#include "list.h"

#define MAXUNIT 5	// 5 Units

#define UNIT_MICROHZ    0
#define UNIT_VBLANK     1
#define UNIT_ECLOCK     2
#define UNIT_WAITUNTIL  3
#define UNIT_WAITECLOCK 4

#define TR_ADDREQUEST (CMD_NONSTD+0)
#define TR_GETSYSTIME (CMD_NONSTD+1)
#define TR_SETSYSTIME (CMD_NONSTD+2)

typedef struct TimerUnit
{
	struct MinList	*tu_UnitList;
	APTR			tu_AddReq;
	APTR			tu_RemReq;
} TimerUnit_T, *pTimerUnit;

typedef struct TimerDevice
{
	struct Device		Device;
	APTR				SysBase;
	UINT32				MiscFlags; // Not Used ATM

	struct TimeVal		CurrentTime;
	struct TimeVal		VBlankTime;
	struct TimeVal		Elapsed;

	struct TimerUnit	Unit[MAXUNIT];
	struct MinList		EClockList;
	struct MinList		VBlankList;
	struct MinList		SysTimeList;

	struct Interrupt	TimerVBLIntServer;
	struct Interrupt	TimerECLOCKIntServer;
} TimerDevice_T, *pTimerDevice;

static void _EndCommand(struct IORequest *ioreq, UINT32 error)
{
	APTR SysBase = ((struct TimerDevice *)ioreq->io_Device)->SysBase;
	ioreq->io_Error = error;
	if (TEST_BITS(ioreq->io_Flags, IOF_QUICK)) return;
	ReplyMsg((struct Message *)ioreq);
	return;
}

void TimerAddRequest(struct IORequest *io)
{
	CLEAR_BITS(io->io_Flags, IOF_QUICK);
	pTimerUnit	TUnit = (pTimerUnit)io->io_Unit;
	TUnit->tu_AddReq(io->io_Device, io, TUnit->tu_UnitList);
}

void (*TimerCmdVector[])(struct IORequest * ) =
{
	TimerInvalid, 		// 0 - CMD_NONSTD -1
	TimerAddRequest, 	// TR_ADDREQUEST
	TimerGetSysTime,	// TR_GETSYSTEME
	TimerSetSysTime		// TR_SETSYSTIME
};

void timer_BeginIO(pTimerDevice *TimerBase, struct IORequest *ioreq)
{
	UINT8 cmd = ioreq->io_Command;
	ioreq->io_Error = 0;

	// We only support our 3 commands, every other Command is put to Invalid
	if (cmd > TR_SETSYSTIME || cmd < TR_ADDREQUEST) cmd = 0;
	else cmd -= CMD_NONSTD+1; // Adjust it to our Index
	TimerCmdVector[cmd](ioreq);
}



