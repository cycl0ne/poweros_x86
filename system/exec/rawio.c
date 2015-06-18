/**
 * @file rawio.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"
#include "asm.h"


#define SERIAL_PORT_B (0x3F8)
#define SERIAL_PORT_A (0x2F8)
#define SERIAL_PORT_C (0x3E8)
#define SERIAL_PORT_D (0x2E8)

typedef struct
{
	INT32 lflag;
	INT32 llflag;

	INT32 align_left;

	INT32 pad0flag;

	INT32 wflag;
	INT32 width;

	INT32 pflag;
	INT32 precision;

} format_flags;

void arch_RawIOInit()
{
	outb(SERIAL_PORT_A + 1, 0x00);
	outb(SERIAL_PORT_A + 3, 0x80); /* Enable divisor mode */
	outb(SERIAL_PORT_A + 0, 0x03); /* Div Low:  03 Set the port to 38400 bps */
	outb(SERIAL_PORT_A + 1, 0x00); /* Div High: 00 */
	outb(SERIAL_PORT_A + 3, 0x03);
	outb(SERIAL_PORT_A + 2, 0xC7);
	outb(SERIAL_PORT_A + 4, 0x0B);
}

void arch_RawPutChar(UINT8 chr)
{
	while ((inb(SERIAL_PORT_A + 5) & 0x20) == 0);
	outb(SERIAL_PORT_A, chr);
}

UINT8 arch_RawMayGetChar()
{
	return inb(SERIAL_PORT_A);
}

INT32 lib_RawMayGetChar(struct SysBase *SysBase)
{
	return arch_RawMayGetChar();
}

void lib_RawPutChar(struct SysBase *SysBase, UINT8 chr)
{
	arch_RawPutChar(chr);
}

void lib_RawIOInit(struct SysBase *SysBase)
{
	arch_RawIOInit();
}

INT32 nlz64(UINT64 x) {
   INT32 n;

   if (x == 0) return(64);
   n = 0;
   if (x <= 0x00000000FFFFFFFF) {n = n + 32; x = x << 32;}
   if (x <= 0x0000FFFFFFFFFFFF) {n = n + 16; x = x << 16;}
   if (x <= 0x00FFFFFFFFFFFFFF) {n = n +  8; x = x <<  8;}
   if (x <= 0x0FFFFFFFFFFFFFFF) {n = n +  4; x = x <<  4;}
   if (x <= 0x3FFFFFFFFFFFFFFF) {n = n +  2; x = x <<  2;}
   if (x <= 0x7FFFFFFFFFFFFFFF) {n = n +  1;}
   return n;
}

UINT32 DIVU(UINT64 dividend, UINT32 divisor)
{
	UINT32 quotient;
	UINT32 d0, d1;
	UINT32 remainder;

	d1 = dividend >> 32;          // Break dividend into two
    d0 = dividend & 0xFFFFFFFF;   // halves.

	__asm__ volatile (
	"divl %4"
	: "=a" (quotient), "=d" (remainder)
	: "a" (d0), "d" (d1), "g" (divisor)
	: "cc", "memory"
	);

	return quotient;
}

UINT64 udiv64(UINT64 *dividend, UINT64 divisor)
{
	UINT64 quotient = 0;
	INT32 zero=0;

	if(divisor == 0)
	{
		return 1/zero;
	}
	if(*dividend == 0)
	{
		return 0;
	}


	if(*dividend == divisor)
	{
		*dividend = 0;
		return 1;
	}
	else if(*dividend > divisor)
	{
		//go below
	}
	else if(*dividend < divisor)
	{
		*dividend = *dividend;
		return 0;
	}


	UINT64 u = *dividend;
	UINT64 v = divisor;
	UINT64 u0, u1, v1, q0, q1, k, n;

	if (v >> 32 == 0) // If v < 2**32:
	{
		if (u >> 32 < v) // If u/v cannot overflow,
		{
			quotient = DIVU( u, v & 0xFFFFFFFF ) & 0xFFFFFFFF; // just do one division.
		}
		else // If u/v would overflow:
		{
			u1 = u >> 32; // Break u up into two
			u0 = u & 0xFFFFFFFF; // halves.

			q1 = DIVU( u1, v & 0xFFFFFFFF ) & 0xFFFFFFFF; // First quotient digit.

			k = u1 - q1*v; // First remainder, < v.
			q0 = DIVU( (k << 32) + u0, v & 0xFFFFFFFF ) & 0xFFFFFFFF; // 2nd quot. digit.

			quotient = (q1 << 32) + q0;
		}
	}
	else // Here v >= 2**32.
	{
		n = nlz64(v); // 0 <= n <= 31.

		v1 = (v << n) >> 32; // Normalize the divisor, so its MSB is 1.
		u1 = u >> 1; // To ensure no overflow.

		q1 = DIVU( u1, v1 & 0xFFFFFFFF ) & 0xFFFFFFFF; // Get quotient from unsigned div instruction.

		q0 = (q1 << n) >> 31; // Undo normalization and division of u by 2.

		if (q0 != 0) // Make q0 correct or small by 1.
			q0 = q0 - 1;
		if ((u - q0*v) >= v) // After this, q0 is correct.
			q0 = q0 + 1;

		quotient = q0;
	}

	*dividend = (u - quotient*v);

	return quotient;
}

//not used now
INT64 sdiv64(INT64 *dividend, INT64 divisor)
{
	INT64 quotient = 0;
	INT32 zero=0;

	if(divisor == 0)
	{
		return 1/zero;
	}
	if(*dividend == 0)
	{
		return 0;
	}


	if(*dividend < 0 && divisor > 0)
	{
		*dividend = -(*dividend);
		divisor = divisor;
		quotient = udiv64((UINT64 *)dividend, divisor);
		*dividend = -(*dividend);
		quotient = -(quotient);
		return quotient;
	}
	else if(*dividend > 0 && divisor < 0)
	{
		*dividend = *dividend;
		divisor = -(divisor);
		quotient = udiv64((UINT64 *)dividend, divisor);
		*dividend = *dividend;
		quotient = -(quotient);
		if((INT64)*dividend == (INT64)0x8000000000000000)
		{
			return 1/zero;
		}
		return quotient;
	}
	else if(*dividend > 0 && divisor > 0)
	{
		//go below
	}
	else if(*dividend < 0 && divisor < 0)
	{
		*dividend = -(*dividend);
		divisor = -(divisor);
		quotient = udiv64((UINT64 *)dividend, divisor);
		*dividend = -(*dividend);
		quotient = quotient;
		if((INT64)quotient == (INT64)0x8000000000000000)
		{
			return 1/zero;
		}
		return quotient;
	}


	if(*dividend == divisor)
	{
		*dividend = 0;
		return 1;
	}
	else if(*dividend > divisor)
	{
		//go below
	}
	else if(*dividend < divisor)
	{
		*dividend = *dividend;
		return 0;
	}

	quotient = udiv64((UINT64 *)dividend, divisor);

	return quotient;
}


static void lib_long_RawDoFmtNumber(UINT32 ul, UINT32 base, void (*PutCh)(INT32, APTR), APTR PutChData, format_flags* flags, INT8* prefix)
{
	// hold a INT32 in base 2
	INT8 *p, buf[(sizeof(INT32) * 8) + 1];
	INT32 i;

	p = buf;
	do {
		*p++ = "0123456789abcdef"[ul % base];
	} while (ul /= base);

	INT8 *pfx, ch;

	INT32 actual_digits = 0;
	INT32 prefix_len = 0;
	INT32 zeros_to_pad = 0;
	INT32 spaces_to_pad = 0;

	//calculate length of number
	actual_digits = p-buf;

	//calculate length of prefix
	pfx = prefix;
	while ((ch = *pfx++) != '\0') prefix_len++;

	//calculate zeros to pad
	// say,
	// p = precision
	// 0 = 0 flag
	// - = left-alignment flag
	// z = how many zeros to pad
	//
	// p 0 -  z
	// = = =  =
	// 0 0 0  0
	// 0 0 1  0
	// 0 1 0  depending upon width, taking prefix length into account
	// 0 1 1  0 (- overrides 0)
	// 1 0 0  depending upon precision, without taking prefix length into account
	// 1 0 1  depending upon precision, without taking prefix length into account
	// 1 1 0  depending upon precision, without taking prefix length into account (p overrides 0)
	// 1 1 1  depending upon precision, without taking prefix length into account (- overrides 0 and p overrides 0)
	if(flags->pflag == 1)
	{
		zeros_to_pad = (flags->precision > actual_digits) ? (flags->precision - actual_digits) : 0;
	}
	else if(flags->pad0flag == 1 && flags->align_left == 0)
	{
		if(flags->wflag == 1)
		{
			zeros_to_pad = (flags->width > prefix_len+actual_digits) ? (flags->width - (prefix_len+actual_digits)) : 0;
		}
		else
		{
			zeros_to_pad = 0;
		}
	}
	else
	{
		zeros_to_pad = 0;
	}

	//calculate spaces to pad
	if(flags->wflag == 1)
	{
		spaces_to_pad = (flags->width > prefix_len+zeros_to_pad+actual_digits) ? (flags->width - (prefix_len+zeros_to_pad+actual_digits)) : 0;
	}
	else
	{
		spaces_to_pad = 0;
	}

	//now print chars based on alignment
	if(flags->align_left)
	{
		//print prefix
		pfx = prefix;
		while ((ch = *pfx++) != '\0')
			PutCh(ch, PutChData);

		//print zeros
		for(i=0; i < zeros_to_pad; i++)
			PutCh('0', PutChData);

		//print digits
		do
		{
			PutCh(*--p, PutChData);
		}
		while (p > buf);

		//print spaces
		for(i=0; i < spaces_to_pad; i++)
			PutCh(' ', PutChData);
	}
	else
	{
		//print spaces
		for(i=0; i < spaces_to_pad; i++)
			PutCh(' ', PutChData);

		//print prefix
		pfx = prefix;
		while ((ch = *pfx++) != '\0')
			PutCh(ch, PutChData);

		//print zeros
		for(i=0; i < zeros_to_pad; i++)
			PutCh('0', PutChData);

		//print digits
		do
		{
			PutCh(*--p, PutChData);
		}
		while (p > buf);
	}
}

static void lib_long_long_RawDoFmtNumber(UINT64 ull, UINT32 base, void (*PutCh)(INT32, APTR), APTR PutChData, format_flags* flags, INT8* prefix)
{
	// hold a INT64 in base 2
	INT8 *p, buf[(sizeof(INT64) * 8) + 1];
	INT32 i;
	UINT64 temp_ull;
	UINT64 quotient;
	UINT64 remainder;

	p = buf;
	do {
		temp_ull = ull;
		quotient = udiv64(&temp_ull, (UINT64)base);
		remainder = temp_ull;
		*p++ = "0123456789abcdef"[remainder];
	} while ((ull = quotient));

	INT8 *pfx, ch;

	INT32 actual_digits = 0;
	INT32 prefix_len = 0;
	INT32 zeros_to_pad = 0;
	INT32 spaces_to_pad = 0;

	//calculate length of number
	actual_digits = p-buf;

	//calculate length of prefix
	pfx = prefix;
	while ((ch = *pfx++) != '\0') prefix_len++;

	//calculate zeros to pad
	// say,
	// p = precision
	// 0 = 0 flag
	// - = left-alignment flag
	// z = how many zeros to pad
	//
	// p 0 -  z
	// = = =  =
	// 0 0 0  0
	// 0 0 1  0
	// 0 1 0  depending upon width, taking prefix length into account
	// 0 1 1  0 (- overrides 0)
	// 1 0 0  depending upon precision, without taking prefix length into account
	// 1 0 1  depending upon precision, without taking prefix length into account
	// 1 1 0  depending upon precision, without taking prefix length into account (p overrides 0)
	// 1 1 1  depending upon precision, without taking prefix length into account (- overrides 0 and p overrides 0)
	if(flags->pflag == 1)
	{
		zeros_to_pad = (flags->precision > actual_digits) ? (flags->precision - actual_digits) : 0;
	}
	else if(flags->pad0flag == 1 && flags->align_left == 0)
	{
		if(flags->wflag == 1)
		{
			zeros_to_pad = (flags->width > prefix_len+actual_digits) ? (flags->width - (prefix_len+actual_digits)) : 0;
		}
		else
		{
			zeros_to_pad = 0;
		}
	}
	else
	{
		zeros_to_pad = 0;
	}

	//calculate spaces to pad
	if(flags->wflag == 1)
	{
		spaces_to_pad = (flags->width > prefix_len+zeros_to_pad+actual_digits) ? (flags->width - (prefix_len+zeros_to_pad+actual_digits)) : 0;
	}
	else
	{
		spaces_to_pad = 0;
	}

	//now print chars based on alignment
	if(flags->align_left)
	{
		//print prefix
		pfx = prefix;
		while ((ch = *pfx++) != '\0')
			PutCh(ch, PutChData);

		//print zeros
		for(i=0; i < zeros_to_pad; i++)
			PutCh('0', PutChData);

		//print digits
		do
		{
			PutCh(*--p, PutChData);
		}
		while (p > buf);

		//print spaces
		for(i=0; i < spaces_to_pad; i++)
			PutCh(' ', PutChData);
	}
	else
	{
		//print spaces
		for(i=0; i < spaces_to_pad; i++)
			PutCh(' ', PutChData);

		//print prefix
		pfx = prefix;
		while ((ch = *pfx++) != '\0')
			PutCh(ch, PutChData);

		//print zeros
		for(i=0; i < zeros_to_pad; i++)
			PutCh('0', PutChData);

		//print digits
		do
		{
			PutCh(*--p, PutChData);
		}
		while (p > buf);
	}

}

va_list lib_RawDoFmt(struct SysBase *SysBase, const INT8 *fmt, va_list ap, void (*PutCh)(INT32, APTR), APTR PutChDataOrg)
{
	INT8 *p;
	INT32 ch;
	UINT32 ul;
	UINT64 ull;
	INT8 prefix[3];

	format_flags flags;

	APTR PutChData = &PutChDataOrg;

	//if(fmt == NULL || ap == NULL || PutCh == NULL) return ap;
	if(fmt == NULL || PutCh == NULL) return ap;

	for (;;)
	{
		while ((ch = *fmt++) != '%')
		{
			if (ch == '\0')
			{
				PutCh(ch, PutChData); // report the \0
				return ap;
			}
			PutCh(ch, PutChData);
		}

		flags.lflag = 0;
		flags.llflag = 0;

		flags.align_left = 0;

		flags.pad0flag = 0;

		flags.wflag = 0;
		flags.width = 0;

		flags.pflag = 0;
		flags.precision = 0;

		prefix[0] = '\0';

reswitch:

	switch (ch = *fmt++) {
		case '\0':
			PutCh(ch, PutChData); // report the \0
			return ap;

		case '%':
			PutCh(ch, PutChData); // report one % character
			break;

		case '-':
			flags.align_left = 1;
			goto reswitch;

		case '0':
			if(flags.pad0flag == 0 && flags.wflag == 0 && flags.pflag == 0)
			{
				flags.pad0flag = 1;
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

			if(flags.pflag == 0)
			{
				flags.wflag = 1;
				if(flags.width != 0)
				{
					flags.width = flags.width*10 + (ch - '0');
				}
				else
				{
					flags.width = ch - '0';
				}
			}
			else
			{
				flags.pflag = 1;
				if(flags.precision != 0)
				{
					flags.precision = flags.precision*10 + (ch - '0');
				}
				else
				{
					flags.precision = ch - '0';
				}

			}
			goto reswitch;

		case '.':
			flags.pflag = 1;
			goto reswitch;

		case '*':
			if(flags.wflag == 0 && flags.pflag == 0)
			{
				flags.wflag = 1;
				flags.width = va_arg(ap, INT32);
				if(flags.width < 0)
					flags.width = -flags.width;
				goto reswitch;
			}
			if(flags.pflag == 1)
			{
				flags.precision = va_arg(ap, INT32);
				if(flags.precision < 0)
					flags.precision = -flags.precision;
				goto reswitch;
			}

		case 'l':
			if(flags.lflag == 1 && flags.llflag == 0)
			{
				flags.llflag = 1;
				flags.lflag = 0;
			}
			else if(flags.lflag == 0 && flags.llflag == 0)
			{
				flags.lflag = 1;
			}

			goto reswitch;

		case 'c':
			ch = va_arg(ap, INT32);
			PutCh(ch & 0xff, PutChData);
			break;

		case 's':
			p = va_arg(ap, INT8 *);
			if(p == NULL)
				break;

			if(flags.pflag == 0 && flags.wflag == 0)
			{
				while ((ch = *p++) != '\0')
					PutCh(ch, PutChData);
			}
			else
			{
				INT8 *q = p;

				INT32 total_chars = 0;
				INT32 chars_to_print = 0;
				INT32 pad_spaces = 0;

				//calculate string length
				while ((ch = *q++) != '\0') total_chars++;

				//calculate max chars to print; precision tells this
				if(flags.pflag == 1)
				{
					chars_to_print = (flags.precision < total_chars) ? flags.precision : total_chars;
				}
				else
				{
					chars_to_print = total_chars;
				}

				//calculate spaces to pad; width tells this
				if(flags.wflag == 1)
				{
					pad_spaces = (flags.width > chars_to_print) ? (flags.width - chars_to_print) : 0;
				}
				else
				{
					pad_spaces = 0;
				}

				//now print chars based on alignment
				if(flags.align_left)
				{
					//print chars
					while ((ch = *p++) != '\0' && chars_to_print-- > 0)
						PutCh(ch, PutChData);

					//print spaces
					while (pad_spaces-- > 0)
						PutCh(' ', PutChData);
				}
				else
				{
					//print spaces
					while (pad_spaces-- > 0)
						PutCh(' ', PutChData);

					//print chars
					while ((ch = *p++) != '\0' && chars_to_print-- > 0)
						PutCh(ch, PutChData);
				}
			}
			break;

		case 'd':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				if ((UINT64)ull <= (UINT64)0x7fffffffffffffff)
				{
					//nothing, this is positive
				}
				else if((UINT64)ull == (UINT64)0x8000000000000000)
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					//ull = -(INT64)ull; //no need to negate, this value is the smallest 64bit negative number, like -128 for 8bit
				}
				else
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					ull = -(INT64)ull;
				}
				lib_long_long_RawDoFmtNumber(ull, 10, PutCh, PutChData, &flags, prefix);

			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}


				if ((UINT32)ul <= (UINT32)0x7fffffff)
				{
					//nothing, this is positive
				}
				else if((UINT32)ul == (UINT32)0x80000000)
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					//ul = -(INT32)ul; //no need to negate, this value is the smallest 32bit negative number, like -128 for 8bit
				}
				else
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					ul = -(INT32)ul;
				}
				lib_long_RawDoFmtNumber(ul, 10, PutCh, PutChData, &flags, prefix);
			}
			break;

		case 'i':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				if ((UINT64)ull <= (UINT64)0x7fffffffffffffff)
				{
					prefix[0] = '+';
					prefix[1] = '\0';
				}
				else if((UINT64)ull == (UINT64)0x8000000000000000)
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					//ull = -(INT64)ull; //no need to negate, this value is the smallest 64bit negative number, like -128 for 8bit
				}
				else
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					ull = -(INT64)ull;
				}
				lib_long_long_RawDoFmtNumber(ull, 10, PutCh, PutChData, &flags, prefix);

			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}


				if ((UINT32)ul <= (UINT32)0x7fffffff)
				{
					prefix[0] = '+';
					prefix[1] = '\0';
				}
				else if((UINT32)ul == (UINT32)0x80000000)
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					//ul = -(INT32)ul; //no need to negate, this value is the smallest 32bit negative number, like -128 for 8bit
				}
				else
				{
					prefix[0] = '-';
					prefix[1] = '\0';
					ul = -(INT32)ul;
				}
				lib_long_RawDoFmtNumber(ul, 10, PutCh, PutChData, &flags, prefix);
			}
			break;

		case 'u':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				lib_long_long_RawDoFmtNumber(ull, 10, PutCh, PutChData, &flags, prefix);
			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}
				lib_long_RawDoFmtNumber(ul, 10, PutCh, PutChData, &flags, prefix);
			}
			break;

		case 'b':
			prefix[0] = '0';
			prefix[1] = 'b';
			prefix[2] = '\0';

		case 'B':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				lib_long_long_RawDoFmtNumber(ull, 2, PutCh, PutChData, &flags, prefix);
			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}
				lib_long_RawDoFmtNumber(ul, 2, PutCh, PutChData, &flags, prefix);
			}

			break;

		case 'o':
			prefix[0] = '0';
			prefix[1] = 'o';
			prefix[2] = '\0';

		case 'O':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				lib_long_long_RawDoFmtNumber(ull, 8, PutCh, PutChData, &flags, prefix);
			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}
				lib_long_RawDoFmtNumber(ul, 8, PutCh, PutChData, &flags, prefix);
			}
			break;

		case 'x':
			prefix[0] = '0';
			prefix[1] = 'x';
			prefix[2] = '\0';

		case 'X':
			if(flags.llflag == 1)
			{
				ull = va_arg(ap, UINT64);
				lib_long_long_RawDoFmtNumber(ull, 16, PutCh, PutChData, &flags, prefix);
			}
			else
			{
				if(flags.lflag == 1)
				{
					ul = va_arg(ap, UINT32);
				}
				else
				{
					ul = va_arg(ap, UINT32);
				}
				lib_long_RawDoFmtNumber(ul, 16, PutCh, PutChData, &flags, prefix);
			}
			break;

		case 'p':
			prefix[0] = '0';
			prefix[1] = 'x';
			prefix[2] = '\0';

			ul = va_arg(ap, UINT32);
			flags.pad0flag = 1;
			flags.width = 8;
			lib_long_RawDoFmtNumber(ul, 16, PutCh, PutChData, &flags, prefix);
			break;

		default:
			PutCh('%', PutChData);
			if (flags.lflag)
			{
				PutCh('l', PutChData);
			}
			if (flags.llflag)
			{
				PutCh('l', PutChData);
				PutCh('l', PutChData);
			}
			PutCh(ch, PutChData);
		}
	}
	return ap;
}


