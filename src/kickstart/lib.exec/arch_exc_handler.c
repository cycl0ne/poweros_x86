#include "types.h"
#include "sysbase.h"
#include "arch_config.h"
#include "_debug.h"
#include "exec_funcs.h"

extern SysBase *g_SysBase;

__attribute__((no_instrument_function)) void arch_exc_handler(registers_t regs)//UINT32 number, istate_t *istate)
{
//	monitor_write("IRQ:");
//	monitor_write_hex(regs.int_no);
//	monitor_put(' ');
	SysBase *SysBase = g_SysBase;
//	DPrintF("IRQ %x\n", g_SysBase);

	UINT8 number = regs.int_no;
	if (number < 0 && number > 64) return;
	struct Interrupt *exc = SysBase->ExcVector[number];
	//	DPrintF("exc: %x\n", exc);
	if (exc)
	{
	//	DPrintF("exccode: %x\n", exc->is_Code);
		exc->is_Code(number, regs, exc->is_Data, g_SysBase);
		exc->is_Count++;
	}
}
