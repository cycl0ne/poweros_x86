/**
 * @file makeleanfs.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "ports.h"
#include "devices.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

typedef struct GlobalData
{
	pDOSBase	dOSBase;
	pSysBase	sysBase;
	pUtilBase	utilBase;
	pMsgPort	p;
	pIOStdReq	io;

	UINT8 buffer_[512];
} GD, *pGD;

#define SysBase		gd->sysBase
#define DOSBase		gd->dOSBase
#define UtilBase	gd->utilBase
#define buffer_			gd->buffer_

static void hexdump(pGD gd, unsigned char *buf,int len)
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

static int WriteSector(pGD gd, UINT32 off, UINT32 len, APTR buf)
{
	INT32 rc;
	gd->io->io_Command = CMD_WRITE;
	gd->io->io_Offset	= off;
	gd->io->io_Length	= len;
	gd->io->io_Data		= buf;
	//KPrintF("Writing to sector: %d len: %d,", off, len);
	rc = DoIO(gd->io);
	if (off==3) hexdump(gd, buf, 512);
	return rc;
}

static int ReadSector(pGD gd, UINT32 off, UINT32 len, APTR buf)
{
	INT32 rc;
	gd->io->io_Command = CMD_READ;
	gd->io->io_Offset	= off;
	gd->io->io_Length	= len;
	gd->io->io_Data		= buf;
	//KPrintF("Reading from sector: %d len: %d,", off, len);
	rc = DoIO(gd->io);
	return rc;
}

static int OpenDev(pGD gd, const char* deviceName, UINT32 unit)
{
	INT32 rc;
	gd->p = CreateMsgPort(NULL);
	if (gd->p)
	{
		gd->io = CreateIORequest(gd->p,0);
		if (gd->io)
		{
			if (!OpenDevice((STRPTR)deviceName, unit, gd->io, 0))
			{
				return RETURN_OK;
			} else rc = RETURN_ERROR;
			DeleteIORequest(gd->io);
		} else rc = RETURN_ERROR;
		DeleteMsgPort(gd->p);
	} else rc = RETURN_ERROR;
	return rc;
}

#define memcpy(a,b,c) CopyMem(b,a,c)

static int Make(pGD gd, const char* deviceName, UINT32 unitnum)
{
	int res = RETURN_OK;
	//Printf("Opening Device [%s] with Unit %d\n", deviceName, unitnum);
	//res = OpenDev(gd, deviceName, unitnum);

	Printf("Searching for SYS:\n");


	pDosEntry de = FindDosEntry(NULL, "SYS", LDF_ALL);
	if (de)
	{
		Printf("Found %s. Sending Format command\n", de->de_Node.ln_Name);
		pHandlerProc hp = ObtainHandler("sys:");
		if (hp)
		{
			Printf("Send format to: %x\n", hp->hp_Port);
			if (!DoPkt(hp->hp_Port, ACTION_INHIBIT, DOSIO_TRUE, 0 ,0, 0, 0))
			{
				res = RETURN_ERROR;
			} else
			{
				if (!DoPkt(hp->hp_Port, ACTION_FORMAT, "System Drive", 0 ,0, 0, 0))
				{
					res = RETURN_ERROR;
				} else
				{
					if (!DoPkt(hp->hp_Port, ACTION_INHIBIT, DOSIO_FALSE, 0 ,0, 0, 0))
					{
						res = RETURN_ERROR;

					}
				}
			}
			ReleaseHandler(hp);
		}

	}

	Flush(Output());
	return res;
}

static UINT32 divide(UINT32 d0, UINT16 d1)
{
	UINT32 q = d0 / d1;
	/* NOTE: I doubt anything depends on this, but lets simulate 68k divu overflow anyway - Piru */
	if (q > 65535UL) return d0 | q;
//	return ((d0 % d1)) | q;
	return ((d0 % d1) << 16) | q;
}

INT32 unittest(pGD gd)
{
struct
	{
		UINT32 a;
		UINT16 b;
		UINT32 r;
	} div[] =
	{
		{0,1,0x00000000},
		{1,1,0x00000001},
		{1,2,0x00010000},
		{1,3,0x00010000},
		{1,4,0x00010000},
		{1,1000,0x00010000},
		{1,6000,0x00010000},
		{10000,666,0x000a000f},
		{80000,1,0x00003880},
		{80000,666,0x01de0015},
	};
	const div_numof = sizeof(div) / sizeof(div[0]);
	int i;
	UINT8 tmp[256], *p;
	UINT8 t2[64];
	UINT32	dive;

	Printf("\ndivide:\n");

	for (i = 0; i< div_numof; i++)
	{
		Printf("%d / %d = 0x%08x - %x - %x\n", div[i].a, div[i].b, divide(div[i].a, div[i].b), (div[i].a % div[i].b)<<16, div[i].a / div[i].b);
		dive = divide(div[i].a, div[i].b);
		if (dive != div[i].r)
		{
			Printf("failed! %d: %x %x %x\n", i, dive, div[i].r, (div[i].a % div[i].b)<<16);
			return 5;
		}
	}
	return 0;
}


#define TEMPLATE 		"DEVICE,UNIT/N"
#define OPT_DEVICE		0
#define OPT_UNIT		1
#define OPT_SUPER		2
#define OPT_SECTOR		3
#define OPT_PREALLOC	4
#define OPT_NAME		5
#define OPT_COUNT		6

#undef SysBase
DOSIO cmd_makesfs(APTR SysBase)
{
	DOSIO		rc = RETURN_ERROR;
	UINT32		opts[OPT_COUNT];
	struct RDargs*	rdargs;
	pGD	gd = AllocVec(sizeof(GD), MEMF_CLEAR);
	gd->sysBase = SysBase;

	if ( (gd->dOSBase = OpenLibrary("dos.library", 0)) )
	{
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL)
		{
			KPrintF("Error SET: ReadArgs\n");
			PrintFault(IoErr(), NULL);
		} else{
			gd->utilBase = OpenLibrary("utility.library", 0);
			rc = Make(gd, (STRPTR)opts[OPT_DEVICE], *((UINT32*)opts[OPT_UNIT]));
		//	rc = unittest(gd);

#if 0
		char deviceName[] 	= "pata.device";
		UINT32 unitNum		= 1;
		UINT64 super		= 1;
		UINT64 sector		= 32768;
		UINT8  logSector	= 12;
		UINT8  prealloc    	= 0;
		char VolName[]		= "My Volume";

		rc = Make(gd, deviceName, unitNum, super, sector, logSector, prealloc, VolName);
#endif
			CloseLibrary(gd->utilBase);
		//rc = RETURN_OK;
#if 1
		}
		FreeArgs(rdargs);
#endif
		CloseLibrary(DOSBase);
	} else
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

//int make(const char* deviceName, uint64_t primarySuper, uint64_t sectorCount, uint8_t logSectorsPerBand, uint8_t preallocCount, const char* volumeLabel);


