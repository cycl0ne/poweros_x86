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

#define	VSTRING	"Touch 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "DIRNAME/A,FILECOUNT/N" CMDREV
#define OPT_NAME    0
#define OPT_AMOUNT	1
#define OPT_COUNT   2

#define	RAND_MAX	0x7fffffff

#define MAX_FILES  10000
#define MAX_NAME   24

static int32_t SeedNext = 0;

int rand()
{
	return ((SeedNext = SeedNext * 1103515245 + 12345) % ((UINT32)RAND_MAX + 1));
}

static void
make_random_name(char *buf, int len)
{
    int i, max = (rand() % (len - 7)) + 6;

    for(i=0; i < max; i++) {
        buf[i] = 'a' + (rand() % 26);
    }

    buf[i] = '\0';
}

void makename(STRPTR buf, int32_t val, int32_t size)
{
	int32_t i;
	int32_t j;
	buf[size-1] = '\0';
	for(i=size-2; i >=0 ; i--)
	{
		j = val % 10;
		val = (val - j) / 10;
		if(j == 0 && val == 0)
		{
			buf[i] = '_';
		} else
		{
			buf[i] = '0' + j;
		}
	}
}

static void _prbuf(INT32 ch, INT8 **str)
{
	*(*str)++ = (char)ch;
}

static int _sprintf(pSysBase SysBase, char* str, char *fmt,...)
{
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) str);
	va_end(pvar);
	return 0;
}

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];
	
//	char      buf[MAX_FILES][MAX_NAME];

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL)
		{
			PrintFault(IoErr(), NULL);
		} else
		{
			rc = RETURN_OK;
			
			if (*((INT32*)opts[OPT_AMOUNT]) > MAX_FILES) *((INT32*)opts[OPT_AMOUNT]) = MAX_FILES;
			int32_t max = *((INT32*)opts[OPT_AMOUNT]);
			
			STRPTR dirname = (STRPTR)opts[OPT_NAME];
			pFileLock lock = CreateDir(dirname);
			if (!lock) lock = Lock(dirname, SHARED_LOCK);
			if (lock)
			{
				pFileHandle files; //[MAX_FILES];
				pFileLock old = CurrentDir(lock);
				char buf[16];
				
				for (int i = 0; i<max; i++)
				{
					//make_random_name(&buf[i*MAX_NAME], MAX_NAME);
					_sprintf(SysBase, buf, "%d", i);
					Printf("Calling Open with MODE_NEWFILE on File: %s\n", buf);
					//makename(buf, i, 6);
					files = Open(buf, MODE_NEWFILE);
					if (files) Printf("Created File [%d]: %s, %x\n", i, buf, files);
					else
					{
						Printf("Couldnt create File [%d] - Exiting\n",i);
						rc = RETURN_ERROR;
						CurrentDir(old);
						UnLock(lock);
						goto err;
					}
					Close(files);
					//Printf("Closing File [%d]\n", i);
				}
				
				for (int i = 0; i<max; i++)
				{
					//makename(buf, i, 6);
					_sprintf(SysBase,buf, "%d", i);
					Printf("Going to delete File\n");
					int32_t del = DeleteFile(buf);
					if (del) Printf("Delete file [%d]: %s [%d]\n", i, buf, del);
					else
					{
						Printf("Couldnt delete file [%d] - Exiting\n",i);
						rc = RETURN_ERROR;
						CurrentDir(old);
						UnLock(lock);
						goto err;
					}
				}
				
				CurrentDir(old);
				UnLock(lock);
				Printf("Deleting dir [%s]\n",dirname);
				DeleteFile(dirname);
			} else
			{
				Printf("Couldnt create Directory\n");
			}
		}
err:
		FreeArgs(rdargs);
		CloseLibrary(DOSBase);
	}
	return rc;
}

