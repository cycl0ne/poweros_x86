#include "kbddev.h"
#include "timer.h"
#include "resident.h"
#include "inputevent.h"
#include "exec_funcs.h"

#define IRQ_CLK       0
#define IRQ_KBD       1
#define IRQ_PIC1      2
#define IRQ_PIC_SPUR  7
#define IRQ_MOUSE     12
#define IRQ_NE2000    5

#define DEVICE_VERSION_STRING "\0$VER: keyboard.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

#define TDEV_UNIT_MAX	1
#define _TDTASK_STACK_ 0x4000
#define DUB_STOPPED (1<<0)

static const char name[] = "keyboard.device";
static const char version[] = DEVICE_VERSION_STRING
static const char EndResident;

APTR kdev_OpenDev(struct KbdBase *KbdBase, struct IORequest *ioreq, UINT32 unitnum, UINT32 flags);
APTR kdev_CloseDev(struct KbdBase *KbdBase, struct IORequest *ioreq);
APTR kdev_ExpungeDev(struct KbdBase *KbdBase);
APTR kdev_ExtFuncDev(struct KbdBase *KbdBase);
void kdev_BeginIO(KbdBase *KbdBase, struct IORequest *ioreq);
void kdev_AbortIO(KbdBase *kdev_, struct IORequest *ioreq);
static struct KbdBase *kdev_Init(struct KbdBase *KbdBase, UINT32 *segList, struct SysBase *SysBase);

static APTR FuncTab[] =
{
	(void(*)) kdev_OpenDev,
	(void(*)) kdev_CloseDev,
	(void(*)) kdev_ExpungeDev,
	(void(*)) kdev_ExtFuncDev,

	(void(*)) kdev_BeginIO,
	(void(*)) kdev_AbortIO,
	(APTR) ((UINT32)-1)
};

static const APTR InitTab[4]=
{
	(APTR)sizeof(struct KbdBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)kdev_Init
};

static const struct Resident ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT | RTF_COLDSTART,
	DEVICE_VERSION,
	NT_DEVICE,
	45,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

#include "kbd_io.h"

#define KEY_DEVICE    0x60
#define KEY_PENDING   0x64
#include "_debug.h"

#define KD_RESET1	(1<<0)
#define KD_RESET2	(1<<1)

#define  BVBITCLEAR(x, y)  ((y)[(x) / (sizeof(UINT8)*8)] &= ~(1 << ((x) & (sizeof(UINT8)*8 - 1))))
#define  BVBITSET(x, y)    ((y)[(x) / (sizeof(UINT8)*8)] |=  (1 << ((x) & (sizeof(UINT8)*8 - 1))))
extern void (*commandVector[])(struct IOStdReq *, KbdBase *);


static void ProcessKey(UINT16 scancode, KbdBase *KbdBase)
{
	SysBase *SysBase = KbdBase->SysBase;
	UINT16 key = scancode & 0x007f;
	UINT16 qualifier = 0;
	if (key < HIGH_KEYCODE)
	{
		// Put it into the Matrix
		if (scancode & 0x80)	BVBITCLEAR(key, KbdBase->Matrix);
		else 					BVBITSET(key, KbdBase->Matrix);
		// check for Shift/Ctrl/Alt
		switch(scancode)
		{
			case LSHIFT:
				KbdBase->Shifts |= IEQUALIFIER_LSHIFT;
				break;
			case RSHIFT:
				KbdBase->Shifts |= IEQUALIFIER_RSHIFT;
				break;
			case ALT:
				KbdBase->Shifts |= IEQUALIFIER_LALT;
				break;
			case CONTROL:
				KbdBase->Shifts |= IEQUALIFIER_CONTROL;
				break;
			case LSHIFT+0x80:
				KbdBase->Shifts &= ~IEQUALIFIER_LSHIFT;
				break;
			case RSHIFT+0x80:
				KbdBase->Shifts &= ~IEQUALIFIER_RSHIFT;
				break;
			case ALT+0x80:
				KbdBase->Shifts &= ~IEQUALIFIER_LALT;
				break;
			case CONTROL+0x80:
				KbdBase->Shifts &= ~IEQUALIFIER_CONTROL;
				break;
		}
		// Check for Numeric
		// Numeric can have the follwing values:
		// 47, 48, 49, 4a, 4b, 4c, 4d, 4e, 4f, 50, 51, 52, 53,
		if ((key > 0x47 && key < 0x53) &&
			(key > 0x47+0x80 && key < 0x53+0x80))
		{
			qualifier |= IEQUALIFIER_NUMERICPAD;
		}

//		DPrintF("Writing Buffer key: %x qual: %x\n", key, qualifier);

		UINT32 tail = KbdBase->BufTail;
		tail += 2;
		tail &= (KBBUFSIZE-1);
		if (tail == KbdBase->BufHead)
		{
			DPrintF("Keyboard Overflow\n");
			// Keyboard overflow
			key = OVERFLOW_CODE;
			tail -=2;
			tail &= (KBBUFSIZE-1);
		}

		KbdBase->BufQueue[tail] = key;
		qualifier |= KbdBase->Shifts;
		//DPrintF("Writing Buffer key: %x qual: %x (tail: %x)\n", key, qualifier, tail);
		KbdBase->BufQueue[tail+1] = qualifier;
		KbdBase->BufTail = tail; // Write Value back


		//DPrintF("BufferHead: %d BufferTail: %d\n", KbdBase->BufHead, KbdBase->BufTail);
		//DPrintF("Buffer: %x  %x\n", KbdBase->BufQueue[KbdBase->BufTail], KbdBase->BufQueue[KbdBase->BufTail+1]);

		if (KbdBase->Unit.unit_Flags & DUB_STOPPED) return;

		//DPrintF("commandVector\n");

		if (!IsMsgPortEmpty(&KbdBase->Unit.unit_MsgPort)) {
			struct IOStdReq *new = (struct IOStdReq *)GetHead(&KbdBase->Unit.unit_MsgPort.mp_MsgList);
			commandVector[KBD_READEVENT](new, KbdBase);
		}
	} else
	{
		DPrintF("Keycode value too high. Is %d. Should be < %d\n", key, HIGH_KEYCODE);
	}
}

static UINT32 keyboard_handler(unsigned int exc_no, APTR Data, SysBase *SysBase)
{
	// Wakeup from Keyboard?
	if (!(inb(0x64) & 0x01)) return FALSE;
	KbdBase *KbdBase = Data;
	unsigned char scancode;
	while(inb(KEY_PENDING) & 2);
	scancode = inb(KEY_DEVICE);
	ProcessKey(scancode, KbdBase);
	//
	// Reset CAUSE Handler *Not Implemented*

	if (!IsListEmpty(&KbdBase->HandlerList))
	{
		struct Node *resetHandler;
		ForeachNode(&KbdBase->HandlerList, resetHandler)
		{
			KbdBase->OutstandingResetHandlers++;
			//Cause(resetHandler);
		}
	}

	//monitor_write_hex((UINT32)SysBase);
	//monitor_write_hex((UINT32)Data);
	return 1;
}

static struct KbdBase *kdev_Init(struct KbdBase *KbdBase, UINT32 *segList, struct SysBase *SysBase)
{
	DPrintF("Keyboard Init...\n");

	KbdBase->Device.dd_Library.lib_OpenCnt = 0;
	KbdBase->Device.dd_Library.lib_Node.ln_Pri = 0;
	KbdBase->Device.dd_Library.lib_Node.ln_Type = NT_DEVICE;
	KbdBase->Device.dd_Library.lib_Node.ln_Name = (STRPTR)name;
	KbdBase->Device.dd_Library.lib_Version = DEVICE_VERSION;
	KbdBase->Device.dd_Library.lib_Revision = DEVICE_REVISION;
	KbdBase->Device.dd_Library.lib_IDString = (STRPTR)&version[7];

	KbdBase->SysBase	= SysBase;
	KbdBase->Flags		= 0;

	// Initialise Unit Command Queue
	NewList((struct List *)&KbdBase->Unit.unit_MsgPort.mp_MsgList);
	KbdBase->Unit.unit_MsgPort.mp_Node.ln_Name = (STRPTR)name;
	KbdBase->Unit.unit_MsgPort.mp_Node.ln_Type = NT_MSGPORT;
	KbdBase->Unit.unit_MsgPort.mp_SigTask = NULL; // Important for our Queue Handling

	// Initialise the reset handler list
	NewList((struct List *)&KbdBase->HandlerList);

	KbdBase->KbdInt = CreateIntServer("IRQ1 Keyboard", 100, keyboard_handler, KbdBase);
	AddIntServer(IRQ_KBD, KbdBase->KbdInt);

	KbdBase->BufHead = KbdBase->BufTail = 0;
	return KbdBase;
}


static const char EndResident = 0;
