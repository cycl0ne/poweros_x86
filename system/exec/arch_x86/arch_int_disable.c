/*
** arch_DisableInt()
**
**
*/

#include "types.h"

__attribute__((no_instrument_function)) UINT32 arch_Int_Disable(void)
{
	UINT32 v;
	asm volatile (
		"pushf\n"
		"popl %[v]\n"
		"cli\n"
		: [v] "=r" (v)
	);
	return v;
}

