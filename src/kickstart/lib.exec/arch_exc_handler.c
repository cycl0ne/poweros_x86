#include "types.h"
#include "sysbase.h"
#include "arch_config.h"
#include "_debug.h"

extern SysBase *g_SysBase;

__attribute__((no_instrument_function)) void arch_exc_handler(registers_t regs)//UINT32 number, istate_t *istate)
{
	UINT8 number = regs.int_no;
	if (number < 0 && number > 64) return;
	SysBase *SysBase = g_SysBase;
	struct Interrupt *exc = SysBase->ExcVector[number];
	if (exc)
	{
		exc->is_Code(number, regs, exc->is_Data, g_SysBase);
		exc->is_Count++;
	}
}
