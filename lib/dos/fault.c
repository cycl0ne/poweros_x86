/**
 * @file fault.c
 *
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

static void _CpyString(UINT8 **dest, CONST_STRPTR src, UINT32 *len)
{
	while (*len && *src)
	{
		*((*dest)++) = *src++;
		(*len)--;
	}
	**dest = '\0';
}

// due to a bug in RAWDOFMT this looks ugly :( FIX
static void _prbuf(INT32 ch, INT32 *str)
{
	UINT8* buffer = (UINT8*)str[0];
	*buffer++ = (char)ch;
	str[0] = (INT32)buffer;
}

static int _sprintf(pDOSBase DOSBase, char* str, char *fmt,...)
{
	INT32	args[1];
	args[0] = (INT32) str;

	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) args);
	return Strlen((STRPTR)str);
}

char *dos_GetString(pDOSBase DOSBase, INT32 code)
{
	struct ErrorString *es;
	UINT8 *ptr;
	INT32 curr,*entry,end;

	es    = DOSBase->dos_Errors;
	ptr   = es->estr_Strings;
	entry = es->estr_Nums;

	do {
		end = *(entry+1);
		for (curr = *entry; curr <= end; curr++)
		{
			if (curr == code) return (char *) ptr+1;
			ptr = ptr + 1 + *ptr;
		}
		entry += 2;
	} while (*entry);
	return NULL;
}

DOSIO dos_Fault(pDOSBase DOSBase, INT32 code, CONST_STRPTR header, STRPTR buffer, INT32 len)
{
	STRPTR	string;
	UINT32	len2 = len;
	UINT8*	buffer2 = (UINT8*)buffer;

	if (code)
	{
		if (header)
		{
			_CpyString(&buffer2, header, &len2);
			_CpyString(&buffer2, ": ", &len2);
		}
		string = GetString(code);
		if (string) _CpyString(&buffer2, string, &len2);
		else
		{
			char number[16];
			_sprintf(DOSBase, number, "%ld", code);
			_CpyString(&buffer2, GetString( STR_ERROR), &len2);
			_CpyString(&buffer2, number, &len2);
		}
		return len-len2;
	}
	*buffer = '\0';
	return DOSIO_FALSE;
}

DOSIO dos_PrintFault(pDOSBase DOSBase, INT32 code, STRPTR header)
{
	STRPTR	string;
	INT32 rc = DOSIO_TRUE;

	if (code)
	{
		if (header) 
		{
			Printf(header);
			Printf(" ");
		}
		string = GetString( code);
		if (string) 
		{
			Printf(string);
		} else {
			Printf(GetString( STR_ERROR));
			Printf("%d", code);
			rc = DOSIO_FALSE;
		}
		Printf("\n");
		SetIoErr(code);
	}
	return rc;
}

