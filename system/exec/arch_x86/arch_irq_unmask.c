#include "types.h"
#include "asm.h"
#include "i8259.h"

void arch_irq_unmask(UINT16 irqmask)
{
	UINT8 x;

	if (irqmask & 0xff) {
		x = pio_read_8(PIC_PIC0PORT2);
		pio_write_8(PIC_PIC0PORT2, (UINT8) (x & (~(irqmask & 0xff))));
	}
	if (irqmask >> 8) {
		x = pio_read_8(PIC_PIC1PORT2);
		pio_write_8(PIC_PIC1PORT2, (UINT8) (x & (~(irqmask >> 8))));
	}
}
