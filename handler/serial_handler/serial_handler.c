/**
 * @file serial_handler.c
 *
 * This file describes a standard DOS handler for use with the serial device.
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "devices.h"
#include "ports.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "residents.h"

#include "exec_interface.h"
#include "utility_interface.h"
#include "dos_interface.h"

static void Serial_Handler(APTR data);


#define HANDLER_VERSION_STRING "\0$VER: serial.handler 0.1 ("__DATE__")\r\n";
#define HANDLER_VERSION 0

static const char Name[] = "serial.handler";
static const char Version[] = HANDLER_VERSION_STRING

static APTR EndResident;

static RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	0, // RTF_AUTOINIT | RTF_COLDSTART,
	HANDLER_VERSION,
	NT_HANDLER,
	-120,
	(APTR)Name,
	(APTR)Version,
	(APTR)Serial_Handler
};

typedef struct ioRequest
{
	IOStdReq	ir_io;
	pDosPacket	ir_source;
}ioRequest, *pioRequest;

static void Serial_Handler(APTR data)
{
	pSysBase		SysBase 	= data;
	pDOSBase		DOSBase		= OpenLibrary("dos.library",0);
	pLibrary		UtilityBase	= OpenLibrary("utility.library",0);

	pioRequest		ioReq;

	pMsgPort		ioPort = CreateMsgPort(NULL);
	pMsgPort		packetPort = CreateMsgPort(NULL);
	pDosPacket		packet;

	INT32			result1, result2;
	pFileHandle		fh;


	// go hang yourself, no dos.library could be opened, but how? we got a packet from dos.
	if (DOSBase == NULL)
	{
		KPrintF("[serial_handler] dos.library not opened\n");
		return;
	}

	packet = WaitPkt();

	if ((!ioPort) || (!packetPort) || (!UtilityBase) || (!packet))
	{
		KPrintF("[serial_handler] utility.library/ioPort/packetPort/packet not opened/created\n");
		DeleteMsgPort(ioPort);
		DeleteMsgPort(packetPort);
		CloseLibrary(DOSBase);
		CloseLibrary(UtilityBase);
		ReplyPkt(packet, DOSIO_FALSE, RETURN_FAIL);
		return;
	}

	/*
	* TODO !!!!! Some Initialisation Stuff TOBEDONE (NOT FINISHED SINCE DOS CODE IS MISSING)
	* EXAMPLE: MSGPORT FOR DOS!
	*/

	// Tell that we are now ready
	ReplyPkt(packet, DOSIO_TRUE, RETURN_OK);

	while(TRUE)
	{
		while ((packet = (pDosPacket) GetMsg(packetPort)) != NULL)
		{
			ioReq = (pioRequest) packet->dp_Arg1;  // dp_Arg1 allways holds the fh_Arg1 which we set to our ioReq Structure in ACTION_FINDxxxx

			switch (packet->dp_Action)
			{
                case ACTION_FINDINPUT :
                case ACTION_FINDOUTPUT:
                case ACTION_FINDUPDATE:
					ioReq = (pioRequest) CreateIORequest(ioPort, sizeof(ioRequest));
					if (ioReq != NULL)
					{
						if (OpenDevice("serial.device", 0, &ioReq->ir_io, 0) == 0)
						{
							fh 					= (pFileHandle) packet->dp_Arg1;
							fh->fh_Interactive	= (BOOL) TRUE;  // we are interactive
							fh->fh_Arg1			= (INT32) ioReq;

							result1	= DOSIO_TRUE;
							result2 = 0;
						} else
						{
							result2 = ERROR_OBJECT_NOT_FOUND;
							if (ioReq->ir_io.io_Error == IOERR_UNITBUSY) result2 = ERROR_OBJECT_IN_USE;
							DeleteIORequest(&ioReq->ir_io);
						}
					} else
					{
						result2 = ERROR_NO_FREE_STORE;
					}
					break;

				case ACTION_END  :
					CloseDevice(&ioReq->ir_io);
					DeleteIORequest(&ioReq->ir_io);
					result1 = DOSIO_TRUE;
					result2 = 0;
					break;

				case ACTION_WRITE:
					ioReq->ir_io.io_Command	= CMD_WRITE;

				case ACTION_READ:
					if (packet->dp_Action == ACTION_READ) ioReq->ir_io.io_Command	= CMD_READ;
					ioReq->ir_io.io_Data	= (APTR) packet->dp_Arg2;
					ioReq->ir_io.io_Length	= (INT32) packet->dp_Arg3;
					ioReq->ir_source		= packet;
					SendIO(&ioReq->ir_io);
					packet = NULL; // no packet until IO from Device returns
					break;

				case ACTION_IS_FILESYSTEM:
					result1 = DOSIO_FALSE;
					result2 = 0;
					break;

				default:
					result1 = DOSIO_FALSE;
					result2 = ERROR_ACTION_NOT_KNOWN;
					break;
			}
			if (packet) ReplyPkt(packet, result1, result2);
		}

		while ((ioReq = (pioRequest) GetMsg(ioPort)) !=NULL)
		{
			packet = ioReq->ir_source;
			if (ioReq->ir_io.io_Error)
			{
				ReplyPkt(packet, -1, ioReq->ir_io.io_Error);
			} else
			{
				ReplyPkt(packet, ioReq->ir_io.io_Actual, 0);
			}
		}

		WaitSignal((1 << packetPort->mp_SigBit) | (1 << ioPort->mp_SigBit));
	}
}

