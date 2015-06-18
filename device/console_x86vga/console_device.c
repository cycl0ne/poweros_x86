/**
 * @file console_device.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "console_device.h"
#include "residents.h"

#define DEVICE_VERSION_STRING "\0$VER: consolex86.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static char DevName[] = "consolex86.device";
static char Version[] = "\0$VER: consolex86.device 0.1 ("__DATE__")\r\n";

/*******************

Prototypes

********************/

static pConBase dev_InitDev(pConBase devBase, UINT32 *segList, pSysBase execBase);
static pConBase dev_OpenDev(pConBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags);
static pConBase dev_CloseDev(pConBase devBase, struct IOStdReq *ioreq);
static pConBase dev_ExpungeDev(pConBase devBase);
static pConBase dev_ExtFuncDev(pConBase devBase);
static void dev_BeginIO(pConBase devBase, pIOStdReq io);
static void dev_AbortIO(pConBase devBase, pIOStdReq io);

void con_EndCommand(struct IOStdReq *io, UINT32 error);
extern void (*con_commandVector[])(pIOStdReq);

/*******************

Function Table

********************/

static APTR volatile FuncTab[] =
{
	(void(*)) dev_OpenDev,
	(void(*)) dev_CloseDev,
	(void(*)) dev_ExpungeDev,
	(void(*)) dev_ExtFuncDev,

	(void(*)) dev_BeginIO,
	(void(*)) dev_AbortIO,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/

static const struct ConBase DevData =
{
	.dev_Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.dev_Device.dev_Node.ln_Type = NT_DEVICE,
	.dev_Device.dev_Node.ln_Pri = 50,
	.dev_Device.dev_OpenCnt = 0,
	.dev_Device.dev_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.dev_Device.dev_NegSize = 0,
	.dev_Device.dev_PosSize = 0,
	.dev_Device.dev_Version = DEVICE_VERSION,
	.dev_Device.dev_Revision = DEVICE_REVISION,
	.dev_Device.dev_Sum = 0,
	.dev_Device.dev_IDString = (APTR)&Version[7],
};

/*******************

ROMTAG RESIDENT

********************/

static struct InitTable
{
	UINT32	LibBaseSize;
	APTR	volatile FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} InitTab =
{
	sizeof(struct ConBase),
	FuncTab,
	(APTR)&DevData,
	dev_InitDev
};

static APTR EndResident;

volatile static RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	50,
	DevName,
	Version,
	&InitTab
};

/*******************

DEVICE FUNCTIONS

********************/

static pConBase dev_InitDev(pConBase devBase, UINT32 *segList, pSysBase SysBase)
{
	devBase->dev_SysBase = SysBase;

//	devBase->dev_BootTask	= FindTask(NULL);
//	devBase->dev_Task = CreateTask("pata.device", (Task_Function)dev_PataTask, devBase, 8192, 0);
//	if (devBase->dev_Task == NULL) KPrintF("ERROR: Pata.device Task created\n");
//	ReadyTask(devBase->dev_Task, TRUE);
//	WaitSignal(SIGF_SINGLE);

	return devBase;
}

void reinit(pIOStdReq io, pConsoleUnit unit);
#define SysBase devBase->dev_SysBase
static pConBase dev_OpenDev(pConBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags)
{
	//KPrintF("[pataDev] Open Unit: %d\n", unitNum);
    devBase->dev_Device.dev_OpenCnt++;
    devBase->dev_Device.dev_Flags &= ~LIBF_DELEXP;

    if (unitNum >= 0 || unitNum < 1)
    {
		ioreq->io_Error = 0;
		ioreq->io_Unit = (struct  Unit *)&devBase->dev_Unit[unitNum];
		ioreq->io_Device = (struct Device *)devBase;
		devBase->dev_Unit[unitNum].con_Width = 80;
		devBase->dev_Unit[unitNum].con_Height = 25;
		devBase->dev_Unit[unitNum].con_CSRX = 0;
		devBase->dev_Unit[unitNum].con_CSRY = 0;
		devBase->dev_Unit[unitNum].con_CurFG = 7;
		devBase->dev_Unit[unitNum].con_CurBG = 0;
		devBase->dev_Unit[unitNum].con_Cursor = 1;

		reinit(ioreq, &devBase->dev_Unit[unitNum]);
		return devBase;
	} else
		ioreq->io_Error = IOERR_OPENFAIL;
	return devBase;
}

static pConBase dev_CloseDev(pConBase devBase, struct IOStdReq *ioreq)
{
	devBase->dev_Device.dev_OpenCnt--;
	if(!devBase->dev_Device.dev_OpenCnt)
	{
		// Should we "expunge" the device?
	}
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return (devBase);
}

static pConBase dev_ExpungeDev(pConBase devBase)
{
	return (NULL);
}

static pConBase dev_ExtFuncDev(pConBase devBase)
{
	return (NULL);
}

static void dev_BeginIO(pConBase devBase, pIOStdReq io)
{
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > MAX_CMD) cmd = 0; // Invalidate the command.
#if 0
		if ((io->Unit.unit_Flags & DUB_STOPPED) != 0)
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
#endif
//KPrintF("Calling CommandVector with %d\n", cmd);
	con_commandVector[cmd](io);
}

static void dev_AbortIO(pConBase devBase, pIOStdReq io)
{
	con_EndCommand(io, IOERR_ABORTED);
}
