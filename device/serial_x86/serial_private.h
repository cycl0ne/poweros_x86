/**
 * @file serial_private.h
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "exec_interface.h"

/* IO-Commands */
#define UART_CTRL_SET_IFLAG   (CMD_NONSTD+0) /**< set input flags               */
#define UART_CTRL_CLR_IFLAG   (CMD_NONSTD+1) /**< clear input flags             */
#define UART_CTRL_GET_IFLAG   (CMD_NONSTD+2) /**< get input flags               */
#define UART_CTRL_SET_OFLAG   (CMD_NONSTD+3) /**< set output flags              */
#define UART_CTRL_CLR_OFLAG   (CMD_NONSTD+4) /**< clear output flags            */
#define UART_CTRL_GET_OFLAG   (CMD_NONSTD+5) /**< get output flags              */
#define UART_CTRL_OUTPUT_IDLE (CMD_NONSTD+6) /**< determine if transmit idle    */

#define UART_MAX_COMMANDS		(CMD_NONSTD+6) // Max number of Commands supported

/* UART Buffer lengths */
#define UART_IBLEN      10240
#define UART_OBLEN      10240

#define DUB_STOPPED (1<<0)


struct uart_csreg
{
    volatile UINT8 buffer;    /**< receive buffer (read only)            */
                              /**<  OR transmit hold (write only)        */
    volatile UINT8 ier;       /**< interrupt enable                      */
    volatile UINT8 iir;       /**< interrupt ident (read only)           */
                              /**<  OR FIFO control (write only)         */
    volatile UINT8 lcr;       /**< line control                          */
    volatile UINT8 mcr;       /**< modem control                         */
    volatile UINT8 lsr;       /**< line status                           */
    volatile UINT8 msr;       /**< modem status                          */
    volatile UINT8 scr;       /**< scratch                               */
};

typedef struct SerUnit 
{
	Unit		su_Unit;

	void		*su_csr;
	UINT32		su_cout;
	UINT32		su_cin;
	UINT32		su_lserr;
	UINT32		su_ovrrn;
	UINT32		su_iirq;
	UINT32		su_oirq;

	MsgPort		su_InputQueue;
	UINT8		su_iflags;
	UINT16		su_istart;
	UINT16		su_icount;
	UINT8		su_in[UART_IBLEN];

	MsgPort		su_OutputQueue;
	UINT8		su_oflags;
	UINT16		su_ostart;
	UINT16		su_ocount;
	UINT8		su_out[UART_OBLEN];
	
	volatile BOOL	su_oidle;	
} SerUnit, *pSerUnit;

typedef struct SerialBase
{
	Device		ser_Device;
	pSysBase	ser_SysBase;

	SerUnit		ser_Unit[2];
	pInterrupt	SerInt;
} SerialBase, *pSerialBase;

