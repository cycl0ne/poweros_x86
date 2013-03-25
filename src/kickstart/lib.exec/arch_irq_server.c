#include "types.h"
#include "sysbase.h"
#include "arch_config.h"
#include "_debug.h"
#include "exec_funcs.h"

// This Code is for Interrupt Handling.
// this code is injected from the Exception Handler (arch_exc_server)
void arch_irq_eoi(UINT16 irqmask);

__attribute__((no_instrument_function))  void arch_irq_server(unsigned int exc_no, registers_t regs, APTR Data, SysBase *SysBase)
{
	UINT8 irqNum = exc_no - 32; // in x86 all Ints are mapped to 32 - 48
	if (irqNum > 15) return; // We only have 15 IRQs
	
	struct Interrupt *irq;
//	monitor_write("IRQ:");
//	monitor_write_hex(irqNum);
//	monitor_put(' ');

	ForeachNode(&SysBase->IntVectorList[irqNum], irq)
	{
		//if (irqNum != 0) DPrintF("SysBase: %x Data: %x\n", SysBase, irq->is_Data);
		if (irq->is_Code(irqNum, irq->is_Data, SysBase)) 
		{
			irq->is_Count++;
			break;
		}
	}
	arch_irq_eoi(1<<irqNum);	// Send an ACK to PIC
	return;	
}

