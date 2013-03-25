#include "types.h"
#include "resident.h"
#include "sysbase.h"

#include "exec_funcs.h"
#include "_debug.h"
/*
** This Task is the Idle Task :)
*/

void lib_Idle(SysBase *SysBase) 
{
	//SysBase *SysBase = g_SysBase;
	//DPrintF("[IDLE] Started\n");
	SetTaskPri(NULL, -125);

	//DPrintF("[IDLE] SetTaskpri -125\n");
	
	while(1)
	{
		asm volatile("hlt");
		// Loop, here should be the hlt() command to lower cpu temp
	}
	DPrintF("[IDLE] PONR\n");
}
