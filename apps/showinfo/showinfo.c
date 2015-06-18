/**
* File: /showinfo．c
* User: cycl0ne
* Date: 2014-10-27
* Time: 01:22 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"
#include "execbase_private.h"
#include "dosbase_private.h"

#undef SysBase // Needed, dosbase_private is defining this.
#undef UtilBase

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"showinfo 0.1 (27.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "DEVICES/S,LIBRARIES/S,HANDLERS/S,TASKS/S,IRQ/S,RESIDENT/S,SEGMENTS/S"CMDREV
#define OPT_DEVICES		0
#define OPT_LIBRARIES	1
#define OPT_HANDLERS	2
#define OPT_TASKS		3
#define OPT_IRQ			4
#define OPT_RESIDENT	5
#define OPT_SEGMENTS	6
#define OPT_COUNT		7

//#define PASSED "\033[60G[\033[1;32mPASS\033[0;39m]\n"
//#define FAILED "\033[60G[\033[1;31mFAIL\033[0;39m]\n"

#define BLACK	"\033[1;30m"
#define RED		"\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW	"\033[1;33m"
#define BLUE	"\033[1;34m"
#define MAGENTA	"\033[1;35m"
#define CYAN	"\033[1;36m"
#define WHITE	"\033[1;37m"
#define DEFAULT	"\033[0;39m"

//DOSCALL cmd_resident(APTR SysBase)
DOSCALL main(pSysBase SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library", 0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else 
			{
				rc = RETURN_OK;
				if (opts[OPT_TASKS])
				{
					pTask dev;
					Printf("PowerOS Registered Tasks :\n\n");
					dev = FindTask(NULL);
					Printf(YELLOW"Run ----------------------------\n"DEFAULT);
					Printf("  Name : "YELLOW"%s\n"DEFAULT,dev->tcb_Node.ln_Name);
					Printf("  Prio : %d\n",dev->tcb_Node.ln_Pri);
					Printf("  Type : %X\n",dev->tcb_Node.ln_Type);
					Printf("\n");

					Printf(YELLOW"Ready --------------------------\n"DEFAULT);
					ForeachNode(&SysBase->TaskReady,dev)
					{
						Printf("  Name : "YELLOW"%s\n"DEFAULT,dev->tcb_Node.ln_Name);
						Printf("  Prio : %d\n",dev->tcb_Node.ln_Pri);
						Printf("  Type : %X\n",dev->tcb_Node.ln_Type);
					}
					Printf("\n");
					Printf(YELLOW"Wait  --------------------------\n"DEFAULT);
					ForeachNode(&SysBase->TaskWait,dev)
					{
						Printf("  Name : "YELLOW"%s\n"DEFAULT,dev->tcb_Node.ln_Name);
						Printf("  Prio : %d\n",dev->tcb_Node.ln_Pri);
						Printf("  Type : %X\n",dev->tcb_Node.ln_Type);
						Printf("\n");
					}
					Printf("\n");
				}
				if (opts[OPT_IRQ])
				{
					Printf("PowerOS Registered IRQs :\n\n");
					pInterrupt irq;
					for(int addr=0; addr<16; addr++)
					{
						irq = NULL;
						Printf("Interrupt [%X] :\n",addr);
						ForeachNode(&SysBase->IntVectorList[addr],irq)
						{
							Printf("-------------------------------\n");
							Printf(YELLOW"  Addr : %x\n"DEFAULT,&irq->is_Node);
							Printf(YELLOW"  Name : %s\n"DEFAULT,irq->is_Node.ln_Name);
							Printf(YELLOW"  Prio : %d\n"DEFAULT,irq->is_Node.ln_Pri);
							Printf(YELLOW"  Type : %X\n"DEFAULT,irq->is_Node.ln_Type);
							Printf(YELLOW"  Funct: %x\n"DEFAULT,irq->is_Code);
						}
						Printf("\n");
					}

				}
				if (opts[OPT_RESIDENT])
				{
					pResidentNode res;
					Printf("PowerOS Residents in Kernel :\n\n");
					Printf("-------------------------------\n");
					ForeachNode(&SysBase->ResidentList, res)
					{
						Printf(" Name:     "YELLOW"%s\n"DEFAULT, res->rn_Resident->rt_Name);
						Printf(" Version:  %d    Type:    %d    Pri:    %d\n", res->rn_Resident->rt_Version, res->rn_Resident->rt_Type, res->rn_Resident->rt_Pri);
						//Printf(" IDString: %s\n", res->rn_Resident->rt_IdString);
						Printf("\n");
					}
					Printf("\n");
				}
				
				if (opts[OPT_LIBRARIES])
				{
					pLibrary lib;
					Printf("PowerOS loaded libraries in memory\n\n");
					Printf("-------------------------------\n");
					ForeachNode(&SysBase->LibList, lib)
					{
						Printf(" Name: "YELLOW"%s\n"DEFAULT, lib->lib_Node.ln_Name);
						Printf(" Version:  %d    Type:    %d    Pri:    %d\n", lib->lib_Version, lib->lib_Node.ln_Type, lib->lib_Node.ln_Pri);
						Printf("\n");						
					}
					
				}
				if (opts[OPT_DEVICES])
				{
					pLibrary lib;
					Printf("PowerOS loaded devices in memory\n\n");
					Printf("-------------------------------\n");
					ForeachNode(&SysBase->DevList, lib)
					{
						Printf(" Name: "YELLOW"%s\n"DEFAULT, lib->lib_Node.ln_Name);
						Printf(" Version:  %d    Type:    %d    Pri:    %d\n", lib->lib_Version, lib->lib_Node.ln_Type, lib->lib_Node.ln_Pri);
						Printf("\n");						
					}
					
				}

				if (opts[OPT_SEGMENTS])
				{
					pSegment seg;
					Printf("PowerOS loaded commands in memory (Seglist)\n\n");
					Printf("-------------------------------\n");
					ForeachNode(&DOSBase->dos_SegList, seg)
					{
						Printf(" Name: "YELLOW"%s\n"DEFAULT, seg->seg_Node.ln_Name);
						Printf(" Flags:  %d    Count:    %d\n", seg->seg_Flags, seg->seg_Count);
						Printf("\n");												
					}
				}

				if (opts[OPT_HANDLERS])
				{
					pDosEntry de;
					Printf("PowerOS DosEntries \n\n");
					Printf("-------------------------------\n");
					ForeachNode(&DOSBase->dos_DosList, de)
					{
						Printf(" Name: "YELLOW"%s\n"DEFAULT, de->de_Node.ln_Name);
						Printf(" Type:  %d", de->de_Type);
						switch (de->de_Type)
						{
							case DLT_DEVICE:
								Printf("(Handler)\n");
								break;
							case DLT_VOLUME:
								Printf("(Volume)\n");
								break;
							case DLT_DIRECTORY:
								Printf("(Assign/MultiAssign) %s\n", de->de_Misc.assignNode.de_AssignName);
								break;
							case DLT_LATE:
								Printf("(Late Assign) %s\n", de->de_Misc.assignNode.de_AssignName);
								break;
							case DLT_NONBINDING:
								Printf("(NonBinding Assign) %s\n", de->de_Misc.assignNode.de_AssignName);
								break;
							default:
								Printf("(Unknown type)\n");
								break;
						}
						Printf("\n");												
					}
				}

/*
 * Your console code goes here!
 */				
				
				FreeArgs(rdargs);
			}			
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}


