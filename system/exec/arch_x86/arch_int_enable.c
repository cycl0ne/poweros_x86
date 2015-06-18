/*
** arch_EnableInt()
**
**
*/

#include "types.h"

__attribute__((no_instrument_function)) UINT32 arch_Int_Enable(void)
{
	UINT32 v;
	asm volatile (
		"pushf\n"
		"popl %[v]\n"
		"sti\n"
		: [v] "=r" (v)
	);

	return v;
}


