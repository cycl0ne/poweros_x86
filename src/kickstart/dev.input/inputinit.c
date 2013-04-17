#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"

#include "inputdev.h"

#include "sysbase.h"
#include "exec_funcs.h"

#define DEVICE_VERSION_STRING "\0$VER: input.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static const char name[] = "input.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

APTR idev_OpenDev(struct IDBase *IDBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR idev_CloseDev(struct IDBase *IDBase, struct IORequest *ioreq);
APTR idev_ExpungeDev(struct IDBase *IDBase);
APTR idev_ExtFuncDev(struct IDBase *IDBase);
void idev_BeginIO(IDBase *IDBase, struct IORequest *ioreq);
void idev_AbortIO(IDBase *IDBase, struct IORequest *ioreq);
static struct IDBase *idev_Init(struct IDBase *IDBase, UINT32 *segList, struct SysBase *SysBase);


static APTR FuncTab[] = 
{
	(void(*)) idev_OpenDev,
	(void(*)) idev_CloseDev,
	(void(*)) idev_ExpungeDev,
	(void(*)) idev_ExtFuncDev,

	(void(*)) idev_BeginIO,
	(void(*)) idev_AbortIO,
	(APTR) ((UINT32)-1)
};

static volatile const APTR InitTab[4]=
{
	(APTR)sizeof(struct IDBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)idev_Init
};

static volatile const struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	40,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

UINT32 idev_InputTask(APTR data, struct SysBase *SysBase);

static struct IDBase *idev_Init(struct IDBase *IDBase, UINT32 *segList, struct SysBase *SysBase)
{
	IDBase->Device.dd_Library.lib_OpenCnt = 0;
	IDBase->Device.dd_Library.lib_Node.ln_Pri = 0;
	IDBase->Device.dd_Library.lib_Node.ln_Type = NT_DEVICE;
	IDBase->Device.dd_Library.lib_Node.ln_Name = (STRPTR)name;
	IDBase->Device.dd_Library.lib_Version = DEVICE_VERSION;
	IDBase->Device.dd_Library.lib_Revision = DEVICE_REVISION;
	IDBase->Device.dd_Library.lib_IDString = (STRPTR)&version[7];
	
	IDBase->SysBase	= SysBase;

	NewList(&IDBase->Unit.unit_MsgPort.mp_MsgList);

	IDBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	IDBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)name;
	
	IDBase->id_Thresh.tv_secs	= 0;
	IDBase->id_Thresh.tv_micro	= 800000;
	IDBase->id_Period.tv_secs	= 0;
	IDBase->id_Period.tv_micro	= 100000;
	IDBase->id_RepeatCode		= -1;

	IDBase->id_MType		= GPCT_MOUSE;
	IDBase->id_MTrig.Keys	= GPTF_DOWNKEYS|GPTF_UPKEYS;
	IDBase->id_MTrig.XDelta	= 1;
	IDBase->id_MTrig.YDelta	= 1;
	
/*
ID_QUALMASK	EQU	$00FF
ID_KEYMASK	EQU	$07FF
ID_MOUSEMASK	EQU	$F000
 */
	if (OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest*)&IDBase->id_RIOR, 0) == 0)
	{
		IDBase->id_TIOR.tr_node.io_Device	= IDBase->id_RIOR.tr_node.io_Device;
		IDBase->id_TIOR.tr_node.io_Unit		= IDBase->id_RIOR.tr_node.io_Unit;
		if (OpenDevice("keyboard.device", 0, (struct IORequest*)&IDBase->id_K1IOR, 0) == 0)
		{
			IDBase->id_K2IOR.io_Device	= IDBase->id_K1IOR.io_Device;
			IDBase->id_K2IOR.io_Unit	= IDBase->id_K1IOR.io_Unit;
			NewList(&IDBase->id_HandlerList);
			IDBase->id_BootTask = FindTask(NULL);
			IDBase->id_Task = TaskCreate(name, idev_InputTask, IDBase, 8192, 20);
			Wait(SIGF_SINGLE);
			return IDBase;
		}
		CloseDevice((struct IORequest*)&IDBase->id_TIOR);
		DPrintF("Error opening keyboard.dev\n");
		return NULL;
	}
	DPrintF("Error opening timer.dev\n");
	return NULL;
//INT32 lib_OpenDevice(struct SysBase *SysBase, STRPTR devName, UINT32 unitNum, struct IORequest *iORequest, UINT32 flags)
//	return IDBase;
}

#define SysBase IDBase->SysBase
void idev_BeginIO(IDBase *IDBase, struct IORequest *io)
{
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;
	
	if (cmd > MD_SETTRIGGER) cmd = 0; // Invalidate the command.

	if (inputCmdQuick[cmd] >= 0)
	{
		QueueCommand((struct IOStdReq*)io, SysBase);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		// If Device is stopped, just return
		if (TEST_BITS(IDBase->Unit.unit_Flags, DUB_STOPPED))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;	
		}
		// we are first in Queue, now we are Quick, otherwise we come from the IS Routine
	}
	inputCmdVector[cmd]((struct IOStdReq*) io, IDBase);
}

void idev_AbortIO(IDBase *IDBase, struct IORequest *ioreq)
{
	EndCommand(IOERR_ABORTED, (struct IOStdReq*)ioreq, SysBase);
}

static const char EndResident = 0;

