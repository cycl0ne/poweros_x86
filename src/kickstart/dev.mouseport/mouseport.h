#ifndef mouseport_h
#define mouseport_h

#include "types.h"
#include "sysbase.h"
#include "io.h"

void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

#define	 MD_READEVENT	   (CMD_NONSTD+0)
#define	 MD_ASKCTYPE	   (CMD_NONSTD+1)
#define	 MD_SETCTYPE	   (CMD_NONSTD+2)
#define	 MD_ASKTRIGGER	   (CMD_NONSTD+3)
#define	 MD_SETTRIGGER	   (CMD_NONSTD+4)

#define	LEFT_CLICK   0x01
#define	RIGHT_CLICK  0x02
#define	MIDDLE_CLICK 0x04

#define IRQ_MOUSE		12
#define UBF_INITIALIZE	1

#if 0
#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08
#endif

#define IS_PRIORITY		0
#define UTF_KEYDOWNS	0
#define UTF_KEYUPS		1
// Buffer for 64 Mousecycles (4 things buffered * 64 = 256)
#define MDBUFSIZE		256

/* gpt_Keys */
#define	 GPTB_DOWNKEYS	   0
#define	 GPTF_DOWNKEYS	   (1<<0)
#define	 GPTB_UPKEYS	   1
#define	 GPTF_UPKEYS	   (1<<1)

#define	 GPCT_ALLOCATED	   -1	 /* allocated by another user */
#define	 GPCT_NOCONTROLLER 0
#define	 GPCT_MOUSE	   1
#define	 GPCT_RELJOYSTICK  2
#define	 GPCT_ABSJOYSTICK  3
/****** Errors ******/
#define	 GPDERR_SETCTYPE   1	 /* this controller not valid at this time */

#define IOF_QUEUED (1<<4)
#define IOF_CURRENT (1<<5)
#define IOF_SERVICING (1<<6)
#define IOF_DONE (1<<7)

#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)

typedef struct MouseTrigger {
   UINT16 Keys;	   /* key transition triggers */
   UINT16 Timeout;	   /* time trigger (vertical blank units) */
   UINT16 XDelta;	   /* X distance trigger */
   UINT16 YDelta;	   /* Y distance trigger */
} MouseTrigger;


typedef struct MDBase {
	struct Device		Device;
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
	SysBase				*SysBase;
} MDBase;

extern void (*mouseCmdVector[])(struct IOStdReq *, MDBase *);
extern INT8 mouseCmdQuick[];
#endif
