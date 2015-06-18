#include "timer_intern.h"
#include "exec_interface.h"

void set_timer(UINT32 hz);

#define FastAddTime(d, s)\
    (d)->tv_micro += (s)->tv_micro;\
    (d)->tv_secs += (s)->tv_secs;\
    if((d)->tv_micro > 999999) {\
		(d)->tv_secs++;\
		(d)->tv_micro -= 1000000;}\

#define FastSubTime(d, s)\
	if((d)->tv_micro < (s)->tv_micro) {\
		(d)->tv_micro += 1000000;\
		(d)->tv_secs--;}\
    (d)->tv_micro -= (s)->tv_micro;\
    (d)->tv_secs -= (s)->tv_secs;


__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase)
{
	//DPrintF("TimerVBLIRQServer\n");
	FastAddTime(&TimerBase->CurrentTime, &TimerBase->VBlankTime);
	FastAddTime(&TimerBase->Elapsed, &TimerBase->VBlankTime);

	//DPrintF("TimerBase->CurrentTime->tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	//DPrintF("TimerBase->CurrentTime->tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	//DPrintF("TimerBase->Elapsed->tv_secs = %d\n", TimerBase->Elapsed.tv_secs);
	//DPrintF("TimerBase->Elapsed->tv_micro = %d\n", TimerBase->Elapsed.tv_micro);

	struct TimeRequest *tr, *trtmp;
	for (int unit_num=0; unit_num < UNIT_MAX; unit_num++)
	{
		switch(unit_num)
		{
		case UNIT_VBLANK:
		//dont touch delay requests for MHZ unit, they are specially processed
		//case UNIT_MICROHZ:
		case UNIT_ECLOCK:
			ForeachNodeSafe(&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList, tr, trtmp)
			{
				if ((tr->tr_time.tv_secs < TimerBase->Elapsed.tv_secs)
				 ||((tr->tr_time.tv_secs <= TimerBase->Elapsed.tv_secs)
				 && (tr->tr_time.tv_micro < TimerBase->Elapsed.tv_micro)))
				{
					//KPrintF("Found something from delay\n");
					Remove((struct Node *)tr);
					tr->tr_time.tv_secs = tr->tr_time.tv_micro = 0;
					tr->tr_node.io_Error = 0;
					ReplyMsg((struct Message *)tr);
				}
			}
			break;
		case UNIT_WAITUNTIL:
		case UNIT_WAITECLOCK:
			ForeachNodeSafe(&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList, tr, trtmp)
			{
				if ((tr->tr_time.tv_secs < TimerBase->CurrentTime.tv_secs)
				 ||((tr->tr_time.tv_secs <= TimerBase->CurrentTime.tv_secs)
				 && (tr->tr_time.tv_micro < TimerBase->CurrentTime.tv_micro)))
				{
					//KPrintF("Found something from alarm\n");
					Remove((struct Node *)tr);
					tr->tr_time.tv_secs = tr->tr_time.tv_micro = 0;
					tr->tr_node.io_Error = 0;
					ReplyMsg((struct Message *)tr);
				}
			}
			break;
		};
	}

	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}

__attribute__((no_instrument_function)) BOOL TimerMICROHZIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase)
{
	//DPrintF("TimerMICROHZIRQServer\n");

	//DPrintF("TimerBase->CurrentTime->tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	//DPrintF("TimerBase->CurrentTime->tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	//DPrintF("TimerBase->Elapsed->tv_secs = %d\n", TimerBase->Elapsed.tv_secs);
	//DPrintF("TimerBase->Elapsed->tv_micro = %d\n", TimerBase->Elapsed.tv_micro);

	struct TimeRequest *tr, *trtmp;

	struct TimeVal min;
	min.tv_secs = 0;
	min.tv_micro = 0;

	int unit_num = UNIT_MICROHZ;
	ForeachNodeSafe(&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList, tr, trtmp)
	{
		//subtract "already spent time" from "requested delay"
		FastSubTime(&tr->tr_time, &TimerBase->TimerIntTime);

		//lets find minimum delay of all requests in this if-else ladder
		//if requested delay has come to zero, ignore
		if((tr->tr_time).tv_secs == 0 && (tr->tr_time).tv_micro == 0)
		{
			//dont do anything, this request will be removed soon
		}
		//assume first non zero delay as minimum
		else if(min.tv_secs == 0 && min.tv_micro == 0)
		{
			min.tv_secs = (tr->tr_time).tv_secs;
			min.tv_micro = (tr->tr_time).tv_micro;
		}
		// if requested delay <= min, min =
		else if(timer_CmpTime(TimerBase, &(tr->tr_time), &min) <= 0)
		{
			min.tv_secs = (tr->tr_time).tv_secs;
			min.tv_micro = (tr->tr_time).tv_micro;
		}

		//lets remove the requested delay that has come to zero
		if ((tr->tr_time).tv_secs == 0 && (tr->tr_time).tv_micro == 0)
		{
			//KPrintF("Found something from MHZ delay\n");
			Remove((struct Node *)tr);
			tr->tr_time.tv_secs = tr->tr_time.tv_micro = 0;
			tr->tr_node.io_Error = 0;
			ReplyMsg((struct Message *)tr);
		}
	}

	//we collected minimum of all requested delay
	TimerBase->RequestedMinDelay.tv_secs = min.tv_secs;
	TimerBase->RequestedMinDelay.tv_micro = min.tv_micro;

	//if request list is empty now
	if(	TimerBase->RequestedMinDelay.tv_secs == 0 && TimerBase->RequestedMinDelay.tv_micro == 0)
	{
		//dont set any timer
	}
	else
	{
	    update_counter_value(TimerBase, TimerBase->RequestedMinDelay);
		update_interval_time(TimerBase, TimerBase->RequestedMinDelay);
		set_timer(TimerBase->TimerCounter);
	}

	return 0; // we return 0 so that Tick() can run, otherwise we would cut off Schedule()
}
