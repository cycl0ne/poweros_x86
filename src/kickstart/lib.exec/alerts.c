#include "types.h"
#include "list.h"
#include "ports.h"
#include "sysbase.h"
#include "_debug.h"
#include "alerts.h"
#include "exec_funcs.h"
#include "arch_config.h"

#define BANNER_LEFT   "####>"
#define BANNER_RIGHT  "<####"

static void printit(INT32 chr, APTR ptr)
{
	monitor_put(chr);
}

#define AT_Assertion (1<<30)

void lib_Alert(SysBase *SysBase, UINT32 alertNum, const char *fmt, ...)
{
	struct Task *task;
	Disable();
	DPrintF("\n%s Guru Meditation (#%x) ", BANNER_LEFT, alertNum);
	if (task = FindTask(NULL))
		DPrintF("on Task [%s] ", task->Node.ln_Name);
	DPrintF("due to ");

	va_list args;
	va_start(args, fmt);
	if (alertNum & AT_Assertion) {
		DPrintF("a failed assertion: %s\n", BANNER_RIGHT);
		RawDoFmt(fmt, args, printit, NULL);
		DPrintF("\n");
	}

	if (alertNum & AT_DeadEnd) {
		DPrintF("DeadEnd: %s\n", BANNER_RIGHT);
		RawDoFmt(fmt, args, printit, NULL);
		DPrintF("\n");
	}

	va_end(args);
	
	DPrintF("\n");
	while(1) arch_Halt();
}

#if 0
void lib_Alert(SysBase *SysBase, UINT32 alertNum)
{
    struct Task *task = SysBase->thisTask;
	
    if (alertNum & AT_DeadEnd)
    {
		DPrintF("DEADEND LOOP\n");
		//DEADEND RESET NORMALY ;)
		for(;;);
	}
}
#endif
