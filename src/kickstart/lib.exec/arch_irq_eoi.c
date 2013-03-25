#include "types.h"
#include "asm.h"
#include "i8259.h"

void arch_irq_eoi(UINT16 irqmask)
{
    if (irqmask & 0xff00)
    {
        pio_write_8(PIC_PIC1PORT1, 0x20);
    }
    pio_write_8(PIC_PIC0PORT1, 0x20);
}
