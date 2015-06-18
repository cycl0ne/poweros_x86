#include "types.h"
#include "execbase_private.h"
#include "exec_interface.h"
#include "_debug.h"
#include "arch_config.h"
#include "asm.h"

#define IRQ_CLK       0
#define IRQ_KBD       1
#define IRQ_PIC1      2
#define IRQ_PIC_SPUR  7
#define IRQ_RTC       8
#define IRQ_MOUSE     12
#define IRQ_NE2000    5

#define CLK_PORT1  ((UINT16 *) 0x40U)
#define CLK_PORT4  ((UINT16 *) 0x43U)
#define RTC_PORT0  ((UINT16 *) 0x70U)
#define RTC_PORT1  ((UINT16 *) 0x71U)

#define CLK_CONST     1193180
#define MAGIC_NUMBER  1194

#define LOOPS  150000
#define SHIFT  11
#define HZ  100

void arch_irq_mask(UINT16 irqmask);
void arch_irq_unmask(UINT16 irqmask);

static UINT32 clkcnt;

#define PIT_MASK 0xFF
#define PIT_SET 0x36
#define PIT_MODE4 0x38

/*
static __inline__ void
outb(UINT16 port, UINT8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static __inline__ UINT8
inb(UINT16 port)
{
   UINT8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}
*/

void NMI_enable(void)
{
	pio_write_8(RTC_PORT0, pio_read_8(RTC_PORT0)&0x7F);
}

void NMI_disable(void)
{
	pio_write_8(RTC_PORT0, pio_read_8(RTC_PORT0)|0x80);
}

void start_irq_8(struct SysBase* SysBase)
{
	UINT32 ipl;
	UINT8 rate = 9; //128 interrupts per second
	UINT8 prevA, prevB;

	ipl = Disable();

	NMI_disable();
	pio_write_8(RTC_PORT0, 0x8A);		// set index to register A, disable NMI
	prevA=pio_read_8(RTC_PORT1);	// get initial value of register A
	pio_write_8(RTC_PORT0, 0x8A);// reset index to A
	pio_write_8(RTC_PORT1, (prevA & 0xF0) | (rate & 0x0F)); //write only our rate to A. Note, rate is the bottom 4 bits.

	pio_write_8(RTC_PORT0, 0x8B);		// select register B, and disable NMI
	prevB=pio_read_8(RTC_PORT1);	// read the current value of register B
	pio_write_8(RTC_PORT0, 0x8B);		// set the index again (a read will reset the index to register D)
	pio_write_8(RTC_PORT1, prevB | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
	NMI_enable();

	Restore(ipl);
}

void rtc_ack(SysBase *SysBase)
{
	pio_write_8(RTC_PORT0, 0x0C);	// select register C
	pio_read_8(RTC_PORT1);
	//DPrintF("in rtc_ack\n");
}

void start_pit_mode_4()
{
	pio_write_8(CLK_PORT4, PIT_MODE4);
}

/*void set_timer(UINT32 hz)*/
void set_timer(UINT32 div)
{
	/*
	UINT32 div = CLK_CONST / hz;
	pio_write_8(CLK_PORT4, PIT_SET);
	pio_write_8(CLK_PORT1, div & PIT_MASK);
	pio_write_8(CLK_PORT1, (div >> 8) & PIT_MASK);
	*/

	//UINT32 div = 65535;
	//pio_write_8(CLK_PORT4, PIT_MODE4);
	pio_write_8(CLK_PORT1, div & PIT_MASK);
	pio_write_8(CLK_PORT1, (div >> 8) & PIT_MASK);
}

UINT16 get_timer()
{
	UINT16 val = 0;
	UINT8 lb;
	UINT8 hb;
	pio_write_8(CLK_PORT4, 0x00);
	lb = pio_read_8(CLK_PORT1);
	hb = pio_read_8(CLK_PORT1);
	val = hb;
	val = val << 8;
	val = val | lb;
	return val;
}

static UINT32 clock_handler(unsigned int exc_no, APTR Data, SysBase *SysBase)
{
	rtc_ack(SysBase);
//	if (!(IsListEmpty(&SysBase->TaskReady))) Reschedule();
	//if ((SysBase->TDNestCnt < 0) && (!(IsListEmpty(&SysBase->TaskReady))))Schedule();

	//	DPrintF("Clock here\n");
#if 0
	clkcnt++;
	if (clkcnt % 100 == 0)
	{
		//DPrintF("SysBase: %x Data: %x\n", SysBase, Data);
		monitor_put('$');
	}
#endif
	return 1;
}

void arch_clk_init(SysBase *SysBase)
{
	/*
	struct Interrupt *irq;
	irq = CreateIntServer("IRQ0 Clock", -100, clock_handler, SysBase);
	AddIntServer(IRQ_CLK, irq);
	clkcnt =0;
	set_timer(HZ);
	*/
	struct Interrupt *irq;
	irq = CreateIntServer("IRQ8 RTC Clock", -100, clock_handler, SysBase);
	AddIntServer(IRQ_RTC, irq);
	clkcnt =0;
	start_irq_8(SysBase);
}



#if 0
#define IRQ_CLK       0
#define IRQ_KBD       1
#define IRQ_PIC1      2
#define IRQ_PIC_SPUR  7
#define IRQ_MOUSE     12
#define IRQ_NE2000    5

#define CLK_PORT1  ((UINT16 *) 0x40U)
#define CLK_PORT4  ((UINT16 *) 0x43U)

#define CLK_CONST     1193180
#define MAGIC_NUMBER  1194

#define LOOPS  150000
#define SHIFT  11
#define HZ  100

void arch_irq_mask(UINT16 irqmask);
void arch_irq_unmask(UINT16 irqmask);

static UINT32 clkcnt;

#define PIT_MASK 0xFF
#define PIT_SET 0x36

static inline void set_timer(UINT32 hz)
{
	UINT32 div = CLK_CONST / hz;
	pio_write_8(CLK_PORT4, PIT_SET);
	pio_write_8(CLK_PORT1, div & PIT_MASK);
	pio_write_8(CLK_PORT1, (div >> 8) & PIT_MASK);
}

static UINT32 clock_handler(unsigned int exc_no, APTR Data, SysBase *SysBase)
{
	if (!(IsListEmpty(&SysBase->TaskReady))) Reschedule();
//	KPrintF("Clock here\n");
#if 0
	clkcnt++;
	if (clkcnt % 100 == 0)
	{
		//DPrintF("SysBase: %x Data: %x\n", SysBase, Data);
		monitor_put('$');
	}
#endif
	return 1;
}

void arch_clk_init(SysBase *SysBase)
{
	struct Interrupt *irq;
	irq = CreateIntServer("IRQ0 Clock", -100, clock_handler, SysBase);
//DPrintF("clk_init -> irq: %x -> %x\n",irq, irq->is_Code);


//	monitor_write("Going to AddIntServer\n");
//	monitor_write_hex((UINT32)irq);
//	monitor_write("\nLeaving AddIntServer\n");
	AddIntServer(IRQ_CLK, irq);
	clkcnt =0; // for Dbg
	set_timer(HZ);
}
#endif
