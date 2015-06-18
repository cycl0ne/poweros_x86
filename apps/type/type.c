/**
* File: /type．c
* User: cycl0ne
* Date: 2014-11-12
* Time: 11:45 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "dos_asl.h"

#include "ctype.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define MSG_OPTIGN   "Option '%lc' ignored\n"
#define MSG_CANTOPEN "TYPE can't open %s\n"
#define MSG_EXISTS   "%s already exists\n"
#define MSG_NOBOTH   "Type can't do both HEX and NUMBER\n"
#define MSG_NO_FILES "No files to type\n"

#define	VSTRING	"Type 0.1 (12.11.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "FROM/A/M,TO/K,OPT/K,HEX/S,NUMBER/S" CMDREV
#define OPT_FROM   0
#define OPT_TO     1
#define OPT_OPT    2
#define OPT_HEX    3
#define OPT_NUMBER 4
#define OPT_COUNT  5

struct uAnchor {
  struct AnchorPath uA_AP;
  UINT8 uA_Path[256];
};

#define STATE_HEX     0
#define STATE_NUM     1
#define STATE_NONUM   2
#define STATE_NUMCONT 3

#define HEXASC  39
#define HEXSIZE 61
#define OUTSIZE 255

static int typefile(int state, pFileHandle fh, pDOSBase DOSBase, APTR SysBase);

//DOSCALL cmd_type(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	INT32 	opts[OPT_COUNT];

	int rc;
	char *msg;
	int c;
	int state;
	long args[1];
	pFileHandle infile, outfile , oldfile;
	pFileLock lock;
	char *curarg, **argptr;
	struct RDargs *rdargs;
	struct uAnchor *ua;

	rc = RETURN_FAIL;

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

		if (rdargs == NULL) PrintFault(IoErr(), NULL);
		else 
		{
			oldfile = Output();
			
			if ((ua = AllocVec(sizeof(struct uAnchor), MEMF_CLEAR)) == NULL)
			{
				PrintFault(IoErr(), NULL);
			} else
			{
				infile = outfile = NULL;
				ua->uA_AP.ap_Strlen		= 255;
				ua->uA_AP.ap_BreakBits	= SIGBREAKF_CTRL_C;
				msg = (char *)opts[OPT_OPT];
				if (msg)
				{
					while ((c = *msg++))
					{
						c |= 0x20;
						if (c == 'h') opts[OPT_HEX] = 1;
						else if (c == 'n') opts[OPT_NUMBER] = 1;
						else
						{
							opts[OPT_OPT] = msg[-1];
							VPrintf(MSG_OPTIGN, opts+ OPT_OPT);
						}
					}
				}
				
				state = STATE_NONUM;
				if (opts[OPT_HEX])		state = STATE_HEX;
				if (opts[OPT_NUMBER])	state--;
				if (state < 0)
				{
					msg	= MSG_NOBOTH;
					rc	= RETURN_WARN;
				} else
				{
					msg = MSG_NO_FILES;
					argptr = (char **)opts[OPT_FROM];
					rc = 0;
					while (( rc == 0) && (curarg = *argptr++))
					{
						msg = NULL;
						if (MatchFirst(curarg, (struct AnchorPath *)ua))
						{
							rc = IoErr();
							if (rc == ERROR_NO_MORE_ENTRIES) rc = 0;
							args[0] = (INT32)curarg;
							VPrintf(MSG_CANTOPEN, args);
							break;
						}
						rc = 0;
						while(rc == 0) 
						{
							lock = CurrentDir(ua->uA_AP.ap_Current->an_Lock);
							infile = Open((UINT8*)ua->uA_AP.ap_Info.fib_FileName, MODE_OLDFILE);
							rc = IoErr();
							CurrentDir(lock);
							if (infile == NULL) 
							{
								args[0] = (long)ua->uA_AP.ap_Info.fib_FileName;
								VPrintf(MSG_CANTOPEN, args);
								break;
							}

							if (opts[OPT_TO] && (outfile == NULL)) 
							{
								if (!(outfile = Open((UINT8 *)opts[OPT_TO], MODE_NEWFILE)))
								{
									rc=IoErr();
									args[0] = opts[OPT_TO];
									VPrintf(MSG_CANTOPEN, args);
									break;
								}
							}
							if (outfile) SelectOutput(outfile);
							rc = typefile(state, infile, DOSBase, SysBase);
							if (outfile) SelectOutput(oldfile);
							Close(infile);
							infile = NULL;
							if (rc) break;

							if (MatchNext((struct AnchorPath *)ua)) {
								rc = IoErr();
								if (rc == ERROR_NO_MORE_ENTRIES) rc = 0;
								break;
							}
						}
						MatchEnd((struct AnchorPath *)ua);
					}
				}
						
				if (msg) 
				{
					VPrintf(msg, opts);
				} else if (rc) 
				{
					if (rc>=0) PrintFault(rc,NULL);
					if (rc == ERROR_BREAK) rc = RETURN_WARN;
					else rc = RETURN_ERROR;
				}
				
				if (infile)  Close(infile);
				if (outfile) Close(outfile);
				FreeVec(ua);
			}
			FreeArgs(rdargs);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}

static int typefile(int state, pFileHandle fh, pDOSBase DOSBase, APTR SysBase)
{
	int counter;
	int cyc;
	int c = 0;
	long opts[10];
	char outbuf[OUTSIZE];
	char *msg;
	int i;

	msg = NULL;
	counter = cyc = 0;

	while(TRUE) 
	{
		c = FGetC(fh);
		if (c == ENDSTREAMCH) break;
		
		if (state == STATE_HEX) 
		{
			if (!cyc) 
			{
				MemSet(outbuf, ' ', HEXSIZE);
				outbuf[HEXSIZE-1] = 0;
				opts[0] = counter;
				opts[1] = (long)outbuf;
			}
			counter++;
			msg = outbuf+(cyc<<1)+(cyc>>2);
			msg[0] = "0123456789ABCDEF"[c>>4];
			msg[1] = "0123456789ABCDEF"[c&15];

			if (!isprint(c) || (c < 0x1f)) c= '.';

			msg = "%04lx: %s\n";
			outbuf[HEXASC+cyc] = c;
			cyc++;

			if (cyc != 16) continue;
		} else 
		{
			msg = "%5ld %s";
			if (state == STATE_NUM) 
			{
				opts[0] = counter+1;
				opts[1] = (long)outbuf;
			} else 
			{
				msg += 5;
				opts[0] = (long)outbuf;
			}
			outbuf[cyc++] = c;
			if (cyc == OUTSIZE-1) 
			{
				if (state == STATE_NUM)	state = STATE_NUMCONT;
			}
			else if (c == '\n') 
			{
				if (state == STATE_NUMCONT) state = STATE_NUM;
				counter++;
			}
			else
				continue;
			outbuf[cyc] = 0;
		}

		cyc = 0;
		i = VPrintf(msg, opts);
		if (i < 0)return(IoErr());
		if (CheckSignal(SIGBREAKF_CTRL_C)) return ERROR_BREAK;
		msg = NULL;
	}

	if (msg != NULL) 
	{
		if ((state != STATE_HEX) && cyc && outbuf[cyc-1] != '\n') 
		{
			outbuf[cyc++] = '\n';
			outbuf[cyc++] = 0;
		}
		i = VPrintf(msg, opts);
		if (i < 0) return IoErr();
	}
	return 0;
}



