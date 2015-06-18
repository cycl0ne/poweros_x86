/**
 * @file why.c
 *
 * CLI Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"
#define MSG_STATS  "Type  Available    In-Use   Maximum   Largest\n" \
                   "chip%11ld %9ld %9ld %9ld\n"                      \
                   "fast%11ld %9ld %9ld %9ld\n"                      \
                   "total%10ld %9ld %9ld %9ld\n"
#define MSG_NOTBOTH "only one of CHIP, FAST, or TOTAL allowed\n"


#define TEMPLATE  "CHIP/S,FAST/S,TOTAL/S,FLUSH/S"
#define OPT_CHIP  0
#define OPT_FAST  1
#define OPT_TOTAL 2
#define OPT_FLUSH 3
#define OPT_COUNT 4

DOSCALL cmd_avail(APTR SysBase)
{
	pDOSBase DOSBase;
	
	UINT32 			opts[OPT_COUNT];
	struct RDargs*	rdargs;
	UINT8*			template = (UINT8*)TEMPLATE;
	UINT32			max_chip, max_fast;
	UINT32			avail_chip, avail_fast;
	UINT32			large_chip, large_fast;
	INT32 			rc = RETURN_FAIL;
	INT32			out_array[13];
   
	if ((DOSBase = OpenLibrary("dos.library", 0))!=NULL)
	{
	
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(template, opts);
		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else {
			rc = RETURN_OK;
			if(opts[OPT_FLUSH])
			{
				INT32 i = 0;
				while (i < 10)
				{
					APTR mem = AllocVec(0x7ffffff0,MEMF_PUBLIC);
					FreeVec( AllocVec(0x7ffffff0,MEMF_PUBLIC) );
					i++;
				}
			}
			Forbid();
			max_chip   = AvailMem (MEMF_CHIP | MEMF_TOTAL);
			avail_chip = AvailMem (MEMF_CHIP);
			large_chip = AvailMem (MEMF_CHIP | MEMF_LARGEST);

			max_fast   = AvailMem (MEMF_FAST | MEMF_TOTAL);
			avail_fast = AvailMem (MEMF_FAST);
			large_fast = AvailMem (MEMF_FAST | MEMF_LARGEST);
			Permit ();

			STRPTR txt = "%ld\n";
			out_array[0] = avail_chip;

			if (opts[OPT_TOTAL])
			{
				if (opts[OPT_FAST] || opts[OPT_CHIP])
				{
					PutStr(MSG_NOTBOTH);
					rc = RETURN_WARN;
				} else
				{
					out_array[0] += avail_fast;
					VPrintf(txt, out_array);
				}
			} else if (opts[OPT_FAST])
			{
				if (opts[OPT_CHIP])
				{
					PutStr(MSG_NOTBOTH);
					rc = RETURN_WARN;
				} else
				{
					VPrintf(txt, &avail_fast);
				}
			} else if (opts[OPT_CHIP])
			{
				VPrintf(txt, &avail_chip);
			} else if (!opts[OPT_CHIP])
			{
				txt =  MSG_STATS;
				out_array[ 1] = max_chip-avail_chip;
				out_array[ 2] = max_chip;
				out_array[ 3] = large_chip;

				out_array[ 4] = avail_fast;
				out_array[ 5] = max_fast-avail_fast;
				out_array[ 6] = max_fast;
				out_array[ 7] = large_fast;

				out_array[ 8] = avail_chip+avail_fast;
				out_array[ 9] = out_array[1]+out_array[5];
				out_array[10] = max_chip+max_fast;
				out_array[11] = large_chip;
				if (large_fast > large_chip) out_array[11] = large_fast;
				VPrintf(MSG_STATS, out_array);
			}
			FreeArgs(rdargs);		
		}
		CloseLibrary(DOSBase);
	} else
		SetIoErr(ERROR_INVALID_RESIDENT_LIBRARY);
	return rc;
}

