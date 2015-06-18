/**
 * @file mouse_device.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "keyboard_device.h"
#include "residents.h"
#include "asm.h"

#define DEVICE_VERSION_STRING "\0$VER: keyboard.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static char DevName[] = "keyboard.device";
static char Version[] = "\0$VER: keyboard.device 0.1 ("__DATE__")\r\n";

#define IRQ_KBD       1
#define KEY_DEVICE    0x60
#define KEY_PENDING   0x64


// TEMP FIX GCC
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

/*******************

Prototypes

********************/

static pKbdBase dev_InitDev(pKbdBase devBase, UINT32 *segList, pSysBase execBase);
static pKbdBase dev_OpenDev(pKbdBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags);
static pKbdBase dev_CloseDev(pKbdBase devBase, struct IOStdReq *ioreq);
static pKbdBase dev_ExpungeDev(pKbdBase devBase);
static pKbdBase dev_ExtFuncDev(pKbdBase devBase);
static void dev_BeginIO(pKbdBase devBase, pIOStdReq io);
static void dev_AbortIO(pKbdBase devBase, pIOStdReq io);

void kbd_QueueCommand(struct IOStdReq *io);
void kbd_EndCommand(struct IOStdReq *io, UINT32 error);
extern void (*kbd_CmdVector[])(pIOStdReq);
extern INT8 kbdCmdQuick[];
static __attribute__((no_instrument_function)) BOOL keyboard_handler(UINT32 number, KbdBase_t *devBase, APTR SysBase);

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

static const struct KbdBase DevData =
{
	.dev_Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.dev_Device.dev_Node.ln_Type = NT_DEVICE,
	.dev_Device.dev_Node.ln_Pri = 65,
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
	sizeof(struct KbdBase),
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
	65,
	DevName,
	Version,
	&InitTab
};

/*******************

DEVICE FUNCTIONS

********************/

static pKbdBase dev_InitDev(pKbdBase devBase, UINT32 *segList, pSysBase SysBase)
{
	(void)segList;
	devBase->dev_SysBase = SysBase;

	// Initialise Unit Command Queue
	NewList((struct List *)&devBase->Unit.unit_MsgPort.mp_MsgList);
	devBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)DevName;
	devBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	devBase->Unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling
	devBase->Unit.unit_Flags &= ~DUB_STOPPED;

	devBase->BufHead= devBase->BufTail = 0;
	NewList(&devBase->HandlerList);
	devBase->KbdInt	= CreateIntServer("IRQ1 Keyboard.device", 100, keyboard_handler, devBase);
	AddIntServer(IRQ_KBD, devBase->KbdInt);
	return devBase;
}

#define  BVBITCLEAR(x, y)  ((y)[(x) / (sizeof(UINT8)*8)] &= ~(1 << ((x) & (sizeof(UINT8)*8 - 1))))
#define  BVBITSET(x, y)    ((y)[(x) / (sizeof(UINT8)*8)] |=  (1 << ((x) & (sizeof(UINT8)*8 - 1))))

#define KEY_UP_MASK   0x80
#define KEY_CODE_MASK 0x7F
#define KEY_CTRL_MASK 0x40

#define KEY_ACTION_DOWN     0x01
#define KEY_ACTION_UP       0x02

#define SET_UNSET(a,b,c) (a) = (c) ? ((a) | (b)) : ((a) & ~(b))

static BOOL ProcessKey(uint8_t c, KbdBase_t *devBase)
{
	APTR SysBase = devBase->dev_SysBase;
	BOOL		down = FALSE;

	if (devBase->E0Key == FALSE)
	{
		// extended Key set, flag it and return for second key
		if (c == 0xE0)
		{
			devBase->E0Key = TRUE;
			return TRUE;
		}
		
		if (c & KEY_UP_MASK)
		{
			c ^= KEY_UP_MASK;
			BVBITCLEAR(c, devBase->Matrix);
		} else
		{
			down = TRUE;
			BVBITSET(c, devBase->Matrix);
		}
		
		switch (c)
		{
			case 0x1D: // LEFT CTRL
				SET_UNSET(devBase->Modifiers, KEY_MOD_LEFT_CTRL, down);
				break;
			case 0x2A: // LEFT SHIFT
				SET_UNSET(devBase->Modifiers, KEY_MOD_LEFT_SHIFT, down);
				break;
			case 0x36: // RIGHT SHIFT
				SET_UNSET(devBase->Modifiers, KEY_MOD_RIGHT_SHIFT, down);
				break;
			case 0x38: // LEFT ALT
				SET_UNSET(devBase->Modifiers, KEY_MOD_LEFT_ALT, down);
			default:
				break;
		}
	} else if (devBase->E0Key == TRUE) // Additional Special Keys
	{
		if (c & KEY_UP_MASK)
		{
			c ^= KEY_UP_MASK;
			BVBITCLEAR(c, devBase->Matrix);
		} else
		{
			down = TRUE;
			BVBITSET(c, devBase->Matrix);
		}
		
		switch (c)
		{
			case 0x5B:
				SET_UNSET(devBase->Modifiers, KEY_MOD_LEFT_SUPER, down);
				break;
			case 0x5C: 
				SET_UNSET(devBase->Modifiers, KEY_MOD_RIGHT_SUPER, down);
				break;
			case 0x1D: 
				SET_UNSET(devBase->Modifiers, KEY_MOD_RIGHT_CTRL, down);
				break;
			case 0x38: 
				SET_UNSET(devBase->Modifiers, KEY_MOD_RIGHT_ALT, down);
				break;
			case 0x48:
				SET_UNSET(devBase->SpecialKey, KEY_ARROW_UP, down);
				break;
			case 0x4D:
				SET_UNSET(devBase->SpecialKey, KEY_ARROW_RIGHT, down);
				break;
			case 0x50:
				SET_UNSET(devBase->SpecialKey, KEY_ARROW_DOWN, down);
				break;
			case 0x4B:
				SET_UNSET(devBase->SpecialKey, KEY_ARROW_LEFT, down);
				break;
			case 0x49:
				SET_UNSET(devBase->SpecialKey, KEY_PAGE_UP, down);
				break;
			case 0x51:
				SET_UNSET(devBase->SpecialKey, KEY_PAGE_DOWN, down);
			default:
				break;
		}
		devBase->E0Key = FALSE;
	}

	uint32_t tail = devBase->BufTail;
	tail++;
	tail &= (KBDBUFSIZE-1);
	
	if (tail == devBase->BufHead)
	{
		KPrintF("Kbd Buffer overflow\n");
		c = 0xff;
		tail--;
		tail &= (KBDBUFSIZE-1);
	}

	devBase->BufQueue[tail] = c;
	devBase->BufTail = tail;
	
	if (devBase->Unit.unit_Flags & DUB_STOPPED) return TRUE;

	if (!IsMsgPortEmpty(&devBase->Unit.unit_MsgPort))
	{
		pIOStdReq io = (pIOStdReq)GetHead(&devBase->Unit.unit_MsgPort.mp_MsgList);
		kbd_CmdVector[KBD_READEVENT](io);
	}
	return TRUE;
}

static __attribute__((no_instrument_function)) BOOL keyboard_handler(uint32_t number, KbdBase_t *devBase, APTR SysBase)
{
	(void) number; // we do not need number here
	(void) SysBase;

	if (!(inb(0x64) & 0x01)) return FALSE; // its not us, return.

	uint8_t scancode;
	while (inb(KEY_PENDING) & 2);
	scancode = inb(KEY_DEVICE);
	ProcessKey(scancode, devBase);
	
//	KPrintF("Char: %x ", scancode);
#if 0
	if (!IsListEmpty(&devBase->HandlerList))
	{
		//Activate Resethandlers
	}
#endif

	return TRUE;
}

#define SysBase devBase->dev_SysBase

static pKbdBase dev_OpenDev(pKbdBase devBase, struct IOStdReq *ioreq, INT32 unitNum, UINT32 flags)
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

static pKbdBase dev_CloseDev(pKbdBase devBase, struct IOStdReq *ioreq)
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

static pKbdBase dev_ExpungeDev(pKbdBase devBase)
{
	(void)devBase;
	return (NULL);
}

static pKbdBase dev_ExtFuncDev(pKbdBase devBase)
{
	(void)devBase;
	return (NULL);
}

static void dev_BeginIO(pKbdBase devBase, pIOStdReq io)
{
	(void)devBase;
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > MAX_CMD) cmd = 0; // Invalidate the command.

	if (kbdCmdQuick[cmd] >= 0)
	{
		kbd_QueueCommand((struct IOStdReq*)io);
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
	kbd_CmdVector[cmd](io);
}

static void dev_AbortIO(pKbdBase devBase, pIOStdReq io)
{
	(void)devBase;
	kbd_EndCommand(io, IOERR_ABORTED);
}

