#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "context.h"
#include "arch_config.h"
#include "exec_funcs.h"

static void printit(INT32 chr, APTR ptr)
{
	monitor_put(chr);
}

void lib_DPrintF(struct SysBase *SysBase, char *fmt, ...)
{
	//RawDoFmt(FormatString, DataStream, PutChProc, PutChData);
	Disable();
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt(fmt, pvar, printit, NULL);
	va_end(pvar);
	Enable(0);
}
