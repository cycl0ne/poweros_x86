/**
 * @file console_device.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "exec_interface.h"

#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)

#define MAX_CMD CMD_NONSTD

#define HIGH_KEYCODE	0x7F
#define RESET_CODE		0x78
#define OVERFLOW_CODE	0x7D
#define MATRIX_BYTES	(HIGH_KEYCODE+8)/8
#define KBDBUFSIZE		32
#define IKC_SIZE		10

#define KBD_ADDRESETHANDLER		9
#define KBD_REMRESETHANDLER		10
#define KBD_RESETHANDLERDONE	11
#define KBD_READMATRIX			12
#define KBD_READEVENT			13

#define KEY_MOD_LEFT_CTRL   0x01
#define KEY_MOD_LEFT_SHIFT  0x02
#define KEY_MOD_LEFT_ALT    0x04
#define KEY_MOD_LEFT_SUPER  0x08
#define KEY_MOD_RIGHT_CTRL  0x10
#define KEY_MOD_RIGHT_SHIFT 0x20
#define KEY_MOD_RIGHT_ALT   0x40
#define KEY_MOD_RIGHT_SUPER 0x80

#define KEY_ARROW_UP	0x01
#define KEY_ARROW_DOWN	0x02
#define KEY_ARROW_LEFT	0x04
#define KEY_ARROW_RIGHT	0x08
#define KEY_PAGE_UP		0x10
#define KEY_PAGE_DOWN	0x20

typedef struct KbdBase
{
	Device		dev_Device;
	pSysBase	dev_SysBase;

	struct Unit	Unit;

	pInterrupt	KbdInt;
	
	uint32_t	BufHead;
	uint32_t	BufTail;
	uint8_t		BufQueue[KBDBUFSIZE];

	List		HandlerList;
	uint16_t	OutstandingResetHandlers;
	
	BOOL		E0Key;

	uint32_t	Modifiers;	
	uint32_t	SpecialKey;
//	uint32_t	Flags;
//	uint32_t	Shifts;
	uint8_t		Matrix[MATRIX_BYTES];

//	pTask		dev_BootTask;
//	pTask		dev_Task;
//	pMsgPort	dev_Port;
//	ConsoleUnit	dev_Unit[1];
} KbdBase_t, *pKbdBase;

