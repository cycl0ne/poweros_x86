/**
* File: /hello．c
* User: cycl0ne
* Date: 2014-10-22
* Time: 08:27 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define TEMPLATE    "HELLOWORLD"
#define OPT_COUNT   0

#if 0
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase = OpenLibrary("dos.library", 0);
	UINT32 		opts[OPT_COUNT];
	struct RDargs *rdargs;

	INT32 			rc = RETURN_FAIL, rc2 = 0;

	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);
	if (rdargs == NULL) PrintFault(IoErr(), NULL);
	else
	{
		rc = RETURN_OK;
		Printf("Hello World\n");
	}
	CloseLibrary(DOSBase);
	return rc;
}
#endif

// More simpler Version:
static INT32 add(INT32 a, INT32 b);

#if 1
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase = OpenLibrary("dos.library", 0);

	INT32 k;
	k = add(6, 7);
	//Printf("Answer to addition = %d\n", k);

	Printf("Hello World\n");

/*
	INT32 a = 9;
	INT32 w = 6;
	INT32 p = 3;
	Printf("a=[%O]\n", a);
	Printf("a=[%0O]\n", a);
	Printf("a=[%-O]\n", a);
	Printf("\n");
	Printf("a=[%06O]\n", a);
	Printf("a=[%06.3O]\n", a);
	Printf("a=[%0.3O]\n", a);
	Printf("a=[%-6O]\n", a);
	Printf("a=[%-6.3O]\n", a);
	Printf("a=[%-.3O]\n", a);
	Printf("\n");
	Printf("a=[%lO]\n", a);
*/

/*
	INT32 a = 9;
	INT32 w = 6;
	INT32 p = 3;
	Printf("a=[%o]\n", a);
	Printf("a=[%0o]\n", a);
	Printf("a=[%-o]\n", a);
	Printf("\n");
	Printf("a=[%06o]\n", a);
	Printf("a=[%06.3o]\n", a);
	Printf("a=[%0.3o]\n", a);
	Printf("a=[%-6o]\n", a);
	Printf("a=[%-6.3o]\n", a);
	Printf("a=[%-.3o]\n", a);
	Printf("\n");
	Printf("a=[%lo]\n", a);
	Printf("a=[%llo]\n", a);
*/

/*
	INT32 a = 9;
	INT32 w = 6;
	INT32 p = 3;
	Printf("a=[%X]\n", a);
	Printf("a=[%0X]\n", a);
	Printf("a=[%-X]\n", a);
	Printf("\n");
	Printf("a=[%06X]\n", a);
	Printf("a=[%06.3X]\n", a);
	Printf("a=[%0.3X]\n", a);
	Printf("a=[%-6X]\n", a);
	Printf("a=[%-6.3X]\n", a);
	Printf("a=[%-.3X]\n", a);
	Printf("\n");
	Printf("a=[%lX]\n", a);
*/

/*
	INT32 a = 9;
	INT32 w = 6;
	INT32 p = 3;
	Printf("a=[%x]\n", a);
	Printf("a=[%0x]\n", a);
	Printf("a=[%-x]\n", a);
	Printf("\n");
	Printf("a=[%06x]\n", a);
	Printf("a=[%06.3x]\n", a);
	Printf("a=[%0.3x]\n", a);
	Printf("a=[%-6x]\n", a);
	Printf("a=[%-6.3x]\n", a);
	Printf("a=[%-.3x]\n", a);
	Printf("\n");
	Printf("a=[%lx]\n", a);
*/

/*
	INT32 a = 9;
	INT32 w = 3;
	INT32 p = 2;
	Printf("a=[%u]\n", a);
	Printf("a=[%0u]\n", a);
	Printf("a=[%-u]\n", a);
	Printf("\n");
	Printf("a=[%03u]\n", a);
	Printf("a=[%03.2u]\n", a);
	Printf("a=[%0.2u]\n", a);
	Printf("a=[%-3u]\n", a);
	Printf("a=[%-3.2u]\n", a);
	Printf("a=[%-.2u]\n", a);
	Printf("\n");
	Printf("a=[%lu]\n", a);
	Printf("\n");
	Printf("a=[%0*u]\n", w, a);
	Printf("a=[%0*.*u]\n", w, p, a);
	Printf("a=[%0.*u]\n", p, a);
	Printf("a=[%-*u]\n", w, a);
	Printf("a=[%-*.*u]\n", w, p, a);
	Printf("a=[%-.*u]\n", p, a);
*/

/*
	INT32 a = 9;
	INT32 w = 3;
	INT32 p = 2;
	Printf("a=[%i]\n", a);
	Printf("a=[%0i]\n", a);
	Printf("a=[%-i]\n", a);
	Printf("\n");
	Printf("a=[%03i]\n", a);
	Printf("a=[%03.2i]\n", a);
	Printf("a=[%0.2i]\n", a);
	Printf("a=[%-3i]\n", a);
	Printf("a=[%-3.2i]\n", a);
	Printf("a=[%-.2i]\n", a);
	Printf("\n");
	Printf("a=[%li]\n", a);
	Printf("\n");
	Printf("a=[%0*i]\n", w, a);
	Printf("a=[%0*.*i]\n", w, p, a);
	Printf("a=[%0.*i]\n", p, a);
	Printf("a=[%-*i]\n", w, a);
	Printf("a=[%-*.*i]\n", w, p, a);
	Printf("a=[%-.*i]\n", p, a);
*/

/*
	INT32 a = 9;
	INT32 w = 3;
	INT32 p = 2;
	Printf("a=[%d]\n", a);
	Printf("a=[%0d]\n", a);
	Printf("a=[%-d]\n", a);
	Printf("\n");
	Printf("a=[%03d]\n", a);
	Printf("a=[%03.2d]\n", a);
	Printf("a=[%0.2d]\n", a);
	Printf("a=[%-3d]\n", a);
	Printf("a=[%-3.2d]\n", a);
	Printf("a=[%-.2d]\n", a);
	Printf("\n");
	Printf("a=[%ld]\n", a);
	Printf("\n");
	Printf("a=[%0*d]\n", w, a);
	Printf("a=[%0*.*d]\n", w, p, a);
	Printf("a=[%0.*d]\n", p, a);
	Printf("a=[%-*d]\n", w, a);
	Printf("a=[%-*.*d]\n", w, p, a);
	Printf("a=[%-.*d]\n", p, a);
*/


/*
	UINT64 a = 0x7fffffffffffffff;
	INT32 w = 25;
	INT32 p = 22;
	Printf("a=[%lld]\n", a);
	Printf("a=[%0lld]\n", a);
	Printf("a=[%-lld]\n", a);
	Printf("a=[%025lld]\n", a);
	Printf("a=[%025.22lld]\n", a);
	Printf("a=[%0.22lld]\n", a);
	Printf("a=[%-25lld]\n", a);
	Printf("a=[%-25.22lld]\n", a);
	Printf("a=[%-.22lld]\n", a);
	Printf("\n");
	Printf("a=[%0*lld]\n", w, a);
	Printf("a=[%0*.*lld]\n", w, p, a);
	Printf("a=[%0.*lld]\n", p, a);
	Printf("a=[%-*lld]\n", w, a);
	Printf("a=[%-*.*lld]\n", w, p, a);
	Printf("a=[%-.*lld]\n", p, a);
*/

/*
	UINT64 a = 0x8000000000000000;
	Printf("a=[%lld]\n", a);
	Printf("a=[%0lld]\n", a);
	Printf("a=[%-lld]\n", a);
	Printf("a=[%025lld]\n", a);
	Printf("a=[%025.22lld]\n", a);
	Printf("a=[%0.22lld]\n", a);
	Printf("a=[%-25lld]\n", a);
	Printf("a=[%-25.22lld]\n", a);
	Printf("a=[%-.22lld]\n", a);
*/

/*
	UINT64 a = 0x8000000000000001;
	Printf("a=[%lld]\n", a);
	Printf("a=[%0lld]\n", a);
	Printf("a=[%-lld]\n", a);
	Printf("a=[%025lld]\n", a);
	Printf("a=[%025.22lld]\n", a);
	Printf("a=[%0.22lld]\n", a);
	Printf("a=[%-25lld]\n", a);
	Printf("a=[%-25.22lld]\n", a);
	Printf("a=[%-.22lld]\n", a);
*/

/*
	STRPTR a = "srinivas";
	INT32 w = 25;
	INT32 p = 22;
	Printf("a=[%s]\n", a);
	Printf("a=[%0s]\n", a);
	Printf("a=[%-s]\n", a);
	Printf("a=[%025s]\n", a);
	Printf("a=[%025.22s]\n", a);
	Printf("a=[%0.22s]\n", a);
	Printf("a=[%-25s]\n", a);
	Printf("a=[%-25.22s]\n", a);
	Printf("a=[%-.22s]\n", a);
	Printf("\n");
	Printf("a=[%0*s]\n", w, a);
	Printf("a=[%0*.*s]\n", w, p, a);
	Printf("a=[%0.*s]\n", p, a);
	Printf("a=[%-*s]\n", w, a);
	Printf("a=[%-*.*s]\n", w, p, a);
	Printf("a=[%-.*s]\n", p, a);
*/

	CloseLibrary(DOSBase);
	return RETURN_OK;
}

static INT32 add(INT32 a, INT32 b)
{
	INT32 c;
	c = a + b;
	return c;
}

#endif
