#include "types.h"
#include "sysbase.h"
#include "exec_funcs.h"
#include "_debug.h"

static void lib_int_RawDoFmtNumber(UINT32 ul, int base, void (*PutCh)(INT32, APTR), APTR PutChData, int pad0flag, int width)
{
	// hold a long in base 2
	char *p, buf[(sizeof(long) * 8) + 1];
	int i;

	p = buf;

	do {
		*p++ = "0123456789abcdef"[ul % base];
	} while (ul /= base);

	if(p-buf < width)
	{
		if(pad0flag == 1)
		{
			for(i=0; i < width - (p-buf); i++)
				PutCh('0', PutChData);
		}
		else
		{
			for(i=0; i < width - (p-buf); i++)
				PutCh(' ', PutChData);
		}

	}

	do {
		PutCh(*--p, PutChData);
	} while (p > buf);
}

va_list lib_RawDoFmt(struct SysBase *SysBase, const char *fmt, va_list ap, void (*PutCh)(INT32, APTR), APTR PutChData)
{
	char *p;
	int ch;
	unsigned long ul;

	int lflag;

	int pad0flag;
	int wflag;
	int width;
	int pflag;
	int precision;
	int total_chars;
	int chars_to_print;
	int pad_spaces;

	if(fmt == NULL || ap == NULL || PutCh == NULL) return ap;

	for (;;) {
		while ((ch = *fmt++) != '%') {
			if (ch == '\0')
			{
				PutCh(ch, PutChData); // report the \0
				return ap;
			}
			PutCh(ch, PutChData);
		}
		lflag = 0;

		pad0flag = 0;
		wflag = 0;
		width = 0;
		pflag = 0;
		precision = 0;

reswitch:

	switch (ch = *fmt++) {
		case '\0':
			PutCh(ch, PutChData); // report the \0
			return ap;

		case '%':
			PutCh(ch, PutChData); // report one % character
			break;

		case 'l':
			lflag = 1;
			goto reswitch;

		case 'c':
			ch = va_arg(ap, int);
			PutCh(ch & 0xff, PutChData);
			break;

		case 's':
			p = va_arg(ap, char *);
			if(p == NULL)
				break;

			if(pflag == 0 && wflag == 0)
			{
				while ((ch = *p++) != '\0')
					PutCh(ch, PutChData);
			}
			else
			{
				char *q = p;
				total_chars = 0;
				chars_to_print = 0;
				pad_spaces = 0;

				//calculate string length
				while ((ch = *q++) != '\0') total_chars++;

				//count max chars to print
				if(pflag == 1)
				{
					chars_to_print = (precision < total_chars) ? precision : total_chars;
				}
				else
				{
					chars_to_print = total_chars;
				}

				//count spaces to pad
				if(wflag == 1)
				{
					pad_spaces = (width > chars_to_print) ? (width - chars_to_print) : 0;
				}
				else
				{
					pad_spaces = 0;
				}

				//print spaces
				while (pad_spaces-- > 0)
					PutCh(' ', PutChData);

				//print chars
				while ((ch = *p++) != '\0' && chars_to_print-- > 0)
					PutCh(ch, PutChData);
			}
			break;

		case 'd':
			ul = lflag ? va_arg(ap, long) : va_arg(ap, int);
			if ((long)ul < 0) {
				PutCh('-', PutChData);
				ul = -(long)ul;
			}
			lib_int_RawDoFmtNumber(ul, 10, PutCh, PutChData, pad0flag, width);
			break;

		case 'i':
			ul = lflag ? va_arg(ap, long) : va_arg(ap, int);
			if ((long)ul < 0)
			{
				PutCh('-', PutChData);
				ul = -(long)ul;
			}
			else
			{
				PutCh('+', PutChData);
			}
			lib_int_RawDoFmtNumber(ul, 10, PutCh, PutChData, pad0flag, width);
			break;

		case 'o':
			PutCh('0', PutChData);
			PutCh('o', PutChData);
			ul = va_arg(ap, UINT32);
			lib_int_RawDoFmtNumber(ul, 8, PutCh, PutChData, pad0flag, width);
			break;

		case 'b':
			PutCh('0', PutChData);
			PutCh('b', PutChData);
			ul = va_arg(ap, UINT32);
			lib_int_RawDoFmtNumber(ul, 2, PutCh, PutChData, pad0flag, width);
			break;

		case 'u':
			ul = va_arg(ap, UINT32);
			lib_int_RawDoFmtNumber(ul, 10, PutCh, PutChData, pad0flag, width);
			break;

		case 'p':
			PutCh('0', PutChData);
			PutCh('x', PutChData);

			ul = va_arg(ap, UINT32);
			lib_int_RawDoFmtNumber(ul, 16, PutCh, PutChData, 1, 8);
			break;

		case 'x':
			PutCh('0', PutChData);
			PutCh('x', PutChData);

			ul = va_arg(ap, UINT32);
			lib_int_RawDoFmtNumber(ul, 16, PutCh, PutChData, pad0flag, width);
			break;

		case '0':
			if(pad0flag == 0 && wflag == 0 && pflag == 0)
			{
				pad0flag = 1;
				goto reswitch;
			}
			else
			{
				//fall through
			}

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

			if(pflag == 0)
			{
				wflag = 1;
				if(width != 0)
				{
					width = width*10 + (ch - '0');
				}
				else
				{
					width = ch - '0';
				}
			}
			else
			{
				pflag = 1;
				if(precision != 0)
				{
					precision = precision*10 + (ch - '0');
				}
				else
				{
					precision = ch - '0';
				}

			}
			goto reswitch;

		case '.':
			pflag = 1;
			goto reswitch;

		case '*':
			if(wflag == 0)
			{
				wflag = 1;
				width = va_arg(ap, int);
				if(width < 0)
					width = -width;
				goto reswitch;
			}
			if(pflag == 1)
			{
				precision = va_arg(ap, int);
				if(precision < 0)
					precision = -precision;
				goto reswitch;
			}

		default:
			PutCh('%', PutChData);
			if (lflag)
				PutCh('l', PutChData);
			PutCh(ch, PutChData);
		}
	}
	return ap;
}

