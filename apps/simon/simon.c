/**
* File: /stack．c
* User: cycl0ne
* Date: 2014-10-31
* Time: 06:29 PM
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

#define	VSTRING	"Simon 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "FILENAME/A,OFFSET/K/N,DATA" CMDREV
#define OPT_FILE	0
#define OPT_OFFSET	1
#define OPT_DATA    2
#define OPT_COUNT   3
#define PROGNAME "Write"
#define SEEK_ERROR        -1L

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];
	pFileHandle fh;

	uint32_t offset = 0;

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL)
			{
				PrintFault(IoErr(), PROGNAME);
			} else
			{
				STRPTR file = (STRPTR)opts[OPT_FILE];
				fh = Open(file, MODE_OLDFILE);
				if (!fh)
				{
					if (opts[OPT_OFFSET])
					{
						Printf("Cant seek into non existing File. Offset parameter given.\n");
						goto error;
					}
			        Printf("Can't open '%s', creating it...\n", opts[OPT_FILE]);
					fh = Open((STRPTR)opts[OPT_FILE], MODE_NEWFILE);

					if (!fh)
					{
			            PrintFault(IoErr(), PROGNAME);
	        			Printf("Can't create file '%s'!\n", opts[OPT_FILE]);
						goto error;
					}
				} else
				{
					Printf("Opening existing file: %s\n", opts[OPT_FILE]);
				}

				if (opts[OPT_OFFSET])
				{
					offset = *((UINT32*)opts[OPT_OFFSET]);
					int32_t err = Seek(fh, offset, SEEK_SET);

					if (err == SEEK_ERROR)
					{
						PrintFault(IoErr(), PROGNAME);
						Printf("Error on Seek() to offset %d!\n", offset);
						goto error;
					} else
					{
						Printf("Successful Seek() to %d\n", *((UINT32*)opts[OPT_OFFSET]));
					}
				}

				if (opts[OPT_DATA])
				{
					rc = RETURN_OK;
					Printf("Writing \"%s\" to offset %d\n", opts[OPT_DATA], offset);
					STRPTR buffer  = (STRPTR)opts[OPT_DATA];
					Write(fh, (uint8_t *)buffer, Strlen(buffer));
				}
			}
		}
error:
		if (fh) Close(fh);
		FreeArgs(rdargs);
		if (UtilBase) CloseLibrary(UtilBase);
		CloseLibrary(DOSBase);
	}
	return rc;
}

