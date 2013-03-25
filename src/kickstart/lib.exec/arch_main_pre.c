#include "types.h"
#include "_debug.h"

// Initialisation Routines before the main_bsp() is called.
// It gets the Multiboot Informations from Grub
void gdt_init();
void arch_exc_init();

//void arch_main_pre(UINT32 signature, void *info)
int arch_main_pre(UINT32 *mboot_ptr)
{
	monitor_clear();
	gdt_init();
	//monitor_write("[premain] gdtinit()\n");
	arch_exc_init();
	//monitor_write("[premain] ExcInit()\n");
}

