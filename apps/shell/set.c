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

#define TEMPLATE    "NAME,STRING/M"
#define OPT_NAME    0
#define OPT_STRING  1
#define OPT_COUNT   2

static void hexdump (pDOSBase DOSBase, unsigned char *buf,int len)
{
	int cnt3,cnt4;
	int cnt=0;
	int cnt2=0;

	do
	{
		VPrintf("%08X | ", &cnt);
		for (cnt3=0;cnt3<16;cnt3++)
		{
			if (cnt<len)
			{
				VPrintf("%02X ", &buf[cnt++]);
			}
			else
				VPrintf("   ", NULL);
		}
		VPrintf("| ", NULL);
		for (cnt4=0;cnt4<cnt3;cnt4++)
		{
			if (cnt2==len)
				break;
			if (buf[cnt2]<0x20)
				VPrintf(".", NULL);
			else
				if (buf[cnt2]>0x7F && buf[cnt2]<0xC0)
					VPrintf(".", NULL);
				else
					VPrintf("%c", &buf[cnt2]);
			cnt2++;
		}
		PutStr("\n");
	}
	while (cnt!=len);
}

int cmd_setenv(APTR SysBase)
{
	pDOSBase		DOSBase;
	DOSIO			rc = RETURN_ERROR;
	char*			str;
	INT32			opts[OPT_COUNT];
	struct RDargs	*rdargs;
	pLocalVar		lv;
	char*			template = TEMPLATE;
	
	INT32 ret;
	
	if ( (DOSBase = OpenLibrary("dos.library", 0)) )
	{
		//UtilityBase = OpenLibrary("utility.library", 0);
#if 0
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(template, opts);
		if (rdargs == NULL)
		{
			KPrintF("Error SET: ReadArgs\n");
			PrintFault(IoErr(), NULL);
		}
#endif		
		PutStr("CreateMsgPort ");
		pMsgPort p	= CreateMsgPort(NULL);
		if (p)
		{
			PutStr("ok, CreateIORequest ");
			pIOStdReq io= CreateIORequest(p, 0);
			if (io)
			{	
				VPrintf("ok (MsgPort: %x), OpenDevice ", &p);
				if (!(ret = OpenDevice("pata.device", 0, io, 0)))
				{
					VPrintf("ok (ret:%d), DoIO ",&ret);
					UINT8 *buffer = AllocVec(4096, 0); //[512];
					io->io_Command	= CMD_READ;
					io->io_Offset	= 0;
					io->io_Length	= 1;
					io->io_Data		= buffer;
					ret = DoIO(io);
					VPrintf("ok (ret = %d)\n", &ret);

					if (ret>=0)
					{
						hexdump(DOSBase, buffer, 512);
						io->io_Command = CMD_WRITE;
						io->io_Offset	= 0;
						io->io_Length	= 1;
						io->io_Data		= buffer;
						INT8 buffer2[] = "Hello, this is a test";
						CopyMem(buffer2, buffer, 22);
						PutStr("Writing now.\n");
						//ret = DoIO(io);
						
						Flush(Output());
						FreeVec(buffer);
						DeleteIORequest(io);
						DeleteMsgPort(p);
						return 0;
					}
				}
			}
			PutStr("Error in creationo of IO Req\n");
			DeleteMsgPort(NULL);
		}
		PutStr("Error in MsgPort\n");
		CloseLibrary(DOSBase);
	}
	Flush(Output());
	return 0;
}



