/**
 * @file debug.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"
#include "_debug.h"

/*
** New DebugPrintf
*/


static void dbgprintit(int32_t chr, pSysBase SysBase)
{
	SysBase->DBG_Log[SysBase->DBG_Cnt++] = (uint8_t)chr;
}

void lib_DBGPrintF(pSysBase SysBase, uint8_t *err, char *fmt, ...)
{
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt(fmt, pvar, dbgprintit, SysBase);
	va_end(pvar);
	SysBase->DBG_Log[SysBase->DBG_Cnt] = '\0';
}

static void raw_printit(INT32 chr, int32_t **arg)
{
	INT32 *args = *arg;
	pSysBase SysBase = (pSysBase)args[0];
	//RawPutChar(chr);
	monitor_put((char)chr);
//	*(*str)++= (char)chr;
	if (SysBase->DBG_Cnt > DBGLOGSIZE-1) SysBase->DBG_Cnt = 0;
	if (chr != 0x0) SysBase->DBG_Log[SysBase->DBG_Cnt++] = (uint8_t)chr;
}

BOOL enabled = FALSE;

void lib_KPrintF(struct SysBase *SysBase, char *fmt, ...)
{
	if (!enabled) {RawIOInit(); enabled = TRUE;}
	va_list pvar;
	va_start(pvar, fmt);
	INT32 args[2];
	args[0] = (INT32)SysBase;
	args[1] = (INT32)0xf;
	RawDoFmt(fmt, pvar, raw_printit, (APTR)args);
	va_end(pvar);
	SysBase->DBG_Log[SysBase->DBG_Cnt] = '\0';
}

