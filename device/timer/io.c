#include "exec_interface.h"
#include "timer_intern.h"

#define SysBase TimerBase->Timer_SysBase

void TimerInvalid(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside TimerInvalid!\n");
	INTERN_EndCommand(TimerBase, IOERR_NOCMD, ioreq);
}

void TimerAddRequest(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
//	KPrintF("Inside TimerAddRequest!\n");
	struct TimerUnit* tu;
	tu = (struct TimerUnit *)((struct TimeRequest*)ioreq)->tr_node.io_Unit;
	((void(*)(struct TimerBase *, struct IOStdReq *))tu->AddRequest)(TimerBase, ioreq);
}

void TimerGetSysTime(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside TimerGetSysTime!\n");
	((struct TimeRequest*)ioreq)->tr_time.tv_micro = TimerBase->CurrentTime.tv_micro;
	((struct TimeRequest*)ioreq)->tr_time.tv_secs  = TimerBase->CurrentTime.tv_secs;

	INTERN_EndCommand(TimerBase, 0, (struct IOStdReq *)ioreq);
}

void TimerSetSysTime(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside TimerSetSysTime!\n");
	UINT32 ipl = Disable();
	//KPrintF("TimerBase->CurrentTime.tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	//KPrintF("TimerBase->CurrentTime.tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	//KPrintF("((struct TimeRequest*)ioreq)->tr_time.tv_secs = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_secs);
	//KPrintF("((struct TimeRequest*)ioreq)->tr_time.tv_micro = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_micro);
	TimerBase->CurrentTime.tv_secs = ((struct TimeRequest*)ioreq)->tr_time.tv_secs;
	TimerBase->CurrentTime.tv_micro= ((struct TimeRequest*)ioreq)->tr_time.tv_micro;
	Restore(ipl);

	INTERN_EndCommand(TimerBase, 0, (struct IOStdReq *)ioreq);
}


void (*TimerCmdVector[])(TimerBase *, struct IOStdReq * ) =
{
	TimerInvalid,
	0, //TimerReset
	0, //TimerRead
	0, //TimerWrite
	0, //TimerUpdate
	0, //TimerClear
	0, //TimerStopCmd
	0, //TimerStart
	0, //TimerFlush

	TimerAddRequest,
	TimerGetSysTime,
	TimerSetSysTime
};

/*
INT8 TimerCmdQuick[] =
// -1 : not queued
// 0  : queued
{
	-1, //TimerInvalid
	-1, //TimerReset
	-1, //TimerRead
	-1, //TimerWrite
	-1, //TimerUpdate
	-1, //TimerClear
	-1, //TimerStopCmd
	-1, //TimerStart
	-1, //TimerFlush

	0, //TimerAddRequest
	-1, //TimerGetSysTime
	-1 //TimerSetSysTime
};
*/

void INTERN_EndCommand(TimerBase *TimerBase, UINT32 error, struct IOStdReq *ioreq)
{
	ioreq->io_Error = error;
	if (TEST_BITS(ioreq->io_Flags, IOF_QUICK)) return;
	ReplyMsg((struct Message *)ioreq);
	return;
}
