#include "execbase_private.h"
#include "exec_interface.h"

#include "arch_config.h"
#include "_debug.h"

// This Code is for Interrupt Handling.
// this code is injected from the Exception Handler (arch_exc_server)
void arch_irq_eoi(UINT16 irqmask);
#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }

__attribute__((no_instrument_function))  void arch_irq_server(unsigned int exc_no, registers_t regs, APTR Data, SysBase *SysBase)
{
	(void) Data;
	(void) regs;
//	IRQ_OFF;
	UINT8 irqNum = exc_no - 32; // in x86 all Ints are mapped to 32 - 48
	if (irqNum > 15) return; // We only have 15 IRQs
	SysBase->SysFlags |= SYSFLG_NOSCHEDULE;  //No Schedule when we are in an IRQ
	
	struct Interrupt *irq;
	//	monitor_write("IRQsrv:");
	//	monitor_write_hex((UINT32)SysBase->SysFlags);
	//	monitor_write_hex((UINT32)irqNum);
	//	monitor_put(' ');

	// This is very critical! Reschedule() needs this BEFORE Reschedule() called, all other IRQs 
	// need it AFTER the IRQ Handler is called.
	//	arch_irq_eoi(1<<irqNum);

	ForeachNode(&SysBase->IntVectorList[irqNum], irq)
	{
		if (irq->is_Code(irqNum, irq->is_Data, SysBase)) 
		{
			irq->is_Count++;
			break;
		}
	}

	arch_irq_eoi(1<<irqNum);
	SysBase->SysFlags &= ~SYSFLG_NOSCHEDULE;  //Now we can Schedule again we left IRQ Handler
//	IRQ_RES;
	// This is now our Reschedule point. No more own "handler", but now more often.
	if (irqNum == 8)
	{
		//monitor_write_hex((UINT32)irqNum);
		if (!(IsListEmpty(&SysBase->TaskReady))) Reschedule();
	} else
	{
		// We missed a Reschedule and are not on the Reschedule() IRQ? No problem, then call Reschedule here.
		if (SysBase->SysFlags & SYSFLG_RESCHEDULE)
		{
			SysBase->SysFlags &= ~SYSFLG_RESCHEDULE;
			if (!(IsListEmpty(&SysBase->TaskReady))) Reschedule();
		}
	}
	return;	
}

