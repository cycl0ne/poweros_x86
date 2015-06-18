/**
 * @file set.c
 *
 * SHELL Command: Set
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

typedef struct GlobalData
{
	pDOSBase	dOSBase;
	pSysBase	sysBase;
//	pUtilBase	utilBase;
	pMsgPort	p;
	pIOStdReq	io;
} GD, *pGD;


#define SysBase		gd->sysBase
#define DOSBase		gd->dOSBase
#define UtilBase	gd->utilBase

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

//#include "pfs_handler_.h"


#undef SysBase
DOSIO cmd_pfs(APTR SysBase)
{
	INT32 rc = RETURN_OK;
	pGD	gd = AllocVec(sizeof(GD), MEMF_CLEAR);
	gd->sysBase = SysBase;

	if ( (gd->dOSBase = OpenLibrary("dos.library", 0)) )
	{
//		Printf("Size Suberblock: %d\n",sizeof(SuperBlock));
//		Printf("Size INode: %d\n",sizeof(INode));
//		Printf("Size IBlock: %d\n",sizeof(IBlock));
		Printf("offset: %d\n", 8/8/1024);
		CloseLibrary(DOSBase);
	} else
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

