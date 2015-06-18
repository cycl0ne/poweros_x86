/**
 * @file serialdevice.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "serial_private.h"
#include "residents.h"
#include "devicehelper.h"

#include "hw_write.h"

#define SysBase SerBase->ser_SysBase

void (*commandVector[])(pSerialBase, struct IOStdReq *);

//CMD_INVALID -1
static void Invalid(pSerialBase SerBase, struct IOStdReq *io)
{
	EndCommandQueue(IOERR_NOCMD, io, SysBase);	
}

//CMD_STOP -1
static void Stop(pSerialBase SerBase, struct IOStdReq *io)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	EndCommandQueue(0, io, SysBase);
}

//CMD_START -1
static void Start(pSerialBase SerBase, struct IOStdReq *io)
{
	pSerUnit unit = (pSerUnit)io->io_Unit;
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)GetHead(&unit->su_InputQueue.mp_MsgList);
	if (new != NULL)
	{
		commandVector[new->io_Command](SerBase, new);		
	} else
	{
		new = (struct IOStdReq *)GetHead(&unit->su_OutputQueue.mp_MsgList);;
		if (new) commandVector[new->io_Command](SerBase, new);
	}
	EndCommandQueue(0, io, SysBase);
}

//CMD_FLUSH -1
static void Flush(pSerialBase SerBase, struct IOStdReq *io)
{
	pSerUnit unit = (pSerUnit)io->io_Unit;
	
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&unit->su_InputQueue.mp_MsgList, node, nextnode)
	{
		EndCommandQueue(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}
	
	ForeachNodeSafe(&unit->su_OutputQueue.mp_MsgList, node, nextnode)
	{
		EndCommandQueue(IOERR_ABORTED, (struct IOStdReq *)node, SysBase);
	}

	EndCommandQueue(0, io, SysBase);
}

//CMD_RESET -1
static void Reset(pSerialBase SerBase, struct IOStdReq *io)
{
	Stop(SerBase, io);
	Flush(SerBase, io);
	Start(SerBase, io);
}

//CMD_CLEAR -1
static void Clear(pSerialBase SerBase, struct IOStdReq *io)
{
	pSerUnit unit = (pSerUnit)io->io_Unit;

	UINT32 ipl = Disable();
	unit->su_icount = unit->su_istart = 0;
	Restore(ipl);
	EndCommandQueue(0, io, SysBase);
}

//CMD_READ 0
static void Read(pSerialBase SerBase, struct IOStdReq *io)
{
	pIOStdReq	ioloop = io;
	//KPrintF("Serial Read\n");

	if (ioloop->io_Unit == NULL)
	{
		EndCommandQueue(IOERR_BADADDRESS, io, SysBase);
		return;
	}

	pSerUnit unit = (pSerUnit)ioloop->io_Unit;
	IRQMask im = Disable();

	if (TEST_BITS(ioloop->io_Flags, IOF_SERVICING)) 
	{
		//KPrintF("READ: SER_IOF_Servicing: %x\n", ioloop); //ioloop->io_Message.mn_Node.ln_Pad);
		Restore(im);
		return;
	}
	
	ioloop->io_Flags |= IOF_SERVICING;
	
	UINT32	count;
	UINT32	len = ioloop->io_Length;
	UINT8	c;
	void 	*buf = ioloop->io_Data;

	// Block if no chars are waiting or not enough chars
	if (unit->su_icount == 0 || len > unit->su_icount)
	{
		//KPrintF("Buffer Empty\n");
		ioloop->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
		Restore(im);
		return;
	}

	for (count = 0; count < len; count++)
	{
		if (unit->su_icount == 0) break;
		
		c = unit->su_in[unit->su_istart];
		((UINT8 *)buf)[count] = c;
		unit->su_icount--;
		unit->su_istart = (unit->su_istart + 1) % UART_IBLEN;			
	}
	Restore(im);
	ioloop->io_Actual = count;
	//KPrintF("EndCommand\n");
	EndCommandQueue(0, ioloop, SysBase);
}

//CMD_WRITE 0
static void Write(pSerialBase SerBase, struct IOStdReq *io)
{
	pIOStdReq	ioloop = io;
//KPrintF("Serial Write\n");
	if (ioloop->io_Unit == NULL)
	{
		EndCommandQueue(IOERR_BADADDRESS, io, SysBase);
		return;
	}

	pSerUnit unit = (pSerUnit)ioloop->io_Unit;
	IRQMask im = Disable();

	if (TEST_BITS(ioloop->io_Flags, IOF_SERVICING)) 
	{
		//KPrintF("WRITE: SER_IOF_Servicing: %x\n", ioloop); //ioloop->io_Message.mn_Node.ln_Pad);
		Restore(im);
		return;
	}

	ioloop->io_Flags |= IOF_SERVICING;

//	Restore(im);
	
	UINT32	count;
	UINT32	len = ioloop->io_Length;
	void 	*buf = ioloop->io_Data;

	// Block if our buffer is full
	if (unit->su_ocount == UART_OBLEN)
	{
		KPrintF("Buffer full (output)\n");
		ioloop->io_Flags &= ~(IOF_SERVICING|IOF_QUICK);
		Restore(im);
		return;
	}
//	im = Disable();
	for (count = 0; count < len; count++)
	{
		UINT8	ch = ((const UINT8 *)buf)[count];

		if (unit->su_oidle)
		{
			_HWPutC(unit->su_csr, ch);
			unit->su_oidle = FALSE;
			unit->su_cout++;
		} else
		{
			if (unit->su_ocount == UART_OBLEN) break;
			unit->su_out[(unit->su_ostart + unit->su_ocount) % UART_OBLEN] = ch;
			unit->su_ocount++;
		}
	}
	Restore(im);
//	KPrintF("serial count: %d\n", count);
	ioloop->io_Actual = count;
//	KPrintF("EndCQ: Write\n");
	EndCommandQueue(0, ioloop, SysBase);
}

#if 0
static void SetIFlg(pSerialBase SerBase, struct IOStdReq *io)
{
	pSerUnit	unit = (pSerUnit)io->io_Unit;
	UINT8		old = unit->su_iflags;
	EndCommandQueue(0, io, SysBase);		
}
#endif


#if 0
#define UART_CTRL_SET_IFLAG   (CMD_NONSTD+0) /**< set input flags               */
#define UART_CTRL_CLR_IFLAG   (CMD_NONSTD+1) /**< clear input flags             */
#define UART_CTRL_GET_IFLAG   (CMD_NONSTD+2) /**< get input flags               */
#define UART_CTRL_SET_OFLAG   (CMD_NONSTD+3) /**< set output flags              */
#define UART_CTRL_CLR_OFLAG   (CMD_NONSTD+4) /**< clear output flags            */
#define UART_CTRL_GET_OFLAG   (CMD_NONSTD+5) /**< get output flags              */
#define UART_CTRL_OUTPUT_IDLE (CMD_NONSTD+6) /**< determine if transmit idle    */
#endif

void (*commandVector[])(pSerialBase, struct IOStdReq *) = {
	Invalid, Reset, Read, Write, Invalid, 
	Clear, Stop, Start, Flush
#if 0
	,SetIFlg, ClearIFlg, GetIFlg,
	SetOFlg, ClearOFlg, GetOFlg,
	OIdle
#endif
//	KDInvalid, KDReset, KDInvalid /*READ*/, KDInvalid /*WRITE*/, KDInvalid /* Update */,
//	KDClear, KDStop, KDStart, KDFlush, KDAddResetHandler, KDRemResetHandler, KDResetHandlerDone, KDReadMatrix, KDReadEvent
};

// All Commands are Quick, except ReadEvent
/*
#define CMD_INVALID	0
#define CMD_RESET	1
#define CMD_READ	2
#define CMD_WRITE	3
#define CMD_UPDATE	4
#define CMD_CLEAR	5
#define CMD_STOP	6
#define CMD_START	7
#define CMD_FLUSH	8
#define CMD_NONSTD	9
*/

INT8 commandQuick[] =
{
	-1 //Invalid
	,-1 //RESET
	, 0 //READ
	, 0 //WRITE
	,-1 //UPDATE
	,-1 //CLEAR
	,-1 //STOP
	,-1 //START
	,-1 //FLUSH
#if 0
	-1, //
	-1, 
	-1, 
	 0
#endif
};

