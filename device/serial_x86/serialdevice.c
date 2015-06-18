/**
 * @file serialdevice.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "serial_private.h"
#include "residents.h"
#include "devicehelper.h"

#include "hw_init.h"

#define SysBase SerBase->ser_SysBase

#define DEVICE_VERSION_STRING "\0$VER: serial.device 0.1 ("__DATE__")\r\n";
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1


static char DevName[] = "serial.device";
static char Version[] = "\0$VER: serial.device 0.1 ("__DATE__")\r\n";

extern void (*commandVector[])(pSerialBase, pIOStdReq);
extern INT8 commandQuick[];

pSerialBase ser_InitDev(pSerialBase SerBase, UINT32 *segList, pSysBase execBase)
{
	SerBase->ser_SysBase = execBase;

	for(int i =0; i<2; i++)
	{
		pSerUnit	unit = &SerBase->ser_Unit[i];
		//CreateMsgPort(&unit->su_Unit.unit_MsgPort);
		unit->su_InputQueue.mp_SigTask	= NULL;
		unit->su_InputQueue.mp_Flags	= PA_IGNORE;
		unit->su_InputQueue.mp_Node.ln_Type = NT_MSGPORT;
		NewListType(&unit->su_InputQueue.mp_MsgList, NT_MSGPORT);

		unit->su_OutputQueue.mp_SigTask	= NULL;
		unit->su_OutputQueue.mp_Flags	= PA_IGNORE;
		unit->su_OutputQueue.mp_Node.ln_Type = NT_MSGPORT;
		NewListType(&unit->su_OutputQueue.mp_MsgList, NT_MSGPORT);

		unit->su_csr	= (void *)0x3f8;

		unit->su_cout	= 0;
		unit->su_cin	= 0;
		unit->su_lserr	= 0;
		unit->su_ovrrn	= 0;
		unit->su_iirq	= 0;
		unit->su_oirq	= 0;

		unit->su_iflags	= 0;
		unit->su_istart	= 0;
		unit->su_icount	= 0;

		unit->su_oflags	= 0;
		unit->su_ostart	= 0;
		unit->su_ocount	= 0;

		unit->su_oidle	= 1;
	}

	return SerBase;
}

static void process_messages(UINT32 cmd, pSerUnit Unit, pSerialBase SerBase)
{
	struct IOStdReq *node;
	struct IOStdReq *nextnode;

	switch(cmd)
	{
		case CMD_READ:
			ForeachNodeSafe(&Unit->su_InputQueue.mp_MsgList, node, nextnode)
			{
				commandVector[cmd](SerBase, node);
			}
			break;
		case CMD_WRITE:
			ForeachNodeSafe(&Unit->su_OutputQueue.mp_MsgList, node, nextnode)
			{
				commandVector[cmd](SerBase, node);
			}
			break;
	}
}

static UINT32 serial_handler(unsigned int exc_no, pSerialBase SerBase, pSysBase execBase)
{
	UINT32	u = 0;
	UINT32	iir = 0;
	UINT32	lsr = 0;
	UINT32	count = 0;
	INT8	c;
	pSerUnit	unit = NULL;
	APTR		pucsr = NULL;

	for (u = 0; u < 1; u++)
	{
		unit = &SerBase->ser_Unit[u];
		pucsr= unit->su_csr;

		if (pucsr == NULL)
		{
			if (lsr == 0) continue; // this is for a compiler warning... Srini..will have to look into it.
			continue;
		}

		// Check interrupt register
		iir = pio_read_8(pucsr + UART_IIR);
		if (iir & UART_IIR_IRQ) continue;

		iir &= UART_IIR_IDMASK;

		switch(iir)
		{
			case UART_IIR_RLSI:
				lsr = pio_read_8(pucsr+UART_LSR);
				unit->su_lserr++;
				break;

			case UART_IIR_RDA:
			case UART_IIR_RTO:
				unit->su_iirq++;
				count = 0;
				while (pio_read_8(pucsr+UART_LSR) & UART_LSR_DR)
				{
					c = pio_read_8(pucsr+UART_DATA);
					if (unit->su_icount < UART_IBLEN)
					{
						unit->su_in[(unit->su_istart+unit->su_icount) % UART_IBLEN] = c;
						unit->su_icount++;
						count++;
					} else
					{
						unit->su_ovrrn++;
					}
				}
				unit->su_cin += count;
				process_messages(CMD_READ, unit, SerBase);
				break;

			/* Transmitter holding register empty */
			case UART_IIR_THRE:
				unit->su_oirq++;
				lsr = pio_read_8(pucsr+UART_LSR);  /* Read from LSR to clear interrupt */
				count = 0;
				if (unit->su_ocount > 0)
				{
					/* Write characters to the lower half of the UART. */
					do
					{
						count++;
						unit->su_ocount--;
						pio_write_8(pucsr+UART_DATA, unit->su_out[unit->su_ostart]);
						unit->su_ostart = (unit->su_ostart + 1) % UART_OBLEN;
					} while ((count < UART_FIFO_LEN) && (unit->su_ocount > 0));
				}
				if (count)
				{
					unit->su_cout += count;
					process_messages(CMD_WRITE, unit, SerBase);
				} else
				{
					/* If no characters were written, set the output idle flag. */
					unit->su_oidle = TRUE;
				}
				break;

				/* Modem status change */
			case UART_IIR_MSC:
				break;
		}

	}
	return 1;
}

#define IRQ_SER 4

static pSerialBase ser_OpenDev(pSerialBase SerBase, struct IOStdReq *ioreq, UINT32 unitNum, UINT32 flags)
{
	//KPrintF("[SerDev] Open Unit: %d\n", unitNum);
    SerBase->ser_Device.dev_OpenCnt++;
    SerBase->ser_Device.dev_Flags &= ~LIBF_DELEXP;

    if (unitNum == 0 ) //&& unitNum < 1)
    {
		ioreq->io_Error = 0;
	    ioreq->io_Unit = (struct  Unit *)&SerBase->ser_Unit[unitNum];
	    ioreq->io_Device = (struct Device *)SerBase;
		if (SerBase->ser_Device.dev_OpenCnt <2)
		{
			_HWInit(SerBase, &SerBase->ser_Unit[unitNum]);
			SerBase->SerInt = CreateIntServer("IRQ4 Serial", 100, serial_handler, SerBase);
			AddIntServer(IRQ_SER, SerBase->SerInt);
		}
		//KPrintF("serial.device - Added IRQ4\n");
	} else
	{
		ioreq->io_Error = IOERR_OPENFAIL;
	}
	return SerBase;
}

static pSerialBase ser_CloseDev(pSerialBase SerBase, struct IOStdReq *ioreq)
{
	SerBase->ser_Device.dev_OpenCnt--;
	if(!SerBase->ser_Device.dev_OpenCnt)
	{
		// Should we "expunge" the device?
	}
	ioreq->io_Unit = NULL;
	ioreq->io_Device = NULL;
	return (SerBase);
}

static pSerialBase ser_ExpungeDev(pSerialBase SerBase)
{
	return (NULL);
}

static pSerialBase ser_ExtFuncDev(pSerialBase SerBase)
{
	return (NULL);
}

static void ser_BeginIO(pSerialBase SerBase, pIOStdReq io)
{
	UINT8 cmd = io->io_Command;
	io->io_Flags &= (~(IOF_QUEUED|IOF_CURRENT|IOF_SERVICING|IOF_DONE))&0x0ff;
	io->io_Error = 0;

	if (cmd > UART_MAX_COMMANDS) cmd = 0; // Invalidate the command.
	if (commandQuick[cmd] >= 0)
	{
		CLEAR_BITS(io->io_Flags, IOF_QUICK);
//	KPrintF("QueueCommandVector\n");
		QueueCommandQueue(io, SysBase);
//	KPrintF("QueueCommandVector\n");
		// Check if we are the first in Queue, if not, just return
		if (!TEST_BITS(io->io_Flags, IOF_CURRENT))
		{
			//CLEAR_BITS(io->io_Flags, IOF_QUICK);
			//KPrintF("we are not current, going to sleep\n");
			return;
		}
#if 0
		if ((io->Unit.unit_Flags & DUB_STOPPED) != 0)
		{
			CLEAR_BITS(io->io_Flags, IOF_QUICK);
			return;
		}
#endif
	}
//	KPrintF("CommandVector\n");
	commandVector[cmd](SerBase, io);
}

static void ser_AbortIO(pSerialBase SerBase, pIOStdReq io)
{
	EndCommandQueue(IOERR_ABORTED, io, SysBase);
}


/*******************

Function Table

********************/

volatile static APTR FuncTab[] =
{
	(void(*)) ser_OpenDev,
	(void(*)) ser_CloseDev,
	(void(*)) ser_ExpungeDev,
	(void(*)) ser_ExtFuncDev,

	(void(*)) ser_BeginIO,
	(void(*)) ser_AbortIO,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/

static const struct SerialBase SerialDevData =
{
	.ser_Device.dev_Node.ln_Name = (APTR)&DevName[0],
	.ser_Device.dev_Node.ln_Type = NT_DEVICE,
	.ser_Device.dev_Node.ln_Pri = 50,
	.ser_Device.dev_OpenCnt = 0,
	.ser_Device.dev_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.ser_Device.dev_NegSize = 0,
	.ser_Device.dev_PosSize = 0,
	.ser_Device.dev_Version = DEVICE_VERSION,
	.ser_Device.dev_Revision = DEVICE_REVISION,
	.ser_Device.dev_Sum = 0,
	.ser_Device.dev_IDString = (APTR)&Version[7],
};

// ROMTAG Resident
struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	APTR	InitFunction;
} ser_InitTab =
{
	sizeof(struct SerialBase),
	FuncTab,
	(APTR)&SerialDevData,
	ser_InitDev
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
	&ser_InitTab
};

