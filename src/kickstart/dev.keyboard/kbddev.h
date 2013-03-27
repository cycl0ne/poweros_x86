#ifndef kbddev_h
#define kbddev_h

#include "types.h"
#include "device.h"
#include "io.h"
#include "timer.h"
#include "sysbase.h"

#include "keyboard.h"

#define HIGH_KEYCODE	0x71
#define RESET_CODE		0x78
#define OVERFLOW_CODE	0x7D
#define MATRIX_BYTES	(HIGH_KEYCODE+8)/8
#define KBBUFSIZE		128
#define IKC_SIZE		10

typedef struct KbdBase
{
	struct Device		Device;
	SysBase				*SysBase;
	struct Unit			Unit;
	struct Interrupt	*KbdInt;

	UINT16				BufHead;
	UINT16				BufTail;
	UINT16  			BufQueue[KBBUFSIZE*2];
	
	struct List			HandlerList; // ResetHandler
	UINT16				OutstandingResetHandlers;
	UINT8				Flags;
	UINT8				Shifts;
	UINT8  				Matrix[MATRIX_BYTES];
} KbdBase;


#endif
