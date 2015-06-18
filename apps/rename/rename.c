/**
* File: /delete.c
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
#include "dos_asl.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"Rename 0.1 (12.01.2015)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define MSG_CATCHALL    "Can't rename %s as %s because "
#define MSG_NODIR       "Destination \"%s\" is not a directory.\n"
#define MSG_RENAME      "Renaming %s as %s\n"

#define TEMPLATE    "FROM/A/M,TO=AS/A,QUIET/S" CMDREV
#define OPT_FROM    0
#define OPT_TO      1
#define OPT_QUIET   2
#define OPT_COUNT   3

struct uAnchor {
	struct AnchorPath uA_AP;
	uint8_t	uA_Path[256];
};
#define  MAX_PATH 256
#define MSGBUFSIZE 512

DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	int         rc;
	int         res2=0;
	int		    loopc;
	long              opts[OPT_COUNT];
	struct RDargs     *rdargs = NULL;
	struct uAnchor    *ua = NULL;
	char              *msgbuf = NULL;
	char              *destpath = NULL;
	char              *srcname = NULL;
	pFileLock         destlock = NULL;
	char              **argptr, *cur;
	char              *destname;
	char              fibstr[sizeof(struct FileInfoBlock)];
	struct FileInfoBlock *fib;
	int				msgblock[2], destisdir;
	pFileLock		sourcelock;

	rc = RETURN_FAIL;

	DOSBase = OpenLibrary("dos.library",0);
	if (DOSBase)
	{
		pUtilBase UtilBase = OpenLibrary("utility.library",0);
		if (!UtilBase)
		{
			CloseLibrary(DOSBase);
			return rc;
		}
			
		if ((msgbuf = AllocVec(MSGBUFSIZE, NULL))              == NULL ||
			(ua = AllocVec(sizeof(struct uAnchor),MEMF_CLEAR)) == NULL ||
			(srcname  = AllocVec(MAX_PATH, NULL      ))        == NULL ||
			(destpath = AllocVec(MAX_PATH, MEMF_CLEAR))        == NULL ) 
		{
			PrintFault(ERROR_NO_FREE_STORE, NULL);
			goto Done;
		}
	
		MemSet((char *)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);
		
		if (rdargs == NULL) 
		{
			res2 = IoErr();
			goto ErrFail;
		}
	
		rc = RETURN_OK;
		
		ua->uA_AP.ap_Strlen = 255;
		ua->uA_AP.ap_Flags = APF_DOWILD;
		ua->uA_AP.ap_BreakBits = SIGBREAKF_CTRL_C;
		argptr = (char **)opts[OPT_FROM];
		if (MatchFirst(*argptr, (struct AnchorPath *)ua)) 
		{
			res2 = IoErr();
			if(res2 == ERROR_OBJECT_NOT_FOUND) 
			{
				opts[OPT_FROM] = (long)*argptr;
				Printf("test\n");
				VPrintf(MSG_CATCHALL, (INT32*)opts);
			}
			goto ErrFail;
		}
		destisdir = FALSE;
		destlock = Lock((char *)opts[OPT_TO], SHARED_LOCK);
		if (destlock) 
		{
			fib = (struct FileInfoBlock *)(((long)fibstr));
			if (Examine(destlock, fib) && (fib->fib_DirEntryType > 0)) destisdir = TRUE;
		}
		if ((!destisdir)                                 &&
			(((ua->uA_AP.ap_Flags & APF_ITSWILD) != 0)   ||
			(argptr[1] != NULL)))
		{
			VPrintf(MSG_NODIR,(INT32*) &opts[OPT_TO]);
			goto Done;
		}

		if(destisdir && !argptr[1]) 
		{
			sourcelock=Lock((char *)argptr[0], SHARED_LOCK);
			if(sourcelock) 
			{
			    if( (SameLock(sourcelock, destlock))==LOCK_SAME) destisdir=FALSE;
				UnLock(sourcelock);
			}
		}
		MatchEnd((struct AnchorPath *)ua);

		if (!destisdir) 
		{
			ParsePattern((char *)*argptr,srcname,MAX_PATH);
			if (! Rename(srcname, (char *)opts[OPT_TO])) 
			{
				res2 = IoErr();
				opts[OPT_FROM] = (long)*argptr;
				Printf("test2\n");
				VPrintf(MSG_CATCHALL, (INT32*)opts);
				SetIoErr(res2);
			}
		} else 
		{
			NameFromLock(destlock, destpath, MAX_PATH);
			destname = &destpath[Strlen(destpath)];

			if (destname[-1] != ':') 
			{
				*destname++ = '/';
			}

			while ((cur = *argptr++))
			{
				if (MatchFirst(cur, (struct AnchorPath *)ua)) 
				{
					res2 = IoErr();
					goto ErrFail;
				}
				do 
				{
					Strcpy(destname, ua->uA_AP.ap_Info.fib_FileName);
					Strcpy(srcname,  (STRPTR)ua->uA_AP.ap_Buf);
					loopc = MatchNext((struct AnchorPath *)ua);
					msgblock[0] = (int)srcname;
					msgblock[1] = (int)destpath;
					
					if (! opts[OPT_QUIET]) VPrintf(MSG_RENAME, msgblock);
					if (! Rename( (char *)srcname, destpath)) 
					{
						res2 = IoErr();
						Printf("test3\n");
						VPrintf(MSG_CATCHALL, msgblock);
						SetIoErr(res2);
						goto ErrFail;
					}
				} while (loopc == NULL);
				MatchEnd((struct AnchorPath *)ua);
			}
		}
ErrFail:
	if(res2) 
	{
		MatchEnd((struct AnchorPath *)ua);
		PrintFault(res2, NULL);
		rc = RETURN_FAIL;
	}
Done:
		if (rdargs)     FreeArgs(rdargs);
		UnLock(destlock);
		FreeVec(ua);
		FreeVec(msgbuf);
		FreeVec(destpath);
		FreeVec(srcname);
		
		CloseLibrary((struct Library *)DOSBase);
	} else
	{
	//OPENFAIL;
	}
	return(rc);
}
