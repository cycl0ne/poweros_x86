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

#define memcpy(a,b,c) CopyMem(b,a,c)

#include "memory.h"

#define TEMPLATE 		"DEVICE,UNIT/N"
#define OPT_DEVICE		0
#define OPT_UNIT		1
#define OPT_SUPER		2
#define OPT_SECTOR		3
#define OPT_PREALLOC	4
#define OPT_NAME		5
#define OPT_COUNT		6

#define PASSED "\033[60G[\033[1;32mPASS\033[0;39m]\n"
#define FAILED "\033[60G[\033[1;31mFAIL\033[0;39m]\n"
#include "context.h"

//DOSIO cmd_sanitycheck(pSysBase SysBase)
DOSIO main(pSysBase SysBase)
{
	APTR		DOSBase;
	DOSIO		rc = RETURN_ERROR;
	UINT32		opts[OPT_COUNT];
	struct RDargs*	rdargs;

	if ( (DOSBase = OpenLibrary("dos.library", 0)) )
	{
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL)
		{
			KPrintF("Error SET: ReadArgs\n");
			PrintFault(IoErr(), NULL);
		} else
		{
			rc = RETURN_OK;

			pMemCHead header;
			struct MemHeader *mh=(struct MemHeader *)GetHead(&SysBase->MemList);
			INT32 cnt = 0;
			ForeachNode(&mh->mh_ListUsed, header)
			{
				Task *task = header->mch_Task;
				Printf("Used Memory at %x, size %x (%d), task [%s]\n", header, header->mch_Size, header->mch_Size, task->tcb_Node.ln_Name);
				pMemCFoot footer = (pMemCFoot) ((UINT32)header + header->mch_Size - sizeof(MemCFoot));

				Printf("Checking Memory Headers.....");
				if (header->mch_Magic != MCHC_MAGIC)
				{
					Printf(FAILED);
				} else
				{
					Printf(PASSED);			
				}
				Printf("Checking Memory Footers.....");
				if (footer->mcf_Magic != MCHC_MAGIC)
				{
					Printf(FAILED);
				} else
				{
					Printf(PASSED);			
				}
				cnt++;
			}
			Printf("Found %d memory entries\n",cnt);
			Printf("---------------------------------------------\n");

			ForeachNode(&mh->mh_List, header)
			{
				Printf("Free Memory at %x, size %x\n", header, header->mch_Size);

				pMemCFoot footer = (pMemCFoot)((UINT32)header + header->mch_Size - sizeof(MemCFoot));
				Printf("Checking Memory Headers.....");
				if (header->mch_Magic != MCHC_MAGIC)
				{
					Printf(FAILED);
				} else
				{
					Printf(PASSED);			
				}
				Printf("Checking Memory Footers.....");
				if (footer->mcf_Magic != MCHC_MAGIC)
				{
					Printf(FAILED);
				} else
				{
					Printf(PASSED);			
				}
			}
			Printf("---------------------------------------------\n");

			Printf("Checking Stack of running Task\n");
			pTask task = FindTask(NULL);
			Printf("[%s] StackUpper: %d, StackLower: %d, Stack: %d, Stacksize: %d \n", task->tcb_Node.ln_Name, task->tcb_SPUpper, task->tcb_SPLower, /*task->tcb_SavedContext.sp*/ task->tcb_Stack, task->tcb_StackSize);
			Printf("Checking Stackregions.....................");
			if ((task->tcb_SPUpper >= task->tcb_Stack/*task->tcb_SavedContext.sp*/) && (task->tcb_SPLower <= task->tcb_Stack/*task->tcb_SavedContext.sp*/)) Printf(PASSED);
			else Printf(FAILED);
			Printf("Ready______________________\n");
			ForeachNode(&SysBase->TaskReady, task)
			{
				Printf("[%s] StackUpper: %d, StackLower: %d, Stack: %d, Stacksize: %d \n", task->tcb_Node.ln_Name, task->tcb_SPUpper, task->tcb_SPLower, task->tcb_SavedContext.sp, task->tcb_StackSize);
				Printf("Checking Stackregions.....................");
				if ((task->tcb_SPUpper > task->tcb_SavedContext.sp) && (task->tcb_SPLower < task->tcb_SavedContext.sp)) Printf(PASSED);
				else Printf(FAILED);
			}
			Printf("Waiting_____________________\n");
			ForeachNode(&SysBase->TaskWait, task)
			{
				Printf("[%s] StackUpper: %d, StackLower: %d, Stack: %d, Stacksize: %d \n", task->tcb_Node.ln_Name, task->tcb_SPUpper, task->tcb_SPLower, task->tcb_SavedContext.sp, task->tcb_StackSize);
				Printf("Checking Stackregions.....................");
				if ((task->tcb_SPUpper > task->tcb_SavedContext.sp) && (task->tcb_SPLower < task->tcb_SavedContext.sp)) Printf(PASSED);
				else Printf(FAILED);
			}
			
			
#if 0
	UINT8			*tcb_Stack;
	UINT32			tcb_SPLower;
	UINT32			tcb_SPUpper;
	UINT32			tcb_StackSize;	
	task.tcb:
#endif	
			FreeArgs(rdargs);		
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


