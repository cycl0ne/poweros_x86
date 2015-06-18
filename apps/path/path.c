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

#define	VSTRING	"Path 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE  "PATH/M,ADD/S,SHOW/S,RESET/S,REMOVE/S,QUIET/S" CMDREV

#define OPT_PATH     0
#define OPT_ADD      1
#define OPT_SHOW     2
#define OPT_RESET    3
#define OPT_SUBTRACT 4
#define OPT_QUIET    5
#define OPT_COUNT    6

//DOSCALL cmd_path(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];
	UINT8	nodeName[256];
	FileInfoBlock	fib;
	pComLinInt		cli;
//	pProcess		this = FindProcess(NULL);
	pMinList		plist;
	INT8			**argptr, *curarg;
	INT32 			i;
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else 
		{
			cli = Cli();
			for (i=0; i<OPT_QUIET; i++) if (opts[i]) break;
			if (i >= OPT_QUIET) opts[OPT_SHOW] = -1;
			
			if ((opts[OPT_ADD] != NULL) && (opts[OPT_SUBTRACT] != NULL))
			{
				PrintFault(118, NULL);
				FreeArgs(rdargs);
				return rc;
			}
			
			plist = &cli->cli_CommandDir;
			
			if (opts[OPT_RESET])
			{
				pPathNode	pnode, pnext;
				ForeachNodeSafe(plist, pnode, pnext)
				{
					Remove((pNode)pnode);
					UnLock(pnode->pl_Lock);
					FreeVec(pnode);
				}
			}
			
			argptr = (INT8 **)opts[OPT_PATH];
			pPathNode tmpnode = NULL;
			
			while (argptr && (curarg = *argptr++))
			{
				tmpnode = AllocVec(sizeof(PathNode), MEMF_PUBLIC|MEMF_CLEAR);
				if (!tmpnode)
				{
					PrintFault(IoErr(), NULL);
					FreeArgs(rdargs);
					return rc;
				}
				
				tmpnode->pl_Lock = Lock((STRPTR)curarg, ACCESS_READ);
				if (!tmpnode)
				{
					PrintFault(IoErr(), (STRPTR)curarg);
					FreeVec(tmpnode);
					FreeArgs(rdargs);
					return rc;
				}
				if (Examine(tmpnode->pl_Lock, &fib) == NULL)
				{
					PrintFault(IoErr(), (STRPTR)curarg);
					FreeVec(tmpnode);
					UnLock(tmpnode->pl_Lock);
					FreeArgs(rdargs);
					return rc;
				}
				if (fib.fib_DirEntryType < 0)
				{
					SetIoErr(ERROR_OBJECT_WRONG_TYPE);
					PrintFault(IoErr(), (STRPTR)fib.fib_FileName);
					FreeVec(tmpnode);
					UnLock(tmpnode->pl_Lock);
					FreeArgs(rdargs);
					return rc;					
				}

				if (!IsListEmpty((pList)plist))
				{
					// Search List for duplicates
					pPathNode	pnode, pnext;
					ForeachNodeSafe(plist, pnode, pnext)
					{
						if (SameLock(pnode->pl_Lock, tmpnode->pl_Lock) == LOCK_SAME)
						{
							if (opts[OPT_SUBTRACT] == NULL)
							{
								SetIoErr(ERROR_LOCK_COLLISION);
								PrintFault(IoErr(), (STRPTR)curarg);
								FreeVec(tmpnode);
								UnLock(tmpnode->pl_Lock);
								FreeArgs(rdargs);
								return rc;
							} else 
							{
								Remove((pNode)pnode);
								UnLock(pnode->pl_Lock);
								FreeVec(pnode);
								break;
							}
						}
					}
				}
				
				if (opts[OPT_SUBTRACT] == NULL)
				{
					AddHead((pList)plist, (pNode)tmpnode);
					break;
				} else
				{
					FreeVec(tmpnode);
					UnLock(tmpnode->pl_Lock);
				}
			}

			rc = RETURN_OK;

			if (opts[OPT_SHOW])
			{
				PrintFault(STR_CURRENT_DIR, NULL);
				pPathNode	pnode;
				ForeachNode((pList)plist, pnode)
				{
					if(CheckSignal(SIGBREAKF_CTRL_C)) 
					{
						PrintFault(304, NULL);
						FreeArgs(rdargs);
						return rc;
					}
					if (NameFromLock(pnode->pl_Lock, (STRPTR)nodeName, 256))
					{
		                PutStr((STRPTR)nodeName);
        		        PutStr("\n");
					} else
					{
						PrintFault(IoErr(), NULL);
					}					
				}
				PutStr("C:\n");                  /* Path ends at 'C:' assignment */
      		}

			FreeArgs(rdargs);
		}			
		CloseLibrary(DOSBase);
	}
	return rc;
}

