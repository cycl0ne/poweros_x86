#include "types.h"
#include "sysbase.h"
#include "asm.h"
#include "exec_funcs.h"
#include "_debug.h"

#define SERIAL_PORT_A ((UINT8 *)0x3F8)
#define SERIAL_PORT_B ((UINT8 *)0x2F8)
#define SERIAL_PORT_C ((UINT8 *)0x3E8)
#define SERIAL_PORT_D ((UINT8 *)0x2E8)


void arch_RawIOInit()
{
	pio_write_8(SERIAL_PORT_A + 1, 0x00);
	pio_write_8(SERIAL_PORT_A + 3, 0x80); /* Enable divisor mode */
	pio_write_8(SERIAL_PORT_A + 0, 0x03); /* Div Low:  03 Set the port to 38400 bps */
	pio_write_8(SERIAL_PORT_A + 1, 0x00); /* Div High: 00 */
	pio_write_8(SERIAL_PORT_A + 3, 0x03);
	pio_write_8(SERIAL_PORT_A + 2, 0xC7);
	pio_write_8(SERIAL_PORT_A + 4, 0x0B);
}

void arch_RawPutChar(UINT8 chr)
{
	while ((pio_read_8(SERIAL_PORT_A + 5) & 0x20) == 0);
	pio_write_8(SERIAL_PORT_A, chr);
}

UINT8 arch_RawMayGetChar()
{
	return pio_read_8(SERIAL_PORT_A);	
}

INT32 lib_RawMayGetChar(struct SysBase *SysBase)
{
	return arch_RawMayGetChar();
}

void lib_RawPutChar(struct SysBase *SysBase, UINT8 chr)
{
	arch_RawPutChar(chr);
}

void lib_RawIOInit(struct SysBase *SysBase)
{
	arch_RawIOInit();
}
