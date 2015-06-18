/**
* File: /gfxtest．c
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
#include "gfx.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "gfx_interface.h"
#include "utility_interface.h"

#define	VSTRING	"Prompt 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE   "PROMPT" CMDREV
#define OPT_PROMPT 0
#define OPT_COUNT  1

APTR gfx_OpenView(APTR GfxBase);

static void draw3dbox(APTR GfxBase, APTR pb,  int x, int y, int w, int h, UINT32 crTop, UINT32 crBottom);

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];

	STRPTR	prompt;
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else 
		{
			Printf("Starting GfxTest\n");
			APTR GfxBase = OpenLibrary("gfx.library",0);
			if (GfxBase)
			{
				APTR pb = gfx_OpenView(GfxBase);

				SetForegroundColor(pb, RGB(150,150,150));
				RectFill(pb, 0, 0, 1024, 768);

				draw3dbox(GfxBase, pb, 50, 50, 200, 200, RGB(162, 141, 104), RGB(234, 230, 221));
				draw3dbox(GfxBase, pb, 51, 51, 198, 198, RGB(  0,   0,   0), RGB(213, 204, 187));

				Printf("Done\n");
				CloseLibrary(GfxBase);
			} else
				Printf("Failed to open Lib.!\n");
			FreeArgs(rdargs);
		}			
		CloseLibrary(DOSBase);
	}
	return rc;
}

static void draw3dbox(APTR GfxBase, APTR pb,  int x, int y, int w, int h, UINT32 crTop, UINT32 crBottom)
{
	UINT32 old = SetForegroundColor(pb, crTop);
	Line(pb, x, y+h-2, x, y+1,TRUE);		/* left*/
	Line(pb, x, y, x+w-2, y,TRUE);			/* top*/

	SetForegroundColor(pb, crBottom);
	Line(pb, x+w-1, y, x+w-1, y+h-2,TRUE);		/* right*/
	Line(pb, x+w-1, y+h-1, x, y+h-1,TRUE);		/* bottom*/
	SetForegroundColor(pb, old);
}

