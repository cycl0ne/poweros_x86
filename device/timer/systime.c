#include "exec_interface.h"
#include "timer_intern.h"

void set_timer(UINT32 hz);
UINT16 get_timer();
__attribute__((no_instrument_function)) BOOL TimerMICROHZIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);


#define SysBase TimerBase->Timer_SysBase

void AddAlarm(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside AddAlarm!\n");
	UINT32 ipl;
	ipl = Disable();

	//KPrintF("TimerBase->CurrentTime->tv_secs = %d\n", TimerBase->CurrentTime.tv_secs);
	//KPrintF("TimerBase->CurrentTime->tv_micro = %d\n", TimerBase->CurrentTime.tv_micro);
	//KPrintF("((struct TimeRequest*)ioreq)->tr_time->tv_secs = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_secs);
	//KPrintF("((struct TimeRequest*)ioreq)->tr_time->tv_micro = %d\n", ((struct TimeRequest*)ioreq)->tr_time.tv_micro);

	// if CurrentTime >= tr_time, dont add alarm!
	if(timer_CmpTime(TimerBase, &TimerBase->CurrentTime, &((struct TimeRequest*)ioreq)->tr_time) >= 0)
	{
		Restore(ipl);
		KPrintF("Can't Add Alarm!\n");
		((struct TimeRequest*)ioreq)->tr_time.tv_secs = ((struct TimeRequest*)ioreq)->tr_time.tv_micro = 0;
		INTERN_EndCommand(TimerBase, 0, (struct IOStdReq *)ioreq);
	}
	else
	{
		// Ok, we add this to the list
		INTERN_QueueRequest(TimerBase, ioreq);

		Restore(ipl);

		//KPrintF("Alarm added!\n");
		//CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
		((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
	}
}

void AddMHZDelay(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside AddMHZDelay!\n");
	UINT16 current_counter_value = 0;

	UINT32 ipl;
	ipl = Disable();

	    // if initially RequestedMinDelay == 0, change RequestedMinDelay
	    if(TimerBase->RequestedMinDelay.tv_secs == 0 && TimerBase->RequestedMinDelay.tv_micro == 0)
	    {
			TimerBase->RequestedMinDelay.tv_secs = (((struct TimeRequest*)ioreq)->tr_time).tv_secs;
			TimerBase->RequestedMinDelay.tv_micro = (((struct TimeRequest*)ioreq)->tr_time).tv_micro;

			//unfortunately if user has requested a delay of 0 msec!
			if(	TimerBase->RequestedMinDelay.tv_secs == 0 && TimerBase->RequestedMinDelay.tv_micro == 0)
			{
				//dont set any timer
			}
			else
			{
				//set the timer...done
				update_counter_value(TimerBase, TimerBase->RequestedMinDelay);
				update_interval_time(TimerBase, TimerBase->RequestedMinDelay);
				set_timer(TimerBase->TimerCounter);
			}
		}
		// if newly requested delay < RequestedMinDelay, update RequestedMinDelay
		else if(timer_CmpTime(TimerBase, &((struct TimeRequest*)ioreq)->tr_time, &TimerBase->RequestedMinDelay) < 0)
		{
			TimerBase->RequestedMinDelay.tv_secs = (((struct TimeRequest*)ioreq)->tr_time).tv_secs;
			TimerBase->RequestedMinDelay.tv_micro = (((struct TimeRequest*)ioreq)->tr_time).tv_micro;

			//unfortunately if user has requested a delay of 0 msec!
			if(	TimerBase->RequestedMinDelay.tv_secs == 0 && TimerBase->RequestedMinDelay.tv_micro == 0)
			{
				//dont set any timer
			}
			else
			{
				//get the current counter value
				current_counter_value = get_timer();

				//get the diff
				UINT16 count_diff = TimerBase->TimerCounter - current_counter_value;
				struct TimeVal elapsed;

				//convert count diff to msec
				convert_counterval_to_microsec(TimerBase, count_diff, &elapsed);

				//update TimerIntTime
				TimerBase->TimerIntTime = elapsed;

				//call TimerMICROHZIRQServer
				TimerMICROHZIRQServer(0, TimerBase, SysBase);
			}
		}
		// if newly requested delay >= RequestedMinDelay, dont update RequestedMinDelay
		else
		{
			// dont update RequestedMinDelay
		}

		//queue request
	    INTERN_QueueRequest(TimerBase, ioreq);

	    Restore(ipl);

		//KPrintF("MHZ Delay added!\n");
	    //CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
	    ((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
}

void update_counter_value(TimerBase *TimerBase, struct TimeVal delay)
{
	struct TimeVal max_pit_delay;
	max_pit_delay.tv_secs = 0;
	max_pit_delay.tv_micro = 54925;

	struct TimeVal zero;
	zero.tv_secs = 0;
	zero.tv_micro = 0;

	// if delay >= max_pit_delay, TimerCounter = 65535
	if(timer_CmpTime(TimerBase, &delay, &max_pit_delay) >= 0)
	{
		TimerBase->TimerCounter = 65535;
	}
	// if delay > zero, TimerCounter = x
	else if(timer_CmpTime(TimerBase, &delay, &zero) > 0)
	{
		TimerBase->TimerCounter = convert_microsec_to_counterval(TimerBase, delay.tv_micro);
	}
	// if delay == zero, TimerCounter = 0;
	else
	{
		TimerBase->TimerCounter = 0;
	}
}

void update_interval_time(TimerBase *TimerBase, struct TimeVal delay)
{
	struct TimeVal max_pit_delay;
	max_pit_delay.tv_secs = 0;
	max_pit_delay.tv_micro = 54925;

	struct TimeVal zero;
	zero.tv_secs = 0;
	zero.tv_micro = 0;

	// if delay >= max_pit_delay, TimerIntTime = 54925
	if(timer_CmpTime(TimerBase, &delay, &max_pit_delay) >= 0)
	{
		TimerBase->TimerIntTime.tv_secs = 0;
		TimerBase->TimerIntTime.tv_micro = 54925;
	}
	// if delay > zero, TimerIntTime = x
	else if(timer_CmpTime(TimerBase, &delay, &zero) > 0)
	{
		TimerBase->TimerIntTime.tv_secs = 0;
		TimerBase->TimerIntTime.tv_micro = delay.tv_micro;
	}
	// if delay == zero, TimerIntTime = 0;
	else
	{
		TimerBase->TimerIntTime.tv_secs = 0;
		TimerBase->TimerIntTime.tv_micro = 0;
	}
}

UINT16 convert_microsec_to_counterval(TimerBase *TimerBase, UINT32 micro_sec)
{
	return (UINT16)((1.193180 * micro_sec) + 1);
}

void convert_counterval_to_microsec(TimerBase *TimerBase, UINT16 count, struct TimeVal *tval)
{
	tval->tv_secs = 0;
	tval->tv_micro = (UINT16)(0.838096 * count);
}

void AddDelay(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	//KPrintF("Inside AddDelay!\n");
	UINT32 ipl;
	ipl = Disable();

	    timer_AddTime(TimerBase, &TimerBase->Elapsed, &((struct TimeRequest*)ioreq)->tr_time);

	    INTERN_QueueRequest(TimerBase, ioreq);

	    Restore(ipl);

		//KPrintF("Delay added!\n");
	    //CLEAR_BITS(((struct TimeRequest*)ioreq)->tr_node.io_Flags, IOF_QUICK);
	    ((struct TimeRequest*)ioreq)->tr_node.io_Flags &= ~IOF_QUICK;
}

void INTERN_QueueRequest(TimerBase *TimerBase, struct IOStdReq *ioreq)
{
	UINT32 ipl = Disable();

	struct Unit	*unit = ioreq->io_Unit;

	if (ioreq->io_Error == 0)
	{
		PutMsg(&unit->unit_MsgPort, &ioreq->io_Message);
	}
	Restore(ipl);
	return;
}
