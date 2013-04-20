#include "types.h"
#include "resident.h"
#include "sysbase.h"
#include "arch_config.h"

#include "exec_funcs.h"
#include "_debug.h"
/*
* This Task initialises everything after ColdStart, where Multitasking is activated.
* this is why we use a Task here and leave Kernel Init.
*/
void arch_irq_init();
void arch_irq_create(SysBase *SysBase);
void arch_clk_init(SysBase *SysBase);
extern arch_config config;

void lib_ExecTask(struct SysBase *SysBase)
{
	InitResidentCode(RTF_COLDSTART);
	//for(int i =0; i<0xfffffff; i++);
	//DPrintF("[EXECTASK] Finished, we are leaving... bye bye... till next reboot\n");
}

