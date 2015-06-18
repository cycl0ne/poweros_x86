/**
* File: sort.c
* User: srinivas
* Date: 2014-10-28
* Time: 08:40 PM
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

#define	VSTRING	"resident 0.1 (28.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NUMBERS/M/N" CMDREV
#define OPT_NUMBERS 0
#define OPT_COUNT 1

BOOL less_or_equal(Node* a, Node* b)
{
	if(a->ln_Type <= b->ln_Type)
		return TRUE;
	else
		return FALSE;
}

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	//INT32	res	= 0;
	INT32 	opts[OPT_COUNT];

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
				rc = RETURN_OK;
/*
 * Your console code goes here!
 */
				if (opts[OPT_NUMBERS])
				{
					UINT32* nums;
					UINT32 total_nums;
					UINT32 i = 0;
					List l;
					Node* n;
					pNodeCmpLe p_le = &less_or_equal;

					nums = (UINT32*)opts[OPT_NUMBERS];
					total_nums = nums[0];

					Printf("total_nums = %d\n", total_nums);

					NewList(&l);

					Printf("Unsorted numbers:\n");
					i = 1;
					while (i <= total_nums)
					{
						Printf("%d ", nums[i]);

						n = NULL;
						n = (Node*) AllocVec(sizeof(Node), MEMF_FAST);

						if(n == NULL)
						{
							Printf ("no memory\n");
							return 1;
						}

						n->ln_Type = nums[i];
						AddHead(&l, n);

						i++;
					}
					Printf("\n");

					//sort it
					QuickSort(&l, p_le);

					//print it again
					Printf("Sorted numbers:\n");
					ForeachNode(&l,n)
					{
						Printf("%d ",n->ln_Type);
					}
					Printf("\n");

					//test it!
					INT32 prev = -1;
					i = 0;
					ForeachNode(&l,n)
					{
						if(prev == -1)
						{
							prev = n->ln_Type;
							i++;
							continue;
						}

						if(!(n->ln_Type >= prev))
							break;

						prev = n->ln_Type;
						i++;
					}

					if(i == total_nums)
						Printf("Sort Succeed\n");
					else
						Printf("Sort Failed\n");

					//free list
					while((n = (Node *)GetHead(&l)))
					{
						Remove(n);
						FreeVec(n);
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
