#include "types.h"
#include "list.h"
#include "irq.h"
#include "sysbase.h"
#include "exec_funcs.h"
#include "_debug.h"

//extern SysBase *g_SysBase;

void arch_irq_unmask(UINT16 irqmask);
void arch_irq_mask(UINT16 irqmask);

void lib_AddIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr)
{
	if (NULL==isr) return;
	if (intNumber < 0 && intNumber > 16) return;

	UINT32 ipl;
	UINT32 empty = IsListEmpty(&SysBase->IntVectorList[intNumber]);

	isr->is_Cycles = 0;
	isr->is_Count = 0;

	ipl = Disable();
	Enqueue(&SysBase->IntVectorList[intNumber], (struct Node *)isr);
	Enable(ipl);
	
	// Unmask IRQ
	DPrintF("empty: %d %d\n", intNumber, empty);
	if (empty) arch_irq_unmask(1 << intNumber);
}

void lib_RemIntServer(SysBase *SysBase, UINT32 intNumber, struct Interrupt *isr)
{
	UINT32 ipl;
	if (NULL==isr) return;
	if (intNumber < 0 && intNumber > 16) return;
	ipl = Disable();
	Remove((struct Node *)isr);
	Enable(ipl);
	if (IsListEmpty(&SysBase->IntVectorList[intNumber]))
	{
		arch_irq_mask(1 << intNumber);
	}
}

struct Interrupt *lib_SetExcVector(SysBase *SysBase, UINT32 excNumber, struct Interrupt *isr)
{
	UINT32 ipl;
	if (excNumber>32) return NULL;

	struct Interrupt *oldisr = SysBase->ExcVector[excNumber];
	if (NULL==isr) 
	{
		SysBase->ExcVector[excNumber] = NULL;
		return oldisr;
	}

	isr->is_Cycles = 0;
	isr->is_Count = 0;
	ipl = Disable();
	SysBase->ExcVector[excNumber] = isr;
	Enable(ipl);
	return oldisr;	
}

struct Interrupt *lib_CreateIntServer(SysBase *SysBase, const STRPTR name, INT8 pri, APTR handler, APTR data)
{
	struct Interrupt *irq;
	irq = AllocVec(sizeof(struct Interrupt), MEMF_FAST);	
	if (irq) {	
		irq->is_Node.ln_Name = name;
		irq->is_Node.ln_Pri = pri;
		irq->is_Node.ln_Type = NT_INTERRUPT;
		irq->is_Code = handler;
		irq->is_Data = data;
		return irq; 
	}
	return NULL;
}

