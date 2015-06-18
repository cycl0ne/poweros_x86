/**
 * @file mouse_device.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "mouse_device.h"
#include "residents.h"
#include "asm.h"

#define DEVICE_VERSION_STRING "\0$VER: mouse.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static char DevName[] = "mouse.device";
static char Version[] = "\0$VER: mouse.device 0.1 ("__DATE__")\r\n";

// TEMP FIX GCC
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/*******************

Prototypes

********************/

static void arch_ps2m_init();

static pMouseBase dev_InitDev(pMouseBase devBase, UINT32 *segList, pSysBase execBase);
static pMouseBase dev_OpenDev(pMouseBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags);
static pMouseBase dev_CloseDev(pMouseBase devBase, struct IOStdReq *ioreq);
static pMouseBase dev_ExpungeDev(pMouseBase devBase);
static pMouseBase dev_ExtFuncDev(pMouseBase devBase);
static void dev_BeginIO(pMouseBase devBase, pIOStdReq io);
static void dev_AbortIO(pMouseBase devBase, pIOStdReq io);

void mouse_QueueCommand(struct IOStdReq *io);
void mouse_EndCommand(struct IOStdReq *io, UINT32 error);
extern void (*mouse_CmdVector[])(pIOStdReq);
extern INT8 mouseCmdQuick[];
static __attribute__((no_instrument_function)) BOOL mouse_handler(UINT32 number, MouseBase_t *MDBase, APTR SysBase);

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

static const struct MouseBase DevData =
{
	.dev_Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.dev_Device.dev_Node.ln_Type = NT_DEVICE,
	.dev_Device.dev_Node.ln_Pri = 60,
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
	sizeof(struct MouseBase),
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

#define MOUSE_IRQ 12

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }

/*
 * inportb
 * Read from an I/O port.
 */
static unsigned char inportb(unsigned short _port) 
{
	unsigned char rv;
	asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

/*
 * outportb
 * Write to an I/O port.
 */
static void outportb(unsigned short _port, unsigned char _data) 
{
	asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

/*******************

DEVICE FUNCTIONS

********************/

static pMouseBase dev_InitDev(pMouseBase devBase, UINT32 *segList, pSysBase SysBase)
{
	(void)segList;
	devBase->dev_SysBase = SysBase;

	// Initialise Unit Command Queue
	NewList((struct List *)&devBase->Unit.unit_MsgPort.mp_MsgList);
	devBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)DevName;
	devBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	devBase->Unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling
	devBase->Unit.unit_Flags &= ~DUB_STOPPED;
	devBase->BufHead = devBase->BufTail = 0;

//DISABLED at the moment!
	return devBase;

	arch_ps2m_init();
	
	KPrintF("PS/2 mouse driver installed\n");

	devBase->IS = CreateIntServer((STRPTR)"IRQ12 mouse.device", IS_PRIORITY, mouse_handler, devBase);
	AddIntServer(IRQ_MOUSE, devBase->IS);

//	devBase->dev_BootTask	= FindTask(NULL);
//	devBase->dev_Task = CreateTask("pata.device", (Task_Function)dev_PataTask, devBase, 8192, 0);
//	if (devBase->dev_Task == NULL) KPrintF("ERROR: Pata.device Task created\n");
//	ReadyTask(devBase->dev_Task, TRUE);
//	WaitSignal(SIGF_SINGLE);

	return devBase;
}

#define SysBase devBase->dev_SysBase

static pMouseBase dev_OpenDev(pMouseBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags)
{
	if (unitNum>0) unitNum = 0;
	if (devBase->dev_Device.dev_OpenCnt < 1)
	{
		devBase->dev_Device.dev_OpenCnt++;
		devBase->dev_Device.dev_Flags &= ~LIBF_DELEXP;
		ioreq->io_Flags = flags;
		ioreq->io_Error = 0;
		ioreq->io_Unit = &devBase->Unit;
	} else 
	{
		ioreq->io_Error = IOERR_UNITBUSY;
	}
	return devBase;
}

static pMouseBase dev_CloseDev(pMouseBase devBase, struct IOStdReq *ioreq)
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

static pMouseBase dev_ExpungeDev(pMouseBase devBase)
{
	(void)devBase;
	return (NULL);
}

static pMouseBase dev_ExtFuncDev(pMouseBase devBase)
{
	(void)devBase;
	return (NULL);
}

static void dev_BeginIO(pMouseBase devBase, pIOStdReq io)
{
	(void)devBase;
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > MAX_CMD) cmd = 0; // Invalidate the command.

	if (mouseCmdQuick[cmd] >= 0)
	{
		mouse_QueueCommand((struct IOStdReq*)io);
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
		
		if ((io->io_Unit->unit_Flags & DUB_STOPPED) != 0)
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
	}
//KPrintF("Calling CommandVector with %d\n", cmd);
	mouse_CmdVector[cmd](io);
}

static void dev_AbortIO(pMouseBase devBase, pIOStdReq io)
{
	(void)devBase;
	mouse_EndCommand(io, IOERR_ABORTED);
}

static void mouse_wait(uint8_t a_type) {
	uint32_t timeout = 100000;
	if (!a_type) {
		while (--timeout) {
			if ((inportb(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
				return;
			}
		}
//		debug_print(INFO, "mouse timeout");
		return;
	} else {
		while (--timeout) {
			if (!((inportb(MOUSE_STATUS) & MOUSE_ABIT))) {
				return;
			}
		}
//		debug_print(INFO, "mouse timeout");
		return;
	}
}

static void mouse_write(uint8_t write) {
	mouse_wait(1);
	outportb(MOUSE_STATUS, MOUSE_WRITE);
	mouse_wait(1);
	outportb(MOUSE_PORT, write);
}

static uint8_t mouse_read(void) {
	mouse_wait(0);
	char t = inportb(MOUSE_PORT);
	return t;
}

static void arch_ps2m_init(void)
{
	uint8_t status;
	IRQ_OFF;
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0xA8);
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0x20);
	mouse_wait(0);
	status = inportb(0x60) | 2;
	mouse_wait(1);
	outportb(MOUSE_STATUS, 0x60);
	mouse_wait(1);
	outportb(MOUSE_PORT, status);
	mouse_write(0xF6);
	mouse_read();
	mouse_write(0xF4);
	mouse_read();
	IRQ_RES;
	
	uint8_t tmp = inportb(0x61);
	outportb(0x61, tmp | 0x80);
	outportb(0x61, tmp & 0x7F);
	inportb(MOUSE_PORT);
}

#undef SysBase

#define	LEFT_CLICK   0x01
#define	RIGHT_CLICK  0x02
#define	MIDDLE_CLICK 0x04

#define IECODE_LBUTTON         0x68
#define IECODE_RBUTTON         0x69
#define IECODE_MBUTTON         0x6A
#define IECODE_NOBUTTON        0xFF

static uint8_t mouse_cycle = 0;
static int8_t  mouse_byte[3];

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

static __attribute__((no_instrument_function)) BOOL mouse_handler(UINT32 number, MouseBase_t *MDBase, APTR SysBase)
{
	(void)number;
IRQ_OFF;
	uint8_t status = inportb(MOUSE_STATUS);
	while (status & MOUSE_BBIT) 
	{
		int8_t mouse_in = inportb(MOUSE_PORT);
		KPrintF("mi:%x -", mouse_in);
		if (status & MOUSE_F_BIT) 
		{
			switch (mouse_cycle) 
			{
				case 0:
					mouse_byte[0] = mouse_in;
					KPrintF("/s%d:\n",mouse_in);
					if (!(mouse_in & MOUSE_V_BIT)) return 1;
					++mouse_cycle;
					break;
				case 1:
					mouse_byte[1] = mouse_in;
					KPrintF("/x%d:\n",mouse_in);
					++mouse_cycle;
					break;
				case 2:
					mouse_byte[2] = mouse_in;
					/* We now have a full mouse packet ready to use */
					KPrintF("/y%d:\n",mouse_in);
					if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) 
					{
						/* x/y overflow? bad packet! */
						break;
					}
					mouse_cycle = 0;
					break;
			}
		}
		status = inportb(MOUSE_STATUS);
	}
IRQ_RES;
#if 0		
		if (tail == MDBase->BufHead)
		{
			//test_over++;
			//KPrintF("B!");			//Bufferoverflow
			//if (test_over == 10) {
			//	DPrintF("Bufferoverflow [%x][%x]\n", MDBase->BufTail, MDBase->BufHead);
			//	while(1);
			//}
		} else 
		{
			//test_over--;
			UINT16 qualifier = 0;
			UINT16 buttons = IECODE_NOBUTTON;
			if (mouse_byte[0] & 0x01) {buttons = IECODE_LBUTTON; qualifier |= LEFT_CLICK;}
			if (mouse_byte[0] & 0x02) {buttons = IECODE_RBUTTON; qualifier |= RIGHT_CLICK;}
			if (mouse_byte[0] & 0x03) {buttons = IECODE_MBUTTON; qualifier |= MIDDLE_CLICK;}

			MDBase->BufQueue[tail]	= buttons;
			MDBase->BufQueue[tail+1]= qualifier;
			MDBase->BufQueue[tail+2]= mouse_byte[1];
			MDBase->BufQueue[tail+3]= mouse_byte[2];
			MDBase->BufTail = tail;
//			KPrintF("|%d:%d|", (int8_t)mouse_byte[1], (int8_t)mouse_byte[2]);
			//DPrintF("bh %x, bt %x\n",MDBase->BufHead, MDBase->BufTail);
		}
		mouse_cycle = 0;
		
		if (MDBase->Flags & DUB_IS_SERVICE) 
		{
			//KPrintF("[Service!]");
			KPrintF("[S!]");
			//while(1);
			return 1; // we are in Service
		}
		MDBase->Flags |= DUB_IS_SERVICE;

		if (MDBase->Unit.unit_Flags & DUB_STOPPED) {KPrintF("mouseport.dev stopped\n");MDBase->Flags &= ~DUB_IS_SERVICE;return 0;}
		if (!IsMsgPortEmpty(&MDBase->Unit.unit_MsgPort)) 
		{
			struct IOStdReq *new = (struct IOStdReq *)GetHead(&MDBase->Unit.unit_MsgPort.mp_MsgList);
			mouse_CmdVector[MD_READEVENT](new);
		}
		MDBase->Flags &= ~DUB_IS_SERVICE;
		break;
	default:
		KPrintF("Error in PS2Device\n");
		break;
	}
#endif
	return 1;
}
