#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "context.h"
#include "arch_config.h"
#include "exec_funcs.h"

/** Main kernel routine.
 *
 * Assuming interrupts_disable().
 *
 */

#define FADDR(fptr)  ((APTR) (fptr))
extern UINT32 *__code;
extern UINT32 *__end;


void ExecInit(void);

arch_config config;
context_t ctx;

// Our Jump Code to the real Code
__attribute__((no_instrument_function)) void main_bsp(void)
{
	config.cpu_count = 1;
	config.cpu_active = 1;
	config.base = (UINT32 *)&__code;
	config.kernel_size = (UINT32)&__end - (UINT32)&__code;//hardcoded_rom_size;
	config.stack_size = 4096;
	config.stack_base = (UINT32*) (&__end + config.stack_size);
	config.arch_name = "PowerOS x86";

	config.memory_base = (UINT32 *)0x200000;//config.stack_base;
	config.memory_size = (UINT32  )0x2000000;
	config.memory_prio = 0;
	config.memory_attribute = MEMF_FAST|MEMF_PUBLIC;
	config.memory_name = "Fast Memory\n";

#if 0
	monitor_write("[main_bsp] ExecInit\n");
	monitor_write_hex((UINT32)config.base);
	monitor_put('\n');
	monitor_write_hex((UINT32)config.kernel_size);
	monitor_put('\n');
	monitor_write_hex((UINT32)config.stack_base);
	monitor_put('\n');
#endif
	
	context_save(&ctx);
	context_set(&ctx, FADDR(ExecInit), config.stack_base, 4096);
	context_restore(&ctx);
}

