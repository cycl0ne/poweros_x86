/*
** arch_int_read()
**
**
*/

#include "types.h"

__attribute__((no_instrument_function)) UINT32 arch_Int_Read(void)
{
	UINT32 v;
	
	asm volatile (
		"pushf\n"
		"popl %[v]\n"
		: [v] "=r" (v)
	);
	
	return v;
}