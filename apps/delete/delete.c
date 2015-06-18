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

#define	VSTRING	"Delete 0.1 (12.01.2015)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define MSG_NO_FILES	"No file to delete\n"
#define MSG_DEVICE	"%s is a device and cannot be deleted\n"
#define MSG_DELETED	"  Deleted\n"
#define MSG_NOT_DELETED "  Not Deleted"

#define TEMPLATE	"FILE/M/A,ALL/S,QUIET/S,FORCE/S" CMDREV
#define OPT_PATTERN	0
#define OPT_ALL 	1
#define OPT_QUIET	2
#define OPT_FORCE	3
#define OPT_COUNT	4

struct Global {
	APTR DOSBase;
	APTR SysBase;
	pUtilBase	UtilBase;
	long opts[OPT_COUNT];
	long dcount;
	pFileLock dlock;
	long rc;
	long result2;
};

int KillList(struct Global *, char *, pFileLock, STRPTR);

//int cmd_delete(void)

  /*-------------------------------------------------------------------------*/
  /* First, clear the global structure so we don't have any garbage floating.*/
  /*-------------------------------------------------------------------------*/

DOSCALL main(APTR SysBase)
{
	struct Global global;
	pDOSBase	DOSBase;
	struct RDargs *rdargs;

	pHandlerProc dproc;
	char *curarg, **argptr;
	
	/* variables added to deal with absolute drive name problems */
	APTR *prelocks = NULL;
	STRPTR *names = NULL;
	APTR *arglock;
	uint16_t i,j;
	char oldCh;
	uint16_t argcnt;
	BOOL argmatched;
	STRPTR argname;
	uint16_t numsources = 0;
	STRPTR *devname;
	APTR oldCD;

	MemSet(&global, 0,sizeof(struct Global));
	global.rc = RETURN_WARN;

	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		pUtilBase UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
		    global.DOSBase = DOSBase;
		    global.SysBase = SysBase;
		    global.UtilBase= UtilBase;
			rdargs = ReadArgs(TEMPLATE, (UINT32 *)global.opts);
		
			if (rdargs == NULL)
			{
				PrintFault(IoErr(), NULL);
			} else
			{
		        argptr = (char **) global.opts[OPT_PATTERN];
		        numsources = 0;
		        while (*argptr++) numsources++;
	
		        /* allocate an array to hold locks on the things to delete */
		        if (!(prelocks = AllocVec((uint32_t)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
		        {
		            PrintFault(ERROR_NO_FREE_STORE,NULL);
		            goto Done;
		        }
	
				/* allocate an array to hold the device names of the things to delete */
				if (!(names = AllocVec((uint32_t)numsources*4,MEMF_CLEAR|MEMF_PUBLIC)))
				{
				    PrintFault(ERROR_NO_FREE_STORE,NULL);
				    goto Done;
				}
	
				argptr = (char **) global.opts[OPT_PATTERN];
				argcnt = 0;
				while (argptr[argcnt])
				{
					/* scan for device name */
					argname = argptr[argcnt];
					i = 0;
					while ((argname[i] != ':') && (argname[i])) i++;
	
					/* got a colon, so its a device name */
					if (argname[i] == ':')
					{
		                i++;
		                oldCh = argname[i];
		                argname[i] = 0;
						/* see if this device name has already been used */
						argmatched = FALSE;
						j = 0;
						while (j <= argcnt)
						{
			                if ((names[j]) && (Stricmp(argname,names[j]) == 0))
			                {
			                    argmatched = TRUE;
			                    break;
			                }
	    	                j++;
		                }
	
		                if (argmatched)
		                {
		                    prelocks[argcnt] = DupLock(prelocks[j]);
		                }
		                else
		                {
		                    prelocks[argcnt] = Lock(argname,ACCESS_READ);
		                }
		
		                if (prelocks[argcnt])
		                {
		                    if (!(names[argcnt] = AllocVec(Strlen(argname)+1,MEMF_CLEAR|MEMF_PUBLIC)))
		                    {
		                        PrintFault(ERROR_NO_FREE_STORE,NULL);
		                        goto Done;
		                    }
		                    Strcpy(names[argcnt],argname);
		
		                    argname[i] = oldCh;
		                    Strcpy(argname,&argname[i]);
		                }
		            }
		            argcnt++;
	    	    }
	
		
				arglock = &prelocks[0];
				devname = &names[0];
				argptr = (char **)global.opts[OPT_PATTERN];
				
				while ((curarg = ( *argptr++))) 
				{
					if (*arglock) oldCD = CurrentDir(*arglock);
			
					if ((dproc = ObtainHandler(curarg)) == NULL) 
					{
						if (*arglock) CurrentDir(oldCD);
						PrintFault(IoErr(), NULL);
						global.dcount++;
					} else 
					{
						ReleaseHandler(dproc);
						if (*arglock) CurrentDir(oldCD);
				
						if ((!curarg[0]) && (*devname))
						{
							VPrintf(MSG_DEVICE, (INT32 *)devname);
							global.dcount++;
							break;
						} else 
						{
							if (KillList(&global, curarg, *arglock, *devname) != NULL) 
							{
								break;
							}
						}
					}
					arglock++;
					devname++;
				}
				FreeArgs(rdargs);
			}
		
			if (!(global.dcount)) 
			{
				PutStr(MSG_NO_FILES);
				global.rc = RETURN_WARN;
			}
		
		Done:
			if (prelocks)
			{
			    i = 0;
			    while (i < numsources) UnLock(prelocks[i++]);
			    FreeVec(prelocks);
			}
	
			if (names)
			{
				i = 0;
				while (i < numsources) FreeVec(names[i++]);
				FreeVec(names);
			}
			
			if (global.rc) SetIoErr(global.result2);
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	} else {
	//OPENFAIL;
	}
	return global.rc;
}
#define ENVMAX 256
/*---------------------------------------------------------------------------*/
/* This routine is used to kill all the files which match a specific pattern */
/* whether the pattern includes wildcards or not.			     */
/*---------------------------------------------------------------------------*/
int KillList(struct Global *global, char *name, pFileLock lock, STRPTR devname)
{
	struct AnchorPath *ua;
	BOOL skip1,printflag=TRUE;
	int32_t temprc=0,trc,loopc;
	pDOSBase DOSBase;
	APTR oldlock, curlock, flock;
	char *cachename;
	char cachefull[ENVMAX]; //ENVMAX
	APTR oldCD;
	pSysBase SysBase = global->SysBase;
	DOSBase = global->DOSBase;
	pUtilBase UtilBase = global->UtilBase;
	global->rc = RETURN_OK;

  /*-------------------------------------------------------------------------*/
  /* Try to allocate a chunk of memory to use with the pattern match stuff.  */
  /*-------------------------------------------------------------------------*/
	if ((ua = AllocVec(sizeof(struct AnchorPath)+ENVMAX,
	MEMF_CLEAR|MEMF_PUBLIC)) == NULL) 
	{
		PrintFault(IoErr(),NULL);
		global->rc = RETURN_FAIL;
	} else 
	{
		/*-----------------------------------------------------------------------*/
		/* Now we have to set up the pattern match structure.		     */
		/*-----------------------------------------------------------------------*/
		/* Set the flag which tells the pattern matcher to decode wild cards.    */
		/*-----------------------------------------------------------------------*/
		ua->ap_Flags = APF_DOWILD;
		ua->ap_Strlen = ENVMAX; //ENVMAX;
		
		/*-----------------------------------------------------------------------*/
		/* Tell the matcher to stop if the user presses Ctrl-C.		     */
		/*-----------------------------------------------------------------------*/
		ua->ap_BreakBits = SIGBREAKF_CTRL_C;
		
		/*-----------------------------------------------------------------------*/
		/* Finally, call the matcher.					     */
		/*-----------------------------------------------------------------------*/
		if (lock) oldCD = CurrentDir(lock);
		loopc = MatchFirst(name,ua);

		/*-----------------------------------------------------------------------*/
		/* Process all matches returned.					     */
		/*-----------------------------------------------------------------------*/
		while ((loopc == 0) && ((temprc == 0) ||
	    	(temprc == ERROR_OBJECT_IN_USE) ||
	    	(temprc == ERROR_DIRECTORY_NOT_EMPTY) ||
	    	(temprc == ERROR_DELETE_PROTECTED))) 
	    {
			/*---------------------------------------------------------------------*/
			/* 'skip1' is a flag which tells us whether or not to try deleting a   */
			/* subdirectory.	Basically, skip it if we have not deleted all the    */
			/* files under it, delete it if we have cleaned it out.  This only has */
			/* validity if the user used the 'ALL' keyword on the command line.    */
			/*---------------------------------------------------------------------*/
			skip1 = FALSE;

			/*---------------------------------------------------------------------*/
			/* Make a copy of this file's name with full path.                     */
			/*---------------------------------------------------------------------*/
			if ((curlock = DupLock(ua->ap_Current->an_Lock)) == NULL) 
			{
				/*-------------------------------------------------------------------*/
				/* If we can't duplicate the lock, something is *seriously* wrong!   */
				/* Let's set some error codes and BAIL OUT!                          */
				/*-------------------------------------------------------------------*/
				temprc = 1L;
				break;
			} else 
			{
				Strcpy(cachefull, (char *)ua->ap_Buf);
				cachename = FilePart(cachefull);
				//Printf("cachefull: %s [%s], cachename %s\n", cachefull, ua->ap_Buf, cachename);

	if (((ua->ap_Info.fib_DirEntryType > 0) &&
	     (ua->ap_Info.fib_DirEntryType < 3)) &&
	    global->opts[OPT_ALL]) {

	  /*-----------------------------------------------------------------*/
	  /* The flag APF_DIDDIR tells us that we are 'backing out' of a     */
	  /* subdirectory... in other words, we have deleted all sub files   */
	  /* of that directory and we are moving back to its parent.  If     */
	  /* this flag is set, we need to clear it, and leave skip1 alone so */
	  /* this subdir gets deleted as the user specified.		     */
	  /*-----------------------------------------------------------------*/
	  if (!(ua->ap_Flags & APF_DIDDIR)) {

	    /*---------------------------------------------------------------*/
	    /* If we are deleting ALL files, tell the matcher to enter this  */
	    /* directory; we also need to skip trying to delete this file    */
	    /* until we exit it, as explained above.			     */
	    /*---------------------------------------------------------------*/
	    ua->ap_Flags |= APF_DODIR;
	    skip1 = TRUE;
	    UnLock(curlock);
	  }
	  ua->ap_Flags &= ~APF_DIDDIR;
	}
      }

      /*---------------------------------------------------------------------*/
      /* Now get the next file which matches the given pattern. 	     */
      /*---------------------------------------------------------------------*/
      loopc = MatchNext(ua);

      /*---------------------------------------------------------------------*/
      /* Now, unless we are skipping the current file, we need to try to     */
      /* delete it.							     */
      /*---------------------------------------------------------------------*/
      if (!skip1) {

	/*-------------------------------------------------------------------*/
	/* Gather info and set current directory of file we wish to delete.  */
	/*-------------------------------------------------------------------*/
	oldlock = CurrentDir(curlock);
//	Printf("locking cachename: %s\n", cachename);
	flock = Lock(cachename,ACCESS_READ);
	if (flock == NULL) {
	  /* There was an error on the current file.. */
	  temprc = IoErr();
	  trc = 0L;
	  if (global->opts[OPT_QUIET] == NULL) {
	    if (devname)
	        PutStr(devname);
	    PutStr(cachefull);
	  }
	  SetIoErr(temprc);
	} else {
	  UnLock(flock);

	  /*-----------------------------------------------------------------*/
	  /* Unless we are being vewy qwiet, print the name of the file we   */
	  /* are about to try to delete.				     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] == NULL) {
	    if (devname)
	        PutStr(devname);
	    PutStr(cachefull);
	  }

	  /*-----------------------------------------------------------------*/
	  /* If we got this far, we can assume we are going to exit with a   */
	  /* return code, whether we actually delete the file or not.	     */
	  /*-----------------------------------------------------------------*/
	  global->rc = RETURN_OK;

	  /*-----------------------------------------------------------------*/
	  /* KILL IT!							     */
	  /*-----------------------------------------------------------------*/
	  global->dcount++;

//Printf("Trying to delete [%s]\n", cachename);
//makedir test2 test2/test3 test2/test3/test4
	  trc=DeleteFile(cachename);
	  if(trc == NULL) 
	  { /* failed for some reason */
	    global->result2 =IoErr();
	    if((global->result2 == ERROR_DELETE_PROTECTED)&& global->opts[OPT_FORCE]) 
	    {

	      global->result2=0; /* ready for second try */
	      SetProtection(cachename,0x0); /* try again */
	      trc=DeleteFile(cachename);
	    }
	  }
	} /* Match to flock == NULL else clause above */
	CurrentDir(oldlock);
	UnLock(curlock);

	if(trc != NULL) {
	  /*-----------------------------------------------------------------*/
	  /* If it worked, tell everybody!  Unless we are in QUIET mode, of  */
	  /* course!							     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] == NULL) PutStr(MSG_DELETED);
	  global->rc=RETURN_OK;
	  global->result2=0;
	}
	else {
	  /*-----------------------------------------------------------------*/
	  /* If we aren't able to kill the file, we need to tell 'em about   */
	  /* it and go on.						     */
	  /*-----------------------------------------------------------------*/
	  temprc = IoErr();
	  global->result2 = temprc;

	  /*-----------------------------------------------------------------*/
	  /* If we have been quiet up to now, we need to go ahead and print  */
	  /* the name out.  This was a rather annoying ommision of the BCPL  */
	  /* version, in that if you told it to be quiet, it would... except */
	  /* when it printed an error message, you never knew WHERE the      */
	  /* error had occured. 					     */
	  /*-----------------------------------------------------------------*/
	  if (global->opts[OPT_QUIET] != NULL) 
	  {
	    if (devname) PutStr(devname);
	    PutStr(cachefull);
	  }
//	  Printf("Here i am, rock me like a hurricane (%s, %s)\n", devname, cachefull);
	  PrintFault(temprc, MSG_NOT_DELETED);
	  printflag=FALSE; /* don't print the message again */
	}
      }
    }

    /*-----------------------------------------------------------------------*/
    /* If the only error we had was that we ran out of files to delete, we   */
    /* are doing fine!	Set the return code so the user knows everything     */
    /* went well.							     */
    /*-----------------------------------------------------------------------*/
    if (temprc == ERROR_NO_MORE_ENTRIES) {
      temprc = NULL;
      global->rc = RETURN_OK;
    }

    /*-----------------------------------------------------------------------*/
    /* If the user hit Ctrl-C, we have to warn him that he is being rude!    */
    /*-----------------------------------------------------------------------*/
    if (temprc == ERROR_BREAK) {
      temprc = NULL;
      PrintFault(IoErr(),NULL);
      global->rc = RETURN_WARN;
      global->dcount++;
    }

    /*-----------------------------------------------------------------------*/
    /* If there was any other error, tell the user about it and leave!	     */
    /*-----------------------------------------------------------------------*/
    if (temprc) {
      if(printflag)PrintFault(IoErr(),NULL);
      global->rc = RETURN_FAIL;
      global->dcount++;
    }

    /*-----------------------------------------------------------------------*/
    /* We have to clean up after ourselves.				     */
    /*-----------------------------------------------------------------------*/
    MatchEnd(ua);

    if (lock)
        CurrentDir(oldCD);

    FreeVec(ua);
  }
  return(global->rc);
}
