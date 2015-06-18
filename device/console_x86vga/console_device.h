/**
 * @file console_device.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "exec_interface.h"

#include "console.h"

#define MAX_CMD				CMD_FLUSH
#define char_width	1
#define char_height	1

typedef struct ConsoleUnit 
{
	Unit			con_Unit;
	UINT16			con_Width;
	UINT16			con_Height;
	UINT16			con_CSRX;
	UINT16			con_CSRY;
	term_cell_t		*con_Buffer;
	UINT32			con_CurFG;
	UINT32			con_CurBG;
	BOOL			con_Cursor;
	UINT8			con_HoldOut;
	term_state_t	*con_AnsiState;
	UINT32			con_TimerTick;
	pSysBase		con_SysBase;
	pUtilBase		con_UtilBase;
} ConsoleUnit, *pConsoleUnit;

typedef struct ConBase
{
	Device		dev_Device;
	pSysBase	dev_SysBase;
	
	pTask		dev_BootTask;
	pTask		dev_Task;
	pMsgPort	dev_Port;
	ConsoleUnit	dev_Unit[1];
} ConBase, *pConBase;


