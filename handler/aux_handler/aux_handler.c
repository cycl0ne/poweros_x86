/**
 * @file aux_handler.c
 *
 * This file describes a standard DOS handler for use with the serial device.
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "ctype.h"

#include "types.h"
#include "lists.h"
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

static void AUX_Handler(APTR data);


#define HANDLER_VERSION_STRING "\0$VER: aux.handler 0.1 ("__DATE__")\r\n";
#define HANDLER_VERSION 0

static const char Name[] = "aux.handler";
static const char Version[] = HANDLER_VERSION_STRING

static APTR EndResident;

volatile static RESIDENT_TAG RomTag =
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
	(APTR)AUX_Handler
};

#define CTRLC	0x3
#define CTRLD	0x4
#define CTRLE	0x5
#define CTRLF	0x6
#define CTRLX	0x18
#define EOF		0x1C
#define BMASK	0xff

typedef struct ioRequest
{
	IOStdReq	ir_io;
	pDosPacket	ir_source;
}ioRequest, *pioRequest;

#define TTY_IBLEN	256
#define TTY_RAW		0x1
#define TTY_INLCR	0x2
#define TTY_IGNCR	0x4
#define TTY_ICRNL	0x8
#define TTY_ECHO	0x10

typedef struct Aux
{
	pSysBase	SysBase;
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	
	pMsgPort	ioPort;
	pioRequest	IO_Input;
	pioRequest	IO_Output;



	UINT8		iflags;
	UINT8		oflags;
	BOOL		ieof;
	BOOL		idelim;
	INT8		in[TTY_IBLEN];
	UINT32		istart;
	UINT32		icount;
	INT8		out[TTY_IBLEN];
	BOOL		curs;
	INT8		buffer[512];
	INT32		count;
}Aux, *pAux;

//Put htis into shell.c where it should belong!
#define	STRING1	'PowerOS Operating System and Libraries\n',0
#define	STRING2	'Copyright © 2014, The PowerOS Development Team. All rights reserved.\n',0
#define	STRING3	'0.01 ROM\n',0

static INT32 read_aux(pAux aux, INT8 *buf, INT32 len);


static void _WriteOut(pSysBase SysBase, APTR UtilBase, pIOStdReq io, STRPTR str)
{
	//KPrintF("------------------------------io: %x\n", io);
	io->io_Data	= (APTR) str;
	io->io_Offset	= 0;
	io->io_Command= CMD_WRITE;
	io->io_Length	= Strlen(str);
	DoIO(io);
}

#if 0
static void _SendOut(pSysBase SysBase, pIOStdReq io, INT32 command, APTR buffer, INT32 len)
{
//KPrintF("Sendout command: %d\n", command);
	io->io_Command	= command;
	io->io_Length	= len;
	io->io_Data		= buffer;
	io->io_Offset	= 0;
	SendIO(io);
}
#endif

static void _SendOut2(pSysBase SysBase, pIOStdReq io, INT32 command, APTR buffer, INT32 len)
{
//KPrintF("Sendout2 command: %d\n", command);
	io->io_Error	= 0;
	io->io_Flags	= 0;
	io->io_Actual	= 0;

	io->io_Command	= command;
	io->io_Length	= len;
	io->io_Data		= buffer;
	io->io_Offset	= 0;
	DoIO(io);
}

static inline void _AddPLTail(pSysBase SysBase, pMinList plist, pDosPacket pkt)
{
	AddTail((pList) plist, &pkt->dp_Message.mn_Node);
}

static inline void _SignalProcess(pSysBase SysBase, pProcess proc, UINT32 signal, BOOL nobckgrnd)
{
	if (proc == NULL) return;
	if (proc->pr_Task.tcb_Node.ln_Type == NT_PROCESS)
	{
		//ReadCli Structure
		//clip =
	}
	// nobckgrnd flag testing
	SignalTask(&proc->pr_Task, signal);
}

static DOSIO _getChar(pAux aux)
{
	UINT8	ch;
	_SendOut2(aux->SysBase, (pIOStdReq)aux->IO_Input, CMD_READ, &ch, 1);
	return ch;
}

#if 0
DOSIO auxRead(pAux aux, void *buf, INT32 len);

static void auxEcho(pAux aux, INT8 ch)
{
	pSysBase SysBase = aux->SysBase;
	if (!CheckIO((pIOStdReq)aux->IO_Output)) WaitIO((pIOStdReq)aux->IO_Output);

	aux->IO_Output->ir_source = NULL; // this is an internal write
	if ((ch == '\b') || (ch == 0x7f))
	{
		UINT8	delete[] = "\b \b";
		_SendOut2(SysBase, (pIOStdReq)aux->IO_Output, CMD_WRITE, delete, 3);
	}
	if (!isprint(ch)) return;
//	KPrintF("Char: %x,",ch);
//	if (ch == '\n') return;
	_SendOut2(SysBase, (pIOStdReq)aux->IO_Output, CMD_WRITE, &ch, 1);
	return;
}
#endif

DOSIO auxWrite(pAux aux, void *buf, UINT32 len);

static void AUX_Handler(APTR SysBase)
{
//	pSysBase		SysBase 	= data;
	pDOSBase		DOSBase		= OpenLibrary("dos.library", 0);
	pLibrary		UtilBase	= OpenLibrary("utility.library", 0);
	
//	KPrintF("--------------------Starting Handler: AUX\n");
	if (DOSBase == NULL)
	{
		KPrintF("[aux_handler] dos.library not opened\n");
		return;
	}

	pDosPacket	packet	= WaitPkt();
	pAux		aux		= AllocVec(sizeof(Aux), MEMF_PUBLIC|MEMF_CLEAR);
	
	MinList		current_input;
	MinList		current_output;

	INT32		usecount = 0;
//	BOOL		EOFpend = FALSE;

	STRPTR name 	= (STRPTR) packet->dp_Arg1;
	pDosEntry de	= (pDosEntry) packet->dp_Arg3;

//	KPrintF("[AUX] Recved Startuppkt: [%x] Startup: [%d]: Call: [%s]\n", packet, packet->dp_Arg2, name);

	pProcess	this = FindProcess(NULL);
	pMsgPort	handlerport = &this->pr_MsgPort;

	aux->ioPort = CreateMsgPort(NULL);
	aux->IO_Input	= (pioRequest)CreateIORequest(aux->ioPort, sizeof(ioRequest));
	aux->IO_Output	= (pioRequest)CreateIORequest(aux->ioPort, sizeof(ioRequest));
	aux->iflags		= TTY_ECHO|TTY_ICRNL; //|TTY_RAW;
	aux->oflags		= TTY_INLCR;
	aux->ieof		= FALSE;
	aux->idelim		= FALSE;
	
	aux->count		= 0;

	aux->SysBase		= SysBase;
	aux->DOSBase		= DOSBase;
	aux->UtilBase		= UtilBase;
	
	NewList((pList)&current_input);
	NewList((pList)&current_output);

//	KPrintF("aux: IO_Input: %x, Output: %x\n", aux->IO_Input, aux->IO_Output);

	if (OpenDevice("serial.device", 0, (pIOStdReq)aux->IO_Input, 0) != 0)
	{
		KPrintF("[AUX] Serial.device failed\n");
		DeleteMsgPort(aux->ioPort);
		DeleteIORequest(&aux->IO_Input->ir_io);
		DeleteIORequest(&aux->IO_Output->ir_io);
		ReplyPkt(packet, DOSIO_FALSE, RETURN_FAIL);
		return;
	}

	//CopyMem(IO_Input, IO_Output, sizeof(ioRequest));
	OpenDevice("serial.device", 0, (pIOStdReq)aux->IO_Output, 0);
	de->de_Handler = handlerport; // this is IMPORTANT!!!
	ReplyPkt(packet, DOSIO_TRUE, RETURN_OK);

//	KPrintF("[AUX] Replied Startuppkt handlerport: %x\n", IsListEmpty(&handlerport->mp_MsgList));

	_WriteOut(SysBase, UtilBase, (pIOStdReq)aux->IO_Output, "PowerOS Operating System and Libraries\n");
	_WriteOut(SysBase, UtilBase, (pIOStdReq)aux->IO_Output, "Copyright © 2014, The PowerOS Development Team. All rights reserved.\n");
	_WriteOut(SysBase, UtilBase, (pIOStdReq)aux->IO_Output, "0.02 ROM\n");

	//---------------------------
	pDosPacket	dp = NULL;
	pFileHandle fh;
	UINT32 count = 0;
	//---------------------------
//	KPrintF("[AUX] Process: %s, MsgPort: %x\n", this->pr_Task.tcb_Node.ln_Name, handlerport);
//	KPrintF("[AUX] Handlerport empty?: %x\n", IsListEmpty(&handlerport->mp_MsgList));

	while(TRUE)
	{

		while( (dp= (pDosPacket) GetMsg(handlerport))  != NULL )
		{
			//KPrintF("[AUX]DosPacket Loop %x Action: %d %d\n", dp, dp->dp_Action, 'R');
			switch(dp->dp_Action)
			{
				case ACTION_FINDINPUT:
				case ACTION_FINDOUTPUT:
				case ACTION_FINDUPDATE:
					fh					= (pFileHandle)dp->dp_Arg1;
					fh->fh_Interactive	= DOSIO_TRUE;
					fh->fh_Arg1			= (INT32) fh;
					//if (usecount == 0) _SendOut(SysBase, (pIOStdReq)IO_Input, CMD_READ, charbuf, 1);
					usecount++;
					ReplyPkt(dp, DOSIO_TRUE, RETURN_OK);
					break;
				case ACTION_END:
					usecount--;
					//if (usecount > 0) EOFpend = 0;
					// Should we remove pending ios from our in/ouput list?
					ReplyPkt(dp, DOSIO_TRUE, RETURN_OK);
					break;
				case ACTION_READ:
					aux->IO_Input->ir_source = dp;
					//KPrintF("Reading AUX: %d\n", dp->dp_Arg3);
					//count	= auxRead(aux, (INT8*)dp->dp_Arg2, (UINT32)dp->dp_Arg3);
					count = read_aux(aux, (INT8*)dp->dp_Arg2, (UINT32)dp->dp_Arg3);
					ReplyPkt(dp, count, 0);
					break;
				case ACTION_WRITE:
					aux->IO_Output->ir_source = dp;
					count = auxWrite(aux, (UINT8*)dp->dp_Arg2, (UINT32)dp->dp_Arg3);
					ReplyPkt(dp, count, 0);
					break;
				default:
					ReplyPkt(dp, DOSIO_FALSE, ERROR_ACTION_NOT_KNOWN);
					break;
			}
		}

		WaitSignal( (1 << handlerport->mp_SigBit) );
	}
}

DOSIO auxWrite(pAux aux, void *buf, UINT32 len)
{
	pSysBase	SysBase = aux->SysBase;
	UINT32	ch		= 0;
	UINT32	count	= 0;
	UINT32	ret		= 0;
	UINT8*	buffer	= buf;

	while (ret < len)
	{
		ch = *buffer++;
		switch(ch)
		{
			case '\n':
				if (aux->oflags & TTY_INLCR)
				{
					aux->out[count] = '\r';
					count++;
				}
				break;
			case '\r':
				if (aux->oflags & TTY_ICRNL) ch = '\n';
				break;
		}
		aux->out[count] = ch;
		count++;
		ret++;
		if (count >= 254 || ret == len)
		{
			if (CheckIO((pIOStdReq)aux->IO_Output)) WaitIO((pIOStdReq)aux->IO_Output);
			_SendOut2(SysBase, (pIOStdReq)aux->IO_Output, CMD_WRITE, aux->out, count);
			count = 0;
		}
	}
	return ret;
}

//#define SysBase aux->SysBase;
#define SysBase 	aux->SysBase
#define DOSBase		aux->DOSBase
#define UtilBase	aux->UtilBase

#if 0
DOSIO auxRead(pAux aux, void *buf, INT32 len)
{
//	pSysBase SysBase = aux->SysBase;
	INT8	ch		= 0;
	INT32	count	= 0;
	INT8*	buffer	= buf;
//KPrintF("AUXREAD: ieof: %d\n", aux->ieof);
	if (aux->ieof)
	{
		aux->ieof	= FALSE;
		return	ENDSTREAMCH;
	}
//	KPrintF("AUX Read: %x, %d\n",buf, len);
	if (aux->iflags & TTY_RAW)
	{
		while ( (0< aux->icount) && (count < len) )
		{
			//KPrintF("read from buffer\n");
			*buffer++ = aux->in[aux->istart];
			aux->icount--;
			aux->istart = (aux->istart+1) % TTY_IBLEN;
			count++;
		}

		while (count<len)
		{
			//KPrintF("Getting char\n");
			ch = _getChar(aux);
			//KPrintF("char: %c\n",ch);
			*buffer++ = ch;
			count++;

			if (aux->iflags & TTY_ECHO) auxEcho(aux, ch);
		}
		return count;
	}

	while ( (aux->icount < TTY_IBLEN) && !aux->idelim ) // && (aux->icount <len))
	{
		//KPrintF("Cooked: Getchar\n");
		ch = _getChar(aux);
		//KPrintF("Cooked: Getchar\n");
KPrintF("[%x][%c][%x]-", ch,ch, '\b');
		if ( (ch>= CTRLC) && (ch<= CTRLF) )
		{
			/*
			pDosPacket dp = aux->IO_Input->ir_source;
			if (dp)
			{
				//KPrintF("Signal Task [%x/%s] with a CTRL-  [%x]\n", dp->dp_Message.mn_ReplyPort->mp_SigTask, dp->dp_Message.mn_ReplyPort->mp_SigTask->tcb_Node.ln_Name,  (1<<(ch+9)));
				_SignalProcess(aux->SysBase, (pProcess)dp->dp_Message.mn_ReplyPort->mp_SigTask, (1<<(ch+9)), TRUE);
			}
			*/
		}
		switch (ch)
		{
			case '\b':
			case 0x7F:
				if (aux->icount <1) continue;
				aux->icount--;
				break;
			case '\n':
				if (aux->iflags & TTY_INLCR) ch = '\r';
				aux->in[(aux->istart + aux->icount) % TTY_IBLEN] = ch;
				aux->icount++;
				aux->idelim = TRUE;
				//aux->ieof = TRUE;
				//KPrintF("found /n ieof: %d\n", aux->ieof);
				break;
			case '\r':
				if (aux->iflags & TTY_IGNCR) continue;
				if (aux->iflags & TTY_ICRNL) ch = '\n';
				aux->in[(aux->istart + aux->icount) % TTY_IBLEN] = ch;
				aux->icount++;
				aux->idelim = TRUE;
				//aux->ieof = TRUE;
				//KPrintF("found /r ieof: %d\n", aux->ieof);
				break;
			case 0x04:
				aux->ieof	= TRUE;
				aux->idelim	= TRUE;
				break;
#if 0
			case 0x5b:
				aux->curs	= TRUE;
				break;
			case 0x44: //left cursor
				if (aux->curs)
				{
					if (!CheckIO((pIOStdReq)aux->IO_Output)) WaitIO((pIOStdReq)aux->IO_Output);
					aux->IO_Output->ir_source = NULL; // this is an internal write
					UINT8	left[] = "\033[1D";
					_SendOut2(SysBase, (pIOStdReq)aux->IO_Output, CMD_WRITE, left, 5);
				}
				break;
			case 0x43: //right cursor
				if (aux->curs)
				{
					if (!CheckIO((pIOStdReq)aux->IO_Output)) WaitIO((pIOStdReq)aux->IO_Output);
					aux->IO_Output->ir_source = NULL; // this is an internal write
					UINT8	right[] = "\033[1C";
					_SendOut2(SysBase, (pIOStdReq)aux->IO_Output, CMD_WRITE, right, 5);	
				}
				break;
#endif
			default:
				if (!isprint(ch)) continue;
				aux->in[(aux->istart + aux->icount) % TTY_IBLEN] = ch;
				aux->icount++;
				break;
		}

		if ((aux->iflags & TTY_ECHO) && (!aux->curs)) auxEcho(aux, ch);
		if (aux->iflags & TTY_RAW) break;
		if (aux->curs && ((ch == 0x44) || (ch == 0x43))) aux->curs = FALSE;
	}
//KPrintF("icount: %d, count: %d, len: %d\n", aux->icount, count, len);
	while ( (0< aux->icount) && (count < len) )
	{
		*buffer++ = aux->in[aux->istart];
		aux->icount--;
		aux->istart = (aux->istart+1) % TTY_IBLEN;
		count++;
	}

	if (0 == aux->icount) aux->idelim = FALSE;
//KPrintF("count = %d %d", count, aux->ieof);
	if (0 == count && aux->ieof)
	{
		aux->ieof	= FALSE;
//		for(;;);
		return	ENDSTREAMCH;
	}
	return count;
}
#endif

static void aux_flush(pAux aux)
{
	if (aux->count == 0) return;
	
	APTR buffer = aux->buffer;
	INT32 len	= aux->count;//Strlen((STRPTR)buffer);
	
	pIOStdReq io = (pIOStdReq)aux->IO_Output;
	io->io_Error	= 0;
	io->io_Flags	= 0;
	io->io_Actual	= 0;

	io->io_Command	= CMD_WRITE;
	io->io_Length	= len;
	io->io_Data		= buffer;
	io->io_Offset	= 0;
	DoIO(io);
	
	aux->count		= 0;
}

static void _prbuf(INT32 ch, INT32 **arg)
{
	INT32 *args = *arg;
	INT8 *buffer = (INT8*)args[0];
	pAux aux = (pAux)args[1];
	buffer[aux->count++] = ch;
}

static void aux_printf(pAux aux, STRPTR string, ...)
{
	va_list fmt;
	va_start(fmt, string);

	INT8	*buffer = aux->buffer;
	INT32 args[2];
	args[0] = (INT32)buffer;
	args[1] = (INT32)aux;
	if (aux->count >= 256) aux_flush(aux);

	RawDoFmt((const char *)string, fmt,(void(*)()) _prbuf, (APTR) args);
	va_end(fmt);

//	INT32 len = Strlen((STRPTR)buffer);
//	aux->count = len;
//	if (aux->count>200) KPrintF("count: %d\n", aux->count);
/*	
	pIOStdReq io = (pIOStdReq)aux->IO_Output;
	io->io_Error	= 0;
	io->io_Flags	= 0;
	io->io_Actual	= 0;

	io->io_Command	= CMD_WRITE;
	io->io_Length	= len;
	io->io_Data		= buffer;
	io->io_Offset	= 0;
	DoIO(io);
*/
}

#if 1
#define KEY_ARROW_UP			256
#define KEY_ARROW_DOWN			257
#define KEY_ARROW_LEFT			258
#define KEY_ARROW_RIGHT			259
#define KEY_SHIFT_ARROW_LEFT	260
#define KEY_SHIFT_ARROW_RIGHT	261


static INT32 read_ch(pAux aux)
{
	INT8 ch = _getChar(aux);
	if (ch == 0x01) return KEY_SHIFT_ARROW_LEFT;
	if (ch == 0x05) return KEY_SHIFT_ARROW_RIGHT;
	
	if (ch == '\033')
	{
		ch = _getChar(aux);
		if (ch == '[')
		{
			ch = _getChar(aux);
			switch (ch)
			{
				case '1':
					ch = _getChar(aux); //;
					ch = _getChar(aux); //2
					ch = _getChar(aux); //Letter
					if (ch == 'C') return KEY_SHIFT_ARROW_RIGHT;
					return KEY_SHIFT_ARROW_LEFT;
				case 'A':
					return KEY_ARROW_UP;
				case 'B':
					return KEY_ARROW_DOWN;
				case 'C':
					return KEY_ARROW_RIGHT;
				case 'D':
					return KEY_ARROW_LEFT;
			}
		}
	}
	return ch;
}

static INT32 read_aux(pAux aux, INT8 *buf, INT32 len)
{
	INT8*	buffer		= buf;
	INT32	collected	= 0;
	INT32	requested	= len;
	INT32	newline		= 0;
	INT32	offset		= 0;
//	INT32	tabbed		= 0;
//	INT32	cancel		= 0;

	INT32	ch		= 0;

	aux_printf(aux, "\033[s");
	aux_flush(aux);
	
	while ((collected < requested) && (!newline))
	{
		ch = read_ch(aux);

		switch(ch)
		{
			case KEY_ARROW_UP:
			case KEY_ARROW_DOWN:
				continue;
			case KEY_ARROW_RIGHT:
				if (offset < collected)
				{
					aux_printf(aux, "\033[C");
					aux_flush(aux);
					offset++;
				}
				continue;
			case KEY_ARROW_LEFT:
				if (offset > 0)
				{
					aux_printf(aux, "\033[D");
					aux_flush(aux);
					offset--;
				}
				continue;
			case KEY_SHIFT_ARROW_LEFT:
				aux_printf(aux, "\033[%dD", offset);
				aux_flush(aux);
				offset = 0;
				continue;
			case KEY_SHIFT_ARROW_RIGHT:
				aux_printf(aux, "\033[%dC", collected);
				aux_flush(aux);
				offset = collected;
				continue;
			case '\b':
			case 0x7F:
				if (collected)
				{
					if (!offset) continue;
					aux_printf(aux, "\b \b");
					if (offset != collected)
					{
						int	remaining = collected - offset;
						for (int i = 0; i < remaining; ++i)
						{
							aux_printf(aux, "%c", buffer[offset+i]);
							buffer[offset + i - 1] = buffer[offset + i];
						}
						aux_printf(aux, " ");
					#if 0
						for (int i = 0; i < remaining + 1; ++i)
						{
							aux_printf(aux, "\033[D");
						}
					#else
						if (remaining+1 > 0) aux_printf(aux, "\033[%dD", remaining+1);
					#endif
						offset--;
						collected--;
					} else
					{
						buffer[--collected] = '\0';
						offset--;
					}
					aux_flush(aux);
				}
				continue;
			case '\n':
			case '\r':
				if (aux->iflags & TTY_ICRNL) ch = '\n';
#if 0
				while (offset < collected) 
				{
					aux_printf(aux, "\033[C");
					offset++;
				}
#else
				aux_printf(aux, "\033[%dC", collected);
				offset = collected;
#endif			
				if (collected < requested)
				{
					buffer[collected]	= '\n';
					buffer[++collected]	= '\0';
					offset++;
				}
				aux_printf(aux, "\n");
				aux_flush(aux);
				newline = 1;
				continue;		
		}
		if (offset != collected) 
		{
			for (int i = collected; i > offset; --i) 
			{
				buffer[i] = buffer[i-1];
			}
			if (collected < requested) 
			{
				buffer[offset] = (char) ch;
				buffer[++collected] = '\0';
				offset++;
			}
			for (int i = offset - 1; i < collected; ++i) 
			{
				aux_printf(aux, "%c", buffer[i]);
			}
#if 0
			for (int i = offset; i < collected; ++i) 
			{
				aux_printf(aux, "\033[D");
			}
#else
			int i = collected-offset;
			if (i > 0)	aux_printf(aux, "\033[%dD", i);
#endif			
//			KPrintF("Offset: %d, Collected: %d\n", offset, collected);
			aux_flush(aux);
		} else {
			aux_printf(aux, "%c", (char)ch);
			if (collected < requested) 
			{
				buffer[collected] = (char)ch;
				buffer[++collected] = '\0';
				offset++;
			}
			aux_flush(aux);
		}
	}
	buffer[collected] = '\0';
//	KPrintF("Collected: %d\n", collected);
	return collected;
}
#endif