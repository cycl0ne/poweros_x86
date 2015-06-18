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

#define	VSTRING	"SetFileSize 1.0 (20.12.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define PROGNAME "SetFileSize"

#define TEMPLATE "FILE/A,SIZE/A/N,MODE/K,SEEKTO/K/N,SEEKMODE/K" CMDREV

#define OPT_FILE		0
#define OPT_SIZE		1
#define OPT_MODE		2
#define OPT_SEEKTO		3
#define OPT_SEEKMODE	4
#define OPT_COUNT		5

#define SEEK_BAD		-1
#define ERRMSG_BADMODE "Bad SEEKMODE '%s', must be one of {(B)egin, (E)nd, (C)urrent}!\n"
#define SETFILESIZE_ERROR -1L
#define SEEK_ERROR        -1L

static int  getMode(STRPTR amode)
{
    if (!amode) {
        return(SEEK_SET);
    }

    switch (*amode)
    {
        case 'b':
        case 'B':
            return(SEEK_SET);
            break;
        case 'C':
        case 'c':
            return(SEEK_CUR);
            break;
        case 'e':
        case 'E':
            return(SEEK_END);
            break;
        default:
            return(SEEK_BAD);
            break;
    }
    return(SEEK_BAD);
}

static STRPTR modeToString(int mode)
{
    switch (mode) {
        case SEEK_SET:
            return("beginning");
            break;
        case SEEK_END:
            return("end");
            break;
        case SEEK_CUR:
            return("current");
            break;
        default:
            break;
    }
    return("invalid");
}




DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	uint32_t	rc 	= RETURN_FAIL;
	uint32_t 	opts[OPT_COUNT];
	int32_t		mode;
	int32_t		newSize;
	pFileHandle fh;

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(), PROGNAME);
		} else
		{
			mode = getMode((STRPTR)opts[OPT_MODE]);
			if (mode == SEEK_BAD)
			{
				Printf(ERRMSG_BADMODE, opts[OPT_MODE]);
				goto error;
			} else
			{
				fh = Open((STRPTR)opts[OPT_FILE], MODE_OLDFILE);
				if (!fh)
				{
			        Printf("Can't open '%s', creating it...\n", opts[OPT_FILE]);
        			fh = Open((STRPTR)opts[OPT_FILE], MODE_NEWFILE);

					if (!fh)
					{
			            PrintFault(IoErr(), PROGNAME);
            			Printf("Can't create file '%s'!\n", opts[OPT_FILE]);
						goto error;
					}
				}
				if (opts[OPT_SEEKTO])
				{
					int32_t		seekMode;
					int32_t		err = 0;

					seekMode = getMode((STRPTR)opts[OPT_SEEKMODE]);
					if (seekMode == SEEK_BAD)
					{
						Printf(ERRMSG_BADMODE, opts[OPT_SEEKMODE]);
						goto error;
					} else
					{
						err = Seek(fh, *((UINT32*)opts[OPT_SEEKTO]), seekMode);
						if (err == SEEK_ERROR)
						{
							PrintFault(IoErr(), PROGNAME);
							PutStr("Error on Seek()!\n");
							rc = RETURN_ERROR;
							goto error;
						} else
						{
							Printf("Tell: %d\n", Tell(fh));
							Printf("Successful Seek() from %ld to %ld from \"%s\"\n", err, *((UINT32*)opts[OPT_SEEKTO]), modeToString(seekMode));
						}
					}
				}
			    Printf("Calling SetFileSize($%08lx, %ld, %s)...\n", fh, *((UINT32*)opts[OPT_SIZE]), modeToString(mode));
			    newSize = SetFileSize(fh, *((UINT32*)opts[OPT_SIZE]), mode);

			    if (newSize == SETFILESIZE_ERROR)
			    {
			        PrintFault(IoErr(), PROGNAME);
			        Printf("Error %ld on SetFileSize!\n", newSize);
			        goto error;
			    } else
			    {
			        Printf("Success.  Position of new EOF: %ld\n", newSize);
			    }
				rc = RETURN_OK;
			}
		}
error:
		if (fh) Close(fh);
		FreeArgs(rdargs);
		CloseLibrary(DOSBase);
	}
	return rc;
}

