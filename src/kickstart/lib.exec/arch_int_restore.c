/*
** arch_EnableInt()
**
**
*/

#include "types.h"

__attribute__((no_instrument_function)) void arch_Int_Restore(UINT32 ipl)
{
	asm volatile (
		"pushl %[ipl]\n"
		"popf\n"
		:: [ipl] "r" (ipl)
	);
}
