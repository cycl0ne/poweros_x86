#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "arch_config.h"
#include "exec_funcs.h"

void arch_irq_mask(UINT16 irqmask);
void arch_irq_unmask(UINT16 irqmask);
__attribute__((no_instrument_function))  void arch_irq_server(unsigned int exc_no, registers_t regs, APTR Data, SysBase *SysBase);

/* This is a simple string array. It contains the message that
*  corresponds to each and every exception. We get the correct
*  message by accessing like:
*  exception_message[interrupt_number] */
char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

/* All of our Exception handling Interrupt Service Routines will
*  point to this function. This will tell us what exception has
*  happened! Right now, we simply halt the system by hitting an
*  endless loop. All ISRs disable interrupts while they are being
*  serviced as a 'locking' mechanism to prevent an IRQ from
*  happening and messing up kernel data structures */
static void fault_handler(unsigned int exc_no, registers_t regs, APTR Data, SysBase *SysBase)
{
    if (exc_no < 32)
    {
		monitor_write(exception_messages[exc_no]);
		monitor_write("[Exception]. System Halted!\n");
		for (;;)asm volatile("hlt");
    }
}

static void null_handler(unsigned int exc_no, registers_t regs, APTR Data, SysBase *SysBase)
{
		monitor_write("[Exception]. NULL!\n");
		for (;;)asm volatile("hlt");
}

void arch_irq_mask(UINT16 irqmask);
#define IRQ_PIC1      2
#define IRQ_PIC_SPUR  7

void arch_irq_create(SysBase *SysBase)
{
	struct Interrupt *irq = AllocVec(sizeof(struct Interrupt) * 64, MEMF_FAST|MEMF_CLEAR);
	DPrintF("irq: %x\n", irq);
	for (int i = 0; i< 32; i++)
	{
		//irq = CreateIntServer("System Exception Handler", -10, fault_handler, SysBase);
		irq[i].is_Node.ln_Name 	= "System Exception Handler";
		irq[i].is_Node.ln_Pri	= -10;
		irq[i].is_Node.ln_Type	= NT_INTERRUPT;
		irq[i].is_Code			= (APTR)fault_handler;
		irq[i].is_Data			= SysBase;
//		monitor_write("Address:");
//		monitor_write_hex((UINT32)irq);
//		monitor_put('\n');
//		monitor_write_hex((UINT32)&SysBase->ExcVector[i]);
//		monitor_put('\n');
//		SysBase->ExcVector[i] = &irq[i];
	}

	for (int i = 32; i< 48; i++)
	{
		irq[i].is_Node.ln_Name 	= "System IRQ Server";
		irq[i].is_Node.ln_Pri	= -10;
		irq[i].is_Node.ln_Type	= NT_INTERRUPT;
		irq[i].is_Code			= (APTR)arch_irq_server;
		irq[i].is_Data			= SysBase;
//		irq = CreateIntServer("System IRQ Server", -10, arch_irq_server, SysBase);
//		SysBase->ExcVector[i] = &irq[i];
		arch_irq_mask(1<< (i-32));
	}

	for (int i = 48; i< 64; i++)
	{
		irq[i].is_Node.ln_Name 	= "System Exception Handler";
		irq[i].is_Node.ln_Pri	= -10;
		irq[i].is_Node.ln_Type	= NT_INTERRUPT;
		irq[i].is_Code			= (APTR)null_handler;
		irq[i].is_Data			= SysBase;
//		irq = CreateIntServer("System Exception Handler", -10, null_handler, SysBase);
//		SysBase->ExcVector[i] = &irq[i];		
	}
	arch_irq_mask(0xffff);
	arch_irq_unmask(1<<IRQ_PIC1);
}
