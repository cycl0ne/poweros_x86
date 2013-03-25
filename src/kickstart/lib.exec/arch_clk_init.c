#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "arch_config.h"
#include "irq.h"
#include "exec_funcs.h"
#include "asm.h"

#define IRQ_CLK       0
#define IRQ_KBD       1
#define IRQ_PIC1      2
#define IRQ_PIC_SPUR  7
#define IRQ_MOUSE     12
#define IRQ_NE2000    5

#define CLK_PORT1  ((UINT8 *) 0x40U)
#define CLK_PORT4  ((UINT8 *) 0x43U)

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
	if ((SysBase->TDNestCnt < 0) && (!(IsListEmpty(&SysBase->TaskReady)))) Schedule();
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
//	monitor_write("Going to AddIntServer\n");
//	monitor_write_hex((UINT32)irq);
//	monitor_write("\nLeaving AddIntServer\n");
	AddIntServer(IRQ_CLK, irq);
	clkcnt =0; // for Dbg
	set_timer(HZ);
}

