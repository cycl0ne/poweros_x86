#include "types.h"
#include "resident.h"
#include "sysbase.h"

#include "exec_funcs.h"
#include "_debug.h"
/*
* This Task initialises everything after ColdStart, where Multitasking is activated.
* this is why we use a Task here and leave Kernel Init.
*/

void lib_ExecTask(struct SysBase *SysBase)
{
	InitResidentCode(RTF_COLDSTART);
	for(int i =0; i<0xfffffff; i++);
	DPrintF("[EXECTASK] Finished, we are leaving... bye bye... till next reboot\n");
}

