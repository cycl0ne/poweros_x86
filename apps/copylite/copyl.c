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

#define	VSTRING	"Copylite 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "FROM/A,TO/A,BUFFER/N" CMDREV
#define OPT_FROM	0
#define OPT_TO		1
#define OPT_BUFFER  2
#define OPT_COUNT   3
#define PROGNAME "CopyLite"
#define SEEK_ERROR        -1L

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	int32_t	rc2 = 0;
	INT32 	opts[OPT_COUNT];
	pFileHandle fh;
	pFileHandle fh2;
	FileInfoBlock	fib;
	uint32_t bufsize = 512*1024;
	pFileLock	lock;

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
				if (opts[OPT_BUFFER]) bufsize = *((UINT32*)opts[OPT_BUFFER]);

				STRPTR file;
				file = (STRPTR)opts[OPT_FROM];
				lock = Lock((STRPTR)opts[OPT_FROM], ACCESS_READ);
				if (lock)
				{
					if (!Examine(lock, &fib) || fib.fib_DirEntryType >0)
					{
						rc= RETURN_FAIL;
						Printf("Error, Inputfile is a Directory\n");
						UnLock(lock);
						goto error;
					}
				} else
				{
					Printf("Input File \"%s\" couldnt be opened.\n", opts[OPT_FROM]);
					rc2 = IoErr();
					goto error;
				}
				UnLock(lock);
				fh = Open(file, MODE_OLDFILE);
				if (fh)
				{
					fh2 = Open((STRPTR)opts[OPT_TO], MODE_NEWFILE);
					if (fh2)
					{
						uint8_t *buffer = AllocVec(bufsize, MEMF_FAST);
						if (buffer)
						{
							uint32_t size = fib.fib_Size;
							int32_t  cnt = 0;
							Printf("[%d]Copying file: %s (size: %d) to %s, with a %d byte buffer\n", TimeStamp(), file, size, opts[OPT_TO], bufsize);
							while (size>0)
							{
								cnt = FRead(fh, buffer, 1, bufsize);
								if (cnt == -1)
								{
									Printf("Error\n");
									goto error;
								}
								FWrite(fh2, buffer, 1, cnt);
								size -= cnt;
							}
							FreeVec(buffer);
							Printf("[%d]Finished\n", TimeStamp());
							rc = RETURN_OK;
						} else
						{
							Printf("Couldnt allocate Memory for copybuffer.\n");
							goto error;
						}
					} else
					{
						Printf("Output File \"%s\" couldnt be opened.\n", opts[OPT_TO]);
						goto error;
					}
				} else
				{
					Printf("Error on Input File \"%s\".\n", opts[OPT_FROM]);
				}
			}
		}
error:
		if (fh) Close(fh);
		if (fh2) Close(fh2);
		FreeArgs(rdargs);
		if (UtilBase) CloseLibrary(UtilBase);
		CloseLibrary(DOSBase);
	}
	return rc;
}

