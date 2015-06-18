/**
 * @file sanitycheck.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "execbase_private.h"
#include "ports.h"
#include "devices.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"


static void hexdump(APTR DOSBase, unsigned char *buf,int len)
{
	int cnt3,cnt4;
	int cnt=0;
	int cnt2=0;

	do
	{
		Printf("%08X | ",cnt);
		for (cnt3=0;cnt3<16;cnt3++)
		{
			if (cnt<len)
			{
				Printf("%02X ",buf[cnt++]);
			}
			else
				Printf("   ");
		}
		Printf("| ");
		for (cnt4=0;cnt4<cnt3;cnt4++)
		{
			if (cnt2==len)
				break;
			if (buf[cnt2]<0x20)
				Printf(".");
			else
				if (buf[cnt2]>0x7F && buf[cnt2]<0xC0)
					Printf(".");
				else
					Printf("%c", buf[cnt2]);
			cnt2++;
		}
		Printf("\n");
	}
	while (cnt!=len);
}


#define TEMPLATE 		"DEVICE,UNIT/N"
#define OPT_DEVICE		0
#define OPT_UNIT		1
#define OPT_SUPER		2
#define OPT_SECTOR		3
#define OPT_PREALLOC	4
#define OPT_NAME		5
#define OPT_COUNT		6

#define PASSED "\033[60G[\033[1;32mPASS\033[0;39m]\n\r"
#define FAILED "\033[60G[\033[1;31mFAIL\033[0;39m]\n\r"

struct GlobalData
{
	APTR SysBase;
	APTR DOSBase;
	APTR UtilBase;
};

void PrintCon(struct GlobalData *gd, pIOStdReq io, STRPTR string)
{
	APTR SysBase = gd->SysBase;
	APTR UtilBase = gd->UtilBase;
	APTR DOSBase = gd->DOSBase;
	
	io->io_Command = CMD_WRITE;
	io->io_Length = Strlen(string);
	io->io_Data = string;
	INT32 ret = DoIO(io);
	if (ret) Printf("Error in DoIO\n");
}
// "\033[1;34m%s-%s \033[1;31m%d\033[1;34m %s#\033[0m ", __kernel_name, version_number, retval, current_process->wd_name

DOSIO cmd_testconsole(pSysBase SysBase)
{
	APTR		DOSBase;
	DOSIO		rc = RETURN_ERROR;
	UINT32		opts[OPT_COUNT];
	struct RDargs*	rdargs;
	struct GlobalData gd;
	if ( (DOSBase = OpenLibrary("dos.library", 0)) )
	{
		APTR UtilBase = OpenLibrary("utility.library", 0);
		gd.UtilBase = UtilBase;
		gd.SysBase	= SysBase;
		gd.DOSBase	= DOSBase;
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL)
		{
			PrintFault(IoErr(), NULL);
		} else
		{
			pMsgPort p = CreateMsgPort(NULL);
			if (p)
			{
				pIOStdReq io = CreateIORequest(p,0);
				if (io)
				{
					if (!OpenDevice("consolex86.device", 0, io, 0))
					{
						rc = RETURN_OK;
						Printf("Sending a CLS to console.device\n");
						UINT8 data[] = "\033[2J";
						io->io_Command = CMD_WRITE;
						io->io_Length = 4;
						io->io_Data = &data;
						DoIO(io);
						Printf("io->io_Error: %d\n", io->io_Error);
						Printf("Screen cleared Response [%d]\n", io->io_Actual);
						Printf("Sending a PASSED to console.device\n");
						PrintCon(&gd, io, "Test it ");
						PrintCon(&gd, io, PASSED);
						PrintCon(&gd, io, "Test it ");
						PrintCon(&gd, io, FAILED);
//						PrintCon(&gd, io, "Test it \n");
						Printf("Done.\n");
						//printf( "\033[2J");
						CloseDevice(io);
					}
					DeleteIORequest(io);
				}
				DeleteMsgPort(p);
			}
			CloseLibrary(DOSBase);
		}
	} else
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

//int make(const char* deviceName, uint64_t primarySuper, uint64_t sectorCount, uint8_t logSectorsPerBand, uint8_t preallocCount, const char* volumeLabel);


