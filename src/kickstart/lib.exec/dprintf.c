#include "types.h"
#include "sysbase.h"
#include "_debug.h"
#include "context.h"
#include "arch_config.h"
#include "exec_funcs.h"

#if 0
static void printit(INT32 chr, APTR ptr)
{
	monitor_put(chr);
}
#endif

static void raw_printit(INT32 chr, APTR SysBase)
{
	RawPutChar(chr);
}

BOOL enabled = FALSE;

void lib_DPrintF(struct SysBase *SysBase, char *fmt, ...)
{
	if (!enabled) {RawIOInit(); enabled = TRUE;}
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt(fmt, pvar, raw_printit, SysBase);
//	RawDoFmt(fmt, pvar, printit, NULL);
	va_end(pvar);
}
