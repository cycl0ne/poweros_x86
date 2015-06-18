/**
 * @file hw_init.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"
#include "asm.h"
/* Base address for TTY */
#define UART_BASE       0x3f8

/* UART Register numbers                                                 */
#define UART_DATA 0
#define UART_IER  1
#define UART_IIR  2
#define UART_FCR  2
#define UART_LCR  3
#define UART_MCR  4
#define UART_LSR  5
#define UART_MSR  6

#define UART_DLL  0 /* LCR_DLAB = 1 */
#define UART_DLM  1 /* LCR_DLAB = 1 */

/* Alternative names for control and status registers                   */
#define rbr buffer              /**< receive buffer (read only)         */
#define thr buffer              /**< transmit hold (write only)         */
#define fcr iir                 /**< FIFO control (write only)          */
#define dll buffer              /**< divisor latch low byte             */
#define dlm ier                 /**< divisor latch high byte            */

/* UART Bit flags for control and status registers                      */
/* Interrupt enable bits                                                */
#define UART_IER_ERBFI  0x01    /**< Received data interrupt mask       */
#define UART_IER_ETBEI  0x02    /**< Transmitter buffer empty interrupt */
#define UART_IER_ELSI   0x04    /**< Recv line status interrupt mask    */
#define UART_IER_EMSI   0x08    /**< Modem status interrupt mask        */

/* Interrupt identification masks */
#define UART_IIR_IRQ    0x01    /**< Interrupt pending bit              */
#define UART_IIR_IDMASK 0x0E    /**< 3-bit field for interrupt ID       */
#define UART_IIR_MSC    0x00    /**< Modem status change                */
#define UART_IIR_THRE   0x02    /**< Transmitter holding register empty */
#define UART_IIR_RDA    0x04    /**< Receiver data available            */
#define UART_IIR_RLSI   0x06    /**< Receiver line status interrupt     */
#define UART_IIR_RTO    0x0C    /**< Receiver timed out                 */

/* FIFO control bits */
#define UART_FCR_EFIFO  0x01    /**< Enable in and out hardware FIFOs   */
#define UART_FCR_RRESET 0x02    /**< Reset receiver FIFO                */
#define UART_FCR_TRESET 0x04    /**< Reset transmit FIFO                */
#define UART_FCR_TRIG0  0x00    /**< RCVR FIFO trigger level one char   */
#define UART_FCR_TRIG1  0x40    /**< RCVR FIFO trigger level 1/4        */
#define UART_FCR_TRIG2  0x80    /**< RCVR FIFO trigger level 2/4        */
#define UART_FCR_TRIG3  0xC0    /**< RCVR FIFO trigger level 3/4        */

/* Line control bits */
#define UART_LCR_DLAB   0x80    /**< Divisor latch access bit           */
#define UART_LCR_8N1    0x03    /**< 8 bits, no parity, 1 stop          */

/* Modem control bits */
#define UART_MCR_OUT2   0x08    /**< User-defined OUT2.                 */
#define UART_MCR_LOOP   0x10    /**< Enable loopback test mode          */

/* Line status bits */
#define UART_LSR_DR     0x01    /**< Data ready                         */
#define UART_LSR_THRE   0x20    /**< Transmit-hold-register empty       */
#define UART_LSR_TEMT   0x40    /**< Transmitter empty                  */

#define UART_FIFO_LEN   16      /**< Size of the hardware FIFO buffer   */

static SysCall _HWInit(pSerialBase SerBase, pSerUnit ser)
{
    void *pucsr = ser->su_csr;

    /* Set baud rate */
    pio_write_8(pucsr+UART_LCR, UART_LCR_DLAB);  /* divisor latch access */
    pio_write_8(pucsr+UART_DLL, 1); /* divisor latch low */
                                                 /* DLL 12 -> 9600 bps   */
                                                 /* DLL 1 -> 115200 bps  */
    pio_write_8(pucsr+UART_DLM, 0);              /* divisor latch high   */

    pio_write_8(pucsr+UART_LCR, UART_LCR_8N1);   /* 8N1 mode             */
    pio_write_8(pucsr+UART_FCR, 0);              /* Disable FIFO         */
    pio_write_8(pucsr+UART_MCR, UART_MCR_OUT2);  /* User-defined OUT2    */
    /* OUT2 is used to control the board's interrupt tri-state           */
    /* buffer. It should be set high to generate interrupt properly.     */

    /* Enable interrupts */
    pio_write_8(pucsr+UART_IER, UART_IER_ERBFI | UART_IER_ETBEI | UART_IER_ELSI);

    /* Enable UART hardware FIFOs, clear contents and set interrupt trigger level */
    pio_write_8(pucsr+UART_FCR,
         UART_FCR_EFIFO | UART_FCR_RRESET | UART_FCR_TRESET | UART_FCR_TRIG2);

//    set_handler(IRQBASE+devptr->irq, devptr->intr);

    return OK;
}

