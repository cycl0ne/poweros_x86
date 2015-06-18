#include "timer_intern.h"
#include "residents.h"

// Include our Protos
#include "exec_interface.h"
//#include "timer.h"

static char DevName[] = "timer.device";
static char Version[] = "\0$VER: timer.device 0.1 ("__DATE__")\r\n";
#define TIMER_INT_PRI 0
#define TICK  128
#define IRQ_CLK  0
#define IRQ_RTC  8

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"


struct TimerBase *timer_OpenDev(struct TimerBase *TimerBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags);
APTR timer_CloseDev(struct TimerBase *TimerBase, struct IOStdReq *ioreq);
UINT32 *timer_ExpungeDev(struct TimerBase *TimerBase);
UINT32 timer_ExtFuncDev(void);
void timer_BeginIO(TimerBase *TimerBase, struct IOStdReq *ioreq);
void timer_AbortIO(TimerBase *TimerBase, struct IOStdReq *ioreq);
INT32 timer_CmpTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_AddTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_SubTime(struct TimerBase *TimerBase, struct TimeVal *src, struct TimeVal *dst);
void timer_GetSysTime(struct TimerBase *TimerBase, struct TimeVal *src);
UINT32 timer_ReadEClock(struct TimerBase *TimerBase, struct EClockVal *src);
__attribute__((no_instrument_function)) BOOL TimerVBLIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);
__attribute__((no_instrument_function)) BOOL TimerMICROHZIRQServer(UINT32 number, TimerBase *TimerBase, APTR SysBase);
void set_timer(UINT32 hz);
UINT16 get_timer();
void start_pit_mode_4();

APTR timer_FuncTab[] =
{
 (void(*)) timer_OpenDev,
 (void(*)) timer_CloseDev,
 (void(*)) timer_ExpungeDev,
 (void(*)) timer_ExtFuncDev,

 (void(*)) timer_BeginIO,
 (void(*)) timer_AbortIO,

 (void(*)) timer_AddTime,
 (void(*)) timer_CmpTime,
 (void(*)) timer_SubTime,
 (void(*)) timer_GetSysTime,
 (void(*)) timer_ReadEClock,

 (APTR) ((UINT32)-1)
};

#include "asm.h"

#define from_bcd(val)  ((val / 16) * 10 + (val & 0xf))
static void cmos_dump(uint16_t * values) 
{
	uint16_t index;
	for (index = 0; index < 128; ++index) 
	{
		outb(0x70, index);
		values[index] = inb(0x71);
	}
}

static uint32_t secs_of_month(int months, int year) 
{
	year += 2000;

	uint32_t days = 0;
	switch(months) {
		case 11:
			days += 30;
		case 10:
			days += 31;
		case 9:
			days += 30;
		case 8:
			days += 31;
		case 7:
			days += 31;
		case 6:
			days += 30;
		case 5:
			days += 31;
		case 4:
			days += 30;
		case 3:
			days += 31;
		case 2:
			days += 28;
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0))) {
				days++;
			}
		case 1:
			days += 31;
		default:
			break;
	}
	return days * 86400;
}

static uint32_t secs_of_years(int years) 
{
	uint32_t days = 0;
	years += 2000;
	while (years > 1969) {
		days += 365;
		if (years % 4 == 0) {
			if (years % 100 == 0) {
				if (years % 400 == 0) {
					days++;
				}
			} else {
				days++;
			}
		}
		years--;
	}
	return days * 86400;
}

int gettimeofday(struct TimeVal *t) 
{
	uint16_t values[128];
	cmos_dump(values);

	/* Math Time */
	uint32_t time = secs_of_years(from_bcd(values[9]) - 1) +
					secs_of_month(from_bcd(values[8]) - 1, from_bcd(values[9])) + 
					(from_bcd(values[7]) - 1) * 86400 +
					(from_bcd(values[4])) * 3600 +
					(from_bcd(values[2])) * 60 +
					from_bcd(values[0]) +
					0;
	t->tv_secs = time;
	t->tv_micro = 0;
	return 0;
}

void
get_date(
		uint16_t * month,
		uint16_t * day
		) {
	uint16_t values[128]; /* CMOS dump */
	cmos_dump(values);

	*month = from_bcd(values[8]);
	*day   = from_bcd(values[7]);
}


static void get_time(
		uint16_t * hours,
		uint16_t * minutes,
		uint16_t * seconds
		) {
	uint16_t values[128]; /* CMOS dump */
	cmos_dump(values);

	*hours   = from_bcd(values[4]);
	*minutes = from_bcd(values[2]);
	*seconds = from_bcd(values[0]);
}

struct TimerBase *timer_InitDev(struct TimerBase *TimerBase, UINT32 *segList, struct SysBase *SysBase)
{
	TimerBase->Timer_SysBase = SysBase;
	
	for(int unit_num=0; unit_num < UNIT_MAX; unit_num++)
	{
		// Initialise Unit Command Queue
		NewList((struct List *)&TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_MsgList);
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)DevName;
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
		TimerBase->TimerUnit[unit_num].tu_unit.unit_MsgPort.mp_SigTask = NULL;
		
		// Initialize different behaviours of Units
		switch(unit_num)
		{
			case UNIT_VBLANK:
			case UNIT_ECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddDelay;
			break;
			case UNIT_MICROHZ:
			TimerBase->TimerUnit[unit_num].AddRequest = AddMHZDelay;
			break;
			case UNIT_WAITUNTIL:
			case UNIT_WAITECLOCK:
			TimerBase->TimerUnit[unit_num].AddRequest = AddAlarm;
			break;
		};
	}
	
	//VBL Timer
	TimerBase->TimerVBLIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerVBLIRQServer, TimerBase);
	AddIntServer(IRQ_RTC, TimerBase->TimerVBLIntServer);
	
	//MICROHZ Timer
	TimerBase->TimerMICROHZIntServer = CreateIntServer(DevName, TIMER_INT_PRI, TimerMICROHZIRQServer, TimerBase);
	AddIntServer(IRQ_CLK, TimerBase->TimerMICROHZIntServer);
	start_pit_mode_4();
	struct TimeVal tv;
	gettimeofday(&tv);
	TimerBase->CurrentTime.tv_secs	= tv.tv_secs;
	TimerBase->CurrentTime.tv_micro	= tv.tv_micro;
	//set_timer(0);
	/*
	set_timer(0);
	UINT16 val = 0;
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	set_timer(40000);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	val = get_timer();
	DPrintF("Now val = %d\n", val);
	*/
	
	return TimerBase;
}

static const struct TimerBase TimerDevData =
{
	.Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.Device.dev_Node.ln_Type = NT_DEVICE,
	.Device.dev_Node.ln_Pri = 50,
	.Device.dev_OpenCnt = 0,
	.Device.dev_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.Device.dev_NegSize = 0,
	.Device.dev_PosSize = 0,
	.Device.dev_Version = VERSION,
	.Device.dev_Revision = REVISION,
	.Device.dev_Sum = 0,
	.Device.dev_IDString = (APTR)&Version[7],

	.CurrentTime.tv_secs  = 0,
	.CurrentTime.tv_micro = 0,

	.VBlankTime.tv_secs = 0,
	.VBlankTime.tv_micro = 1000000/TICK,

	.Elapsed.tv_secs = 0,
	.Elapsed.tv_micro = 0
};

// ROMTAG Resident
struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} timer_InitTab =
{
	sizeof(struct TimerBase),
	timer_FuncTab,
	(APTR)&TimerDevData,
	timer_InitDev
};
static APTR TimerEndResident;

RESIDENT_TAG TimerRomTag =
{
	RTC_MATCHWORD,
	&TimerRomTag,
	&TimerEndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	VERSION,
	NT_DEVICE,
	50,
	DevName,
	Version,
	&timer_InitTab
};

