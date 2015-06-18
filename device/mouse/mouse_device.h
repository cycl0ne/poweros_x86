/**
 * @file console_device.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "exec_interface.h"

#define	 MD_READEVENT	   (CMD_NONSTD+0)
#define	 MD_ASKCTYPE	   (CMD_NONSTD+1)
#define	 MD_SETCTYPE	   (CMD_NONSTD+2)
#define	 MD_ASKTRIGGER	   (CMD_NONSTD+3)
#define	 MD_SETTRIGGER	   (CMD_NONSTD+4)

#define MAX_CMD				MD_SETTRIGGER

#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)


#define IRQ_MOUSE		12
#define UBF_INITIALIZE	1

#define IS_PRIORITY		0
#define UTF_KEYDOWNS	0
#define UTF_KEYUPS		1


// Buffer for 64 Mousecycles (4 things buffered * 64 = 256)
#define MDBUFSIZE		256

typedef struct MouseTrigger {
   UINT16 Keys;	   /* key transition triggers */
   UINT16 Timeout;	   /* time trigger (vertical blank units) */
   UINT16 XDelta;	   /* X distance trigger */
   UINT16 YDelta;	   /* Y distance trigger */
} MouseTrigger;

typedef struct MouseBase
{
	Device		dev_Device;
	pSysBase	dev_SysBase;

	struct Unit			Unit;
	UINT16				OpenCnt;
	UINT16				BufHead;
	UINT16				BufTail;
	UINT16				BufQueue[MDBUFSIZE];
	MouseTrigger		Timeout;
	MouseTrigger		AccumTimeout;
	UINT16				LastSave;
	UINT16				CurrSave;
	UINT8				Type;
	
	struct Interrupt	*IS;  // Interrupt Code
	UINT8				MouseCycle;
	UINT32				Flags;
	
	pTask		dev_BootTask;
	pTask		dev_Task;
	pMsgPort	dev_Port;
//	ConsoleUnit	dev_Unit[1];
} MouseBase_t, *pMouseBase;


#include "timer.h"

struct InputEvent
{
    struct InputEvent * ie_NextEvent;

    UINT8 ie_Class;     /* see below for definitions */
    UINT8 ie_SubClass;  /* see below for definitions */
    UINT16 ie_Code;      /* see below for definitions */
    UINT16 ie_Qualifier; /* see below for definitions */

    union
    {
        struct
        {
            INT16 ie_x;
            INT16 ie_y;
        } ie_xy;

        APTR ie_addr;

        struct
        {
            UINT8 ie_prev1DownCode;
            UINT8 ie_prev1DownQual;
            UINT8 ie_prev2DownCode;
            UINT8 ie_prev2DownQual;
        } ie_dead;
    } ie_position;

    struct TimeVal      ie_TimeStamp;
};
#define ie_X             ie_position.ie_xy.ie_x
#define ie_Y             ie_position.ie_xy.ie_y
#define ie_EventAddress  ie_position.ie_addr
#define ie_Prev1DownCode ie_position.ie_dead.ie_prev1DownCode
#define ie_Prev1DownQual ie_position.ie_dead.ie_prev1DownQual
#define ie_Prev2DownCode ie_position.ie_dead.ie_prev2DownCode
#define ie_Prev2DownQual ie_position.ie_dead.ie_prev2DownQual

/* InputEvent Classes */
#define IECLASS_NULL           0
#define IECLASS_RAWKEY         1
#define IECLASS_RAWMOUSE       2
#define IECLASS_EVENT          3
#define IECLASS_POINTERPOS     4
#define IECLASS_TIMER          6
#define IECLASS_GADGETDOWN     7
#define IECLASS_GADGETUP       8
#define IECLASS_REQUESTER      9
#define IECLASS_MENULIST       10
#define IECLASS_CLOSEWINDOW    11
#define IECLASS_SIZEWINDOW     12
#define IECLASS_REFRESHWINDOW  13
#define IECLASS_NEWPREFS       14
#define IECLASS_DISKREMOVED    15
#define IECLASS_DISKINSERTED   16
#define IECLASS_ACTIVEWINDOW   17
#define IECLASS_INACTIVEWINDOW 18
#define IECLASS_NEWPOINTERPOS  19 /* (IEPointerPixel *) */
#define IECLASS_MENUHELP       20
#define IECLASS_CHANGEWINDOW   21
