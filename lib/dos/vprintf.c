/**
 * @file vprintf.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

static STRPTR writeNumber(pDOSBase DOSBase, char *buffer, UINT32 base, UINT32 n, BOOL minus)
{
    int val;

    do {
		val = n % base;
		*--buffer = val < 10 ? val + '0' : val - 10 + 'A';
		n /= base;
    } while(n != 0);

    if(minus) *--buffer = '-';
    return buffer;
}

static INT32 putNumber(pDOSBase DOSBase, pFileHandle fh,  CONST_STRPTR *format, IPTR **args, UINT32 base)
{
    char    buffer[sizeof(UINT32)*8/3+2];
    INT32    icount = 0;
    INT32    number;
    INT32    len;		/* Maximum width of number (ASCII) */
    BOOL    minus = FALSE;
    STRPTR  aNum;
    INT32    i;			/* Loop variable */

    buffer[sizeof(UINT32)*8/3+1] = 0;

    (*format)++;
    len = **format - '0';
    
    number = **args;
    (*args)++;

    if(number < 0)
    {
		number = -number;
		minus = TRUE;
    }
    
    aNum = writeNumber(DOSBase, &buffer[sizeof(UINT32)*8/3+1], base, number, minus);
    
    for(i = 0; i < len; i++)
    {
		FPutC(fh, *aNum++);
		icount++;
		if(*aNum == 0) break;
    }

    return icount;
}   

static void _putch(INT32 ch, INT32 **arg)
{
	INT32*	args = *arg;
	APTR DOSBase = (APTR)args[4];
	if (args[1]++ && !args[3])
	{
		if (FPutC((pFileHandle)args[0],args[2]) == ENDSTREAMCH)
			args[3] = -1;
	}
	args[2] = ch & 0xff;
}

DOSIO dos_Printf(pDOSBase DOSBase, CONST_STRPTR fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	DOSIO ret = VPrintf(fmt, (INT32*)vl);
	va_end(vl);
	return ret;
}

DOSIO dos_VFPrintf(pDOSBase DOSBase, pFileHandle fh, CONST_STRPTR fmt, INT32 *argv)
{
	INT32 args[5];

	args[0] = (INT32)fh;
	args[1] = (INT32)0;	// num of bytes 
	args[2] = (INT32)0;	// last char
	args[3] = (INT32)0;	// error 
	args[4] = (INT32)DOSBase; // for FPutC
	
	RawDoFmt(fmt, (va_list)argv,(void (*)(INT32, void *)) _putch,(APTR) args);

	if (args[3]) return args[3];
	return args[1]-1;
}

DOSIO dos_VPrintf(pDOSBase DOSBase, CONST_STRPTR fmt, INT32 *argv )
{
	return VFPrintf(Output(),fmt, argv);
}

DOSIO dos_VFWritef(pDOSBase DOSBase, pFileHandle fh, CONST_STRPTR fmt, INT32 *argarray)
{
	char buffer[sizeof(UINT32)*8/3+2];
	INT32	count  = 0;
	CONST_STRPTR	format = fmt;
	IPTR	*args   = (IPTR *)argarray;
	
	STRPTR	string;
	STRPTR	wBuf;
	INT32	len;
	INT32	i;
	INT32	number;
	BOOL    minus;
	BOOL    quitNow = FALSE;

	while(*format != 0 && !quitNow)
	{
		if(*format == '%')
		{
			format++;
			switch(*format)
			{
				case 'S':		/* Regular c string */
				case 's':
				case 'T':
				case 't':
					string = (STRPTR)*args;
					args++;
					if(string == NULL) return -1;
					while(*string != 0)
					{
						FPutC(fh, *string++);
						count++;
					}
					break;

				case 'C':		/* Character */
				case 'c':
					FPutC(fh, (char)*args);
					count++;
					args++;
					break;

				case 'O':		/* Octal number */
				case 'o':
					count += putNumber(DOSBase, fh, &format, &args, 8);
					break;

				case 'X':		/* Hexadecimal number */
				case 'x':
					count += putNumber(DOSBase, fh, &format, &args, 16);
					break;

				case 'I':		/* Decimal number */
				case 'i':
					count += putNumber(DOSBase, fh, &format, &args, 10);
					break;

				case 'N':		/* Decimal number (no length restriction) */
				case 'n':
					number = *args;
					args++;

					if(number < 0)
					{
						number = -number;
						minus = TRUE;
					} else
						minus = FALSE;

					buffer[sizeof(UINT32)*8/3+1] = 0;
					wBuf = writeNumber(DOSBase, &buffer[sizeof(UINT32)*8/3+1], 10, number, minus);

					while(*wBuf != 0)
					{
						FPutC(fh, *wBuf++);
						count++;
					}
					break;

				case 'U':		/* Unsigned decimal number */
				case 'u':
					format++;
					len = *format - '0';
					number = *args;
					args++;
					wBuf = writeNumber(DOSBase, &buffer[sizeof(UINT32)*8/3+1], 10, number, FALSE);

					for(i = 0; i < len; i++)
					{
						FPutC(fh, *wBuf++);
						count++;
						if(*wBuf == 0) break;
					}
					break;

				case '$':
					args++;
					break;

				case 0:
					quitNow = TRUE;
					break;

				default:
					FPutC(fh, *format);
					count++;
					break;
			}
		} else
		{
			FPutC(fh, *format);
			count++;
		}
		format++;
	}
	return count;
}



