#include "types.h"
#include "asm.h"
#include "i8259.h"

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */

void arch_irq_init(void)
{
    pio_write_8(PIC_PIC0PORT1, 0x11);
    pio_write_8(PIC_PIC1PORT1, 0x11);
    pio_write_8(PIC_PIC0PORT2, 0x20);
    pio_write_8(PIC_PIC1PORT2, 0x28);
    pio_write_8(PIC_PIC0PORT2, 0x04);
    pio_write_8(PIC_PIC1PORT2, 0x02);
    pio_write_8(PIC_PIC0PORT2, 0x01);
    pio_write_8(PIC_PIC1PORT2, 0x01);
    pio_write_8(PIC_PIC0PORT2, 0x0);
    pio_write_8(PIC_PIC1PORT2, 0x0);
    /*
	pio_write_8(PIC_PIC0PORT1, PIC_ICW1 | PIC_NEEDICW4);
	pio_write_8(PIC_PIC0PORT2, IVT_IRQBASE);
	pio_write_8(PIC_PIC0PORT2, 1 << IRQ_PIC1);
	pio_write_8(PIC_PIC0PORT2, 1);
	pio_write_8(PIC_PIC1PORT1, PIC_ICW1 | PIC_NEEDICW4);
	pio_write_8(PIC_PIC1PORT2, IVT_IRQBASE + 8);
	pio_write_8(PIC_PIC1PORT2, IRQ_PIC1);
	pio_write_8(PIC_PIC1PORT2, 1);
	pio_wr	
	*/
}