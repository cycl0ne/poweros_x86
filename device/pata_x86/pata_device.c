/**
 * @file pata_device.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "pata_private.h"
#include "ports.h"
#include "residents.h"
#include "ata.h"

#include "hw.h"

#define DEVICE_VERSION_STRING "\0$VER: pata.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static char DevName[] = "pata.device";
static char Version[] = "\0$VER: pata.device 0.1 ("__DATE__")\r\n";

/*******************

Prototypes

********************/

static pPataBase dev_InitDev(pPataBase devBase, UINT32 *segList, pSysBase execBase);
static pPataBase dev_OpenDev(pPataBase devBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags);
static pPataBase dev_CloseDev(pPataBase devBase, struct IOStdReq *ioreq);
static pPataBase dev_ExpungeDev(pPataBase devBase);
static pPataBase dev_ExtFuncDev(pPataBase devBase);
static void dev_BeginIO(pPataBase devBase, pIOStdReq io);
static void dev_AbortIO(pPataBase devBase, pIOStdReq io);

extern void (*pata_commandVector[])(pIOStdReq);
extern INT8 pata_commandQuick[];

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

static const struct PataBase DevData =
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
	.dev_Unit[PRIMARY_MASTER].pu_ioBase	= 0x1F0,
	.dev_Unit[PRIMARY_MASTER].pu_Control= 0x3f6,
	.dev_Unit[PRIMARY_MASTER].pu_Slave	= 0,
	.dev_Unit[PRIMARY_SLAVE].pu_ioBase	= 0x1F0,
	.dev_Unit[PRIMARY_SLAVE].pu_Control	= 0x3f6,
	.dev_Unit[PRIMARY_SLAVE].pu_Slave	= 1,

	.dev_Unit[SECONDARY_MASTER].pu_ioBase	= 0x170,
	.dev_Unit[SECONDARY_MASTER].pu_Control	= 0x376,
	.dev_Unit[SECONDARY_MASTER].pu_Slave	= 0,
	.dev_Unit[SECONDARY_SLAVE].pu_ioBase	= 0x170,
	.dev_Unit[SECONDARY_SLAVE].pu_Control	= 0x376,
	.dev_Unit[SECONDARY_SLAVE].pu_Slave		= 1,
};

/*******************

ROMTAG RESIDENT

********************/

struct InitTable
{
	UINT32	LibBaseSize;
	APTR	volatile FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} InitTab =
{
	sizeof(struct PataBase),
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
#define SysBase devBase->dev_SysBase

static void ata_device_init(pPataBase devBase, pPataUnit unit)
{
	//KPrintF("Initializing IDE device on bus %d\n", unit->pu_ioBase);

	outportb(unit->pu_ioBase + 1, 1);
	outportb(unit->pu_Control, 0);

	outportb(unit->pu_ioBase + ATA_REG_HDDEVSEL, 0xA0 | unit->pu_Slave << 4);
	ata_io_wait(unit);

	outportb(unit->pu_ioBase + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	ata_io_wait(unit);

	int status = inportb(unit->pu_ioBase + ATA_REG_COMMAND);
	//KPrintF("Device status: %d\n", status);

	ata_wait(unit, 0);

	UINT16 * buf = (UINT16 *)&unit->pu_Identity;

	for (int i = 0; i < 256; ++i) {
		buf[i] = inports(unit->pu_ioBase);
	}

	UINT8 * ptr = (UINT8 *)&unit->pu_Identity.model;
	for (int i = 0; i < 39; i+=2) {
		UINT8 tmp = ptr[i+1];
		ptr[i+1] = ptr[i];
		ptr[i] = tmp;
	}

	//KPrintF("Device Name:  %s\n", unit->pu_Identity.model);
	//KPrintF("Sectors (48): %d\n", (UINT32)unit->pu_Identity.sectors_48);
	//KPrintF("Sectors (24): %d size(%x)\n", unit->pu_Identity.sectors_28, unit->pu_Identity.sectors_28*512);

	outportb(unit->pu_ioBase + ATA_REG_CONTROL, 0x02);
}

static int ata_device_detect(pPataBase devBase, pPataUnit unit)
{

	//KPrintF("ioBase: %x, Control: %x, slave: %x\n",unit->pu_ioBase,unit->pu_Control,unit->pu_Slave);
	ata_soft_reset(unit);
	outportb(unit->pu_ioBase + ATA_REG_HDDEVSEL, 0xa | unit->pu_Slave <<4);
	ata_io_wait(unit);

	unsigned char cl = inportb(unit->pu_ioBase + ATA_REG_LBA1);
	unsigned char ch = inportb(unit->pu_ioBase + ATA_REG_LBA2);

	//KPrintF("[PATA] Device detected: %2x %2x\n", cl, ch);

	if (cl == 0xFF && ch == 0xFF) return 0;
	if (cl == 0x00 && ch == 0x00)
	{
		ata_device_init(devBase, unit);
	}
	return 1;
}

#undef SysBase

static UINT32 dev_PataTask(APTR SysBase, pPataBase devBase);

static pPataBase dev_InitDev(pPataBase devBase, UINT32 *segList, pSysBase SysBase)
{
	devBase->dev_SysBase = SysBase;
	NewList((pList)&devBase->dev_InOutQueue);

	devBase->dev_Status[0]	= ata_device_detect(devBase, &devBase->dev_Unit[PRIMARY_MASTER]);
	devBase->dev_Status[1]	= ata_device_detect(devBase, &devBase->dev_Unit[PRIMARY_SLAVE]);
	devBase->dev_Status[2]	= ata_device_detect(devBase, &devBase->dev_Unit[SECONDARY_MASTER]);
	devBase->dev_Status[3]	= ata_device_detect(devBase, &devBase->dev_Unit[SECONDARY_SLAVE]);

	devBase->dev_BootTask	= FindTask(NULL);
	devBase->dev_Task = CreateTask("pata.device", (Task_Function)dev_PataTask, devBase, 8192, 0);
	if (devBase->dev_Task == NULL) KPrintF("ERROR: Pata.device Task created\n");
	ReadyTask(devBase->dev_Task, TRUE);
	WaitSignal(SIGF_SINGLE);

	return devBase;
}

#define SysBase devBase->dev_SysBase
static pPataBase dev_OpenDev(pPataBase devBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags)
{
	//KPrintF("[pataDev] Open Unit: %d\n", unitNum);
    devBase->dev_Device.dev_OpenCnt++;
    devBase->dev_Device.dev_Flags &= ~LIBF_DELEXP;

    if (unitNum >= 0 || unitNum < MAX_BUS)
    {
		if (devBase->dev_Status[unitNum])
		{
			ioreq->io_Error = 0;
			ioreq->io_Unit = (struct  Unit *)&devBase->dev_Unit[unitNum];
			ioreq->io_Device = (struct Device *)devBase;
			return devBase;
		} else
			ioreq->io_Error = IOERR_OPENFAIL;
	} else
		ioreq->io_Error = IOERR_OPENFAIL;
	return devBase;
}

static pPataBase dev_CloseDev(pPataBase devBase, struct IOStdReq *ioreq)
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

static pPataBase dev_ExpungeDev(pPataBase devBase)
{
	return (NULL);
}

static pPataBase dev_ExtFuncDev(pPataBase devBase)
{
	return (NULL);
}

static void dev_BeginIO(pPataBase devBase, pIOStdReq io)
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
	pata_commandVector[cmd](io);
}

static void dev_AbortIO(pPataBase devBase, pIOStdReq io)
{
	pata_EndCommand(io, IOERR_ABORTED);
}



