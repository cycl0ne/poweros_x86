/*
** arch_EnableInt()
**
**
*/

#include "types.h"

__attribute__((no_instrument_function)) void arch_Int_Restore(UINT32 ipl)
{
	asm volatile (
		"cli\n"
		"pushl %[ipl]\n"
		"popf\n"
		"sti\n"
		:: [ipl] "r" (ipl)
	);
}
