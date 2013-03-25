#ifndef i8259_h
#define i8259_h

#define PIC_PIC0PORT1  ((volatile UINT8 *) 0x20U)
#define PIC_PIC0PORT2  ((volatile UINT8 *) 0x21U)
#define PIC_PIC1PORT1  ((volatile UINT8 *) 0xa0U)
#define PIC_PIC1PORT2  ((volatile UINT8 *) 0xa1U)

#define PIC_NEEDICW4  (1 << 0)
#define PIC_ICW1      (1 << 4)

#define IVT_FIRST  0
#define EXC_COUNT  32
#define IRQ_COUNT  16

#define IVT_EXCBASE   0
#define IVT_IRQBASE   (IVT_EXCBASE + EXC_COUNT)
#define IVT_FREEBASE  (IVT_IRQBASE + IRQ_COUNT)

#define IRQ_CLK       0
#define IRQ_KBD       1
#define IRQ_PIC1      2
#define IRQ_NE2000    5
#define IRQ_PIC_SPUR  7
#define IRQ_MOUSE     12

#endif
