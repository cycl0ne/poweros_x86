/**
* File: /resident．c
* User: cycl0ne
* Date: 2014-10-25
* Time: 04:41 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"

#include "libraries.h"
#include "residents.h"
#include "semaphores.h"
#include "memory.h"
#include "ports.h"
//#include "utility.h"
#include "timer.h"

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

typedef struct DOSBase {
	Library				dos_LibNode;
	APTR				dos_SysBase;
	APTR				dos_UtilBase;
	MinList				dos_DosList;
	List				dos_SegList;
	MinList				dos_CliList;
	UINT32				dos_CliNum;
	SignalSemaphore		dos_DosListLock;
	SignalSemaphore 	dos_EntryLock;
	SignalSemaphore		dos_DeleteLock;

	SignalSemaphore		dos_SegLock;	// Read/Write Access to SegList
	SignalSemaphore		dos_CliLock;	// Read/Write Access to CliList
	struct TimeRequest	dos_TimerIO;
	struct DateStamp	dos_Time;
	APTR				dos_Errors;
	Segment				dos_Shell;
	Segment				dos_Console;
	Segment				dos_ConTTY;
	Segment				dos_FileSystem;
	Segment				dos_RAMHandler;
}DOSBase, *pDOSBase;

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"resident 0.1 (24.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME,FILE,REMOVE/S,ADD/S,REPLACE/S,PURE=FORCE/S,SYSTEM/S" CMDREV
#define OPT_NAME    	0
#define OPT_FILE  	1
#define OPT_REMOVE	2
#define OPT_ADD		3
#define OPT_REPLACE	4
#define OPT_PURE	5
#define OPT_SYSTEM	6
#define OPT_COUNT	7

DOSCALL cmd_resident(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32	res	= 0;
	INT32 	opts[OPT_COUNT];
	INT32	count = 0;
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else 
			{
				UINT8	name_buffer[256];
				STRPTR	name, file, fi;
				name = (STRPTR)name_buffer;
			
				rc = RETURN_OK;
				
				if ((!opts[OPT_FILE]) && (opts[OPT_NAME]))
				{
					file = (STRPTR) opts[OPT_NAME];
					fi = FilePart(file);
					Strcpy((STRPTR)name_buffer, (STRPTR)fi);
				} else if ((opts[OPT_FILE]) && (!opts[OPT_NAME]))
				{
					file = (STRPTR) opts[OPT_FILE];
					fi = FilePart(file);
					Strcpy((STRPTR)name_buffer, (STRPTR)fi);					
				} else
				{
					name = (STRPTR)opts[OPT_NAME];
					file = (STRPTR)opts[OPT_FILE];
				}
				
				if (name)
				{
					pSegment seg = FindSegment(name, NULL, FALSE);
					if (opts[OPT_REMOVE])
					{
						if (seg)
						{
							LockSegment(seg);
							if (seg->seg_Flags <= CMD_INTERNAL)
							{
								seg->seg_Flags = CMD_DISABLED;
							} else if (seg->seg_Count > 1) // 1 because we added the count on Add
							{
								rc = RETURN_WARN;
								res = ERROR_OBJECT_IN_USE;
							} else
							{
								seg->seg_Count--; // We remove our count and delete
								if (RemSegment(seg) == DOSCMD_FAIL)
								{
									seg->seg_Count++;
									rc = RETURN_ERROR;
									res = IoErr();
								}
							}
							
							UnLockSegment(seg);
							if (rc == RETURN_OK)
							{
								UnloadSegment(seg); //FIX: Unloadsegment can fail if someone opens it just in the second.
							}
						} else
						{
							rc = RETURN_WARN;
							res = ERROR_OBJECT_NOT_FOUND;
						}
					} else if (file &&	((opts[OPT_ADD] || 
										(seg->seg_Flags != CMD_DISABLED)) ||
										((!opts[OPT_REPLACE]) && seg->seg_Count >1)))
					{
						pSegment code = LoadSegment(file);
						if (!code)
						{
							res = IoErr();
							if (!res) res = ERROR_OBJECT_NOT_FOUND;
							rc = RETURN_WARN;
						} else
						{
							if (!Stricmp(name, "resident") || !Stricmp(name, "CLI"))
							{
								rc	= 1;
								res = ERROR_OBJECT_WRONG_TYPE;
							} else
							{
								LockSegment(code);
								if (opts[OPT_SYSTEM]) count = -1;
								if (AddSegment(name, code, count) == DOSCMD_FAIL)
								{
									//Printf("Error\n");
									UnLockSegment(code);
									UnloadSegment(code);
									res = IoErr();
									rc = RETURN_ERROR;
								} else
								{
									code->seg_Count++; //Make sure noone unloads it, while in resident list !
									//Printf("count: %d\n", code->seg_Count);
									UnLockSegment(code);
								}
							}
						}
					} else if (file)
					{
						if (opts[OPT_REPLACE])
						{
							if (!seg)
							{
								rc = RETURN_WARN;
								res = ERROR_OBJECT_NOT_FOUND;						
							} else 
							{
								LockSegment(seg);
								if (seg->seg_Count > 1 )
								{
									rc = RETURN_ERROR;
									res = ERROR_OBJECT_IN_USE;
								} else if (seg->seg_Flags <= CMD_INTERNAL)
								{
									seg->seg_Flags = CMD_INTERNAL;
									UnLockSegment(seg);
									FreeArgs(rdargs);
									if (rc)
									{
										if (rc >1) PrintFault(res, NULL);
										SetIoErr(res);
									}
									CloseLibrary(UtilBase);
									CloseLibrary(DOSBase);
									return rc;
								} else if (seg->seg_Flags != CMD_USER)
								{
									rc = RETURN_ERROR;
									res = ERROR_DELETE_PROTECTED;
								}
								UnLockSegment(seg);
								if (rc == RETURN_OK)
								{
									pSegment code = LoadSegment(file);
									if (!code)
									{
										res = ERROR_OBJECT_NOT_FOUND;
										rc = RETURN_WARN;
									} else
									{
#define CMD_SYSTEM		-1
#define CMD_INTERNAL	-2
#define CMD_DISABLED	-999
#define CMD_USER		0
										LockSegment(seg);
										seg->seg_Count--;
										UnLockSegment(seg);
										UnloadSegment(seg);
										code->seg_Count++; //be sure to add our count!
										INT32 flags = opts[OPT_SYSTEM]? CMD_SYSTEM : CMD_USER;
										//Printf("Adding: %s, %x, %d\n",name,code,flags);
										AddSegment(name, code, flags);
									}
								}
								
							}
						}
					}

				} else
				{
//					UINT8 buffer[64];
//					UINT8 buffer2[16];
//					if (Fault(STR_TH_USE_COUNT, NULL, buffer, 64) &&
//					    Fault(STR_NAME, NULL, buffer2, 16))
//						VPrintf(buffer, buffer2);
					Printf("%-18s USE COUNT\n\n","NAME");
					LockSegList();
					pSegment node;
					pList	list = (pList)&DOSBase->dos_SegList;
					ForeachNode(list, node)
					{
						if (CheckSignal(SIGBREAKF_CTRL_C)) 
						{
							PrintFault(ERROR_BREAK, 0);
							break;
						}
						
						switch(node->seg_Flags)
						{
							case CMD_DISABLED:
								Printf("%-18s DISABLED\n", node->seg_Node.ln_Name);
								break;
							case CMD_INTERNAL:
								Printf("%-18s INTERNAL\n", node->seg_Node.ln_Name);
								break;
							case CMD_SYSTEM:
								Printf("%-18s SYSTEM\n", node->seg_Node.ln_Name);
								break;
							case CMD_USER:
							default:
								Printf("%-18s %-4d\n", node->seg_Node.ln_Name, node->seg_Count);
								break;
						}
					}
					UnLockSegList();
				}
				FreeArgs(rdargs);
				if (rc)
				{
					if (rc >1) PrintFault(res, NULL);
					SetIoErr(res);
				}
			}
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}
