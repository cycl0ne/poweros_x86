#include "types.h"
#include "_debug.h"

// Initialisation Routines before the main_bsp() is called.
// It gets the Multiboot Informations from Grub
void gdt_init();
void arch_exc_init();

//void arch_main_pre(UINT32 signature, void *info)
void arch_main_pre(UINT32 *mboot_ptr)
{
	monitor_clear();
//	monitor_write("[premain] gdtinit()\n");
	gdt_init();
	arch_exc_init();
	//monitor_write("[premain] ExcInit()\n");
}

