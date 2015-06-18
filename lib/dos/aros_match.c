/**
 * @file aros_match.c
 *
 * Taken from AROS, until we finish our own MatchMaking
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

/*
    Copyright © 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Support functions for MatchFirst/MatchNext/MatchEnd
    Lang: english
*/

#define BYTE	INT8
#define LONG	INT32
#define WORD	INT16
#define UBYTE	UINT8

#include "aros.h"

#define COMPTYPE_NORMAL  1 
#define COMPTYPE_PATTERN 2
#define COMPTYPE_UNKNOWN 3

static struct AChain *Match_AllocAChain(LONG extrasize, pDOSBase DOSBase)
{
    return AllocVec(sizeof(struct AChain) + extrasize, MEMF_PUBLIC | MEMF_CLEAR);
}

/****************************************************************************************/

static void Match_FreeAChain(struct AChain *ac, pDOSBase DOSBase)
{
    FreeVec(ac);
}


/****************************************************************************************/

static void RemoveTrailingSlash(pDOSBase DOSBase, STRPTR s)
{
    LONG len = Strlen(s);

    if (len >= 2)
    {
        if ((s[len - 1] == '/') &&
            ((s[len - 2] != '/') && (s[len - 2] != ':')))
        {
            s[len - 1] = '\0';
        }
    }
}

static LONG Match_BuildAChainList(const STRPTR pattern, struct AnchorPath *ap,
                           struct AChain **retac, pDOSBase DOSBase)
{
    struct AChain       *baseac = 0, *prevac = 0, *ac;
    STRPTR              patterncopy = 0;
    STRPTR              patternstart, patternend, patternpos;
    LONG                len, error = 0;
    WORD                comptype = COMPTYPE_UNKNOWN;
    WORD                compcount = 0;
    WORD                i;
    UBYTE               c;

    *retac = 0;

    len = Strlen(pattern);

    patterncopy = AllocVec(len + 1, MEMF_PUBLIC);
    if (!patterncopy)
    {
        error = ERROR_NO_FREE_STORE;
        goto done;
    }
    
    Strcpy(patterncopy, pattern);

    RemoveTrailingSlash(DOSBase, patterncopy);

    patternstart = patterncopy;

    patternpos = Strchr(patterncopy, ':');
    if (!patternpos)
    {
        comptype = COMPTYPE_UNKNOWN;
        patternpos = patternstart;
        patternend = patternstart;
    }
    else
    {
        comptype = COMPTYPE_NORMAL;
        patternend = patternpos++;
        compcount = 1;
    }

    do
    {
        for(;;)
        {
            c = *patternpos;
            if (c == '/')
            {
                if (comptype == COMPTYPE_UNKNOWN)
                {
                    comptype = COMPTYPE_NORMAL;
                    patternend = patternpos;
                }
                else if (comptype == COMPTYPE_NORMAL)
                {
                    patternend = patternpos;
                    compcount++;
                }
                if (comptype == COMPTYPE_PATTERN)
                {
                    patternend = patternpos;
                    break;              
                }
            }
            else if (c == '\0')
            {
                if (comptype == COMPTYPE_UNKNOWN)
                {
                    comptype = COMPTYPE_NORMAL;
                    patternend = patternpos;
                    break;
                }
                if (comptype == COMPTYPE_NORMAL)
                {
                    compcount++;
                    break;
                }
                patternend = patternpos;
                break;
            }
            else if ((c == '#') ||
                     (c == '~') ||
                     (c == '[') ||
                     (c == ']') ||
                     (c == '?') ||
                     (c == '*') ||
                     (c == '(') ||
                     (c == ')') ||
                     (c == '|') ||
                     (c == '%'))
            {
                if (comptype == COMPTYPE_NORMAL)
                {
                    break;
                }
                comptype = COMPTYPE_PATTERN;
            }
            
            patternpos++;

        } /* for(;;) */

        len = (LONG)(patternend - patternstart + 2);
        if (comptype == COMPTYPE_PATTERN) len = len * 2 + 2;

        ac = Match_AllocAChain(len, DOSBase);
        if (!ac)
        {
            error = ERROR_NO_FREE_STORE;
            goto done;
        }

        if (comptype == COMPTYPE_NORMAL)
        {
            if (*patternend == '\0')
            {
                Strcpy(ac->an_String, patternstart);
            } else {
                c = patternend[1];
                patternend[1] = '\0';
                Strcpy(ac->an_String, patternstart);
                patternend[1] = c;
            }
            
        } /* if (comptype == COMPTYPE_NORMAL) */
        else
        {
            if (*patternend == '\0')
            {
                i = ParsePatternNoCase(patternstart, ac->an_String, len);
                if (i == 0)
                {
                    /* It is not a pattern, although we guessed it was one.
                       Do the strcpy, otherwise we have uppercase stuff in
                       ac->an_String because of ParsePatternNOCASE() */
                    Strcpy(ac->an_String, patternstart);
                }
            }
            else
            {
                c = patternend[1];
                patternend[1] = '\0';
                i = ParsePatternNoCase(patternstart, ac->an_String, len);
                if (i == 0)
                {
                    /* It is not a pattern, although we guessed it was one.
                       Do the strcpy, otherwise we have uppercase stuff in
                       ac->an_String because of ParsePatternNOCASE() */
                    Strcpy(ac->an_String, patternstart);
                }
                patternend[1] = c;
            }
            
            if (i == -1)
            {
                error = ERROR_BAD_TEMPLATE;
                Match_FreeAChain(ac, DOSBase);ac = 0;
                goto done;
            }
            
            if (i)
            {
                ac->an_Flags |= DDF_PatternBit;
                ap->ap_Flags |= APF_ITSWILD;
            }
            
        } /* if (comptype == COMPTYPE_NORMAL) else ... */

        RemoveTrailingSlash(DOSBase, ac->an_String);

        if (!prevac)
        {
            baseac = ac;
        }
        else
        {
            prevac->an_Child = ac;
            ac->an_Parent = prevac;
        }

        prevac = ac;

        patternpos = patternend;
        comptype = COMPTYPE_UNKNOWN;
        patternstart = patternend = patternpos + 1;
        compcount = 0;

    } while (*patternpos++ != '\0');

done:
    FreeVec(patterncopy);

    if (!error)
    {
#if MATCHFUNCS_NO_DUPLOCK
        /*
        * No DupLock() here, because then we would have to UnLock it in
        * MatchEnd and we would not know any valid lock to which we could
        * CurrentDir after, because we must make sure there is a valid
        * CurrentDir after MatchEnd.
        */
        
        baseac->an_Lock = CurrentDir(0);
        CurrentDir(baseac->an_Lock);
#endif
        
        *retac = baseac;
    }
    else
    {
        ap->ap_Flags |= APF_NOMEMERR;
        
        if (baseac)
        {
            #define nextac prevac /* to not have to add another variable */

            ac = baseac;
            while(ac)
            {
                nextac = ac->an_Child;
                Match_FreeAChain(ac, DOSBase);
                ac = nextac;
            }
        }
    }

    return error;
}

/******************************************************************************/

static LONG Match_MakeResult(struct AnchorPath *ap, pDOSBase DOSBase)
{
    LONG error = 0;

    ap->ap_Info = ap->ap_Current->an_Info;
    if (ap->ap_Strlen)
    {
        struct AChain *ac;

        ap->ap_Buf[0] = 0;
        
        for(ac = ap->ap_Base; (ac && !error); ac = ac->an_Child)
        {
            if (!AddPart(ap->ap_Buf, 
                         ((ac->an_Flags & DDF_PatternBit) ? ac->an_Info.fib_FileName : ac->an_String),
                         ap->ap_Strlen))
            {
                error = IoErr();
            }
        }
        
    }
    return error;
}

/******************************************************************************/

INT32 dos_MatchFirst(pDOSBase DOSBase, const STRPTR pat, struct AnchorPath *AP)
{
	struct AChain       *ac;
	LONG                error;

	AP->ap_Flags   = 0;
	AP->ap_Base    = 0;
	AP->ap_Current = 0;

	error = Match_BuildAChainList(pat, AP, &ac, DOSBase);
	if (error == 0)
	{
		AP->ap_Base = AP->ap_Current = ac;
		error = MatchNext(AP);
	} /* if (error == 0) */

	SetIoErr(error); // this should be wrong...
	return error;
}

void dos_MatchEnd(pDOSBase DOSBase, struct AnchorPath *AP)
{
	struct AChain *ac = AP->ap_Base, *acnext;

	if (ac)
	{
#if MATCHFUNCS_NO_DUPLOCK
        /*
        ** CurrentDir to a valid lock, ie. one that will not be
        ** killed further below
        */
        
        CurrentDir(ac->an_Lock);
#endif
        
        while(ac)
        {
            acnext = ac->an_Child;

            /*
            ** Dont unlock lock in first AChain because it is the same
            ** as the current directory when MatchFirst was called. And
            ** this lock was not DupLock()ed (except MATCHFUNCS_NO_DUPLOCK == 0)!!!
            */
            
            if (ac->an_Lock
#if MATCHFUNCS_NO_DUPLOCK
                && (ac != AP->ap_Base)
#endif
               )
            {
                UnLock(ac->an_Lock);
            }
            Match_FreeAChain(ac, DOSBase);
            ac = acnext;
        }
    }
    AP->ap_Current = NULL;
    AP->ap_Base = NULL;
}

INT32 dos_MatchNext(pDOSBase DOSBase, struct AnchorPath *AP)
{
    struct AChain       *ac = AP->ap_Current;
    pFileLock           origdir;
    LONG                error = 0;
    BOOL                dir_changed = FALSE;
    
    origdir = CurrentDir(0);
    CurrentDir(origdir);

    AP->ap_Flags &= ~APF_DIDDIR;
    
    /*
    ** Check if we are asked to enter a directory, but only do this
    ** if it is really possible
    */
       
    if ((AP->ap_Flags & APF_DODIR) &&
        (ac->an_Flags & DDF_ExaminedBit) &&
        (ac->an_Info.fib_DirEntryType > 0) &&
        (ac->an_Child == NULL))
    {
        /*
        ** Alloc a new AChain. Make it the active one. Set its string to
        ** "#?" and mark it with DDF_AllBit Flag to indicate that this is a
        ** "APF_DODIR-AChain".  This is important for "back steppings",
        ** because "APF_DODIR-AChains" must be removed and freed then and
        ** the user must be notified about the leaving of a APF_DODIR-AChain
        ** with APF_DIDDIR.
        */
           
        if ((ac->an_Child = Match_AllocAChain(1, DOSBase)))
        {
            ac->an_Child->an_Parent = ac;
            ac = ac->an_Child;
            AP->ap_Current = ac;
            
            ac->an_String[0] = P_ANY;
            ac->an_String[1] = 0;
            ac->an_Flags = DDF_PatternBit | DDF_AllBit;
            
            dir_changed = TRUE;     
        }
        
        /*
        ** If the allocation did not work, we simple ignore APF_DODIR. Just
        ** like if the user did not set this flag. Good idea or bad idea?
        */
    }

    
    /* Main loop for AChain traversing */
    
    for(;;)
    {
        BOOL must_go_back = FALSE;
        
        /* Check for user breaks (CTRL_C, ...) */
         
        if (AP->ap_BreakBits)
        {
            AP->ap_FoundBreak = CheckSignal(AP->ap_BreakBits);
            if (AP->ap_FoundBreak)
            {
                error = ERROR_BREAK;
                goto done;
            }
        }
        
        /* Check if AChain must be "setup" */
        
        if (!(ac->an_Flags & DDF_ExaminedBit))
        {
            /*
            ** This AChain must be "setup". First AChain->an_Lock must point
            ** to the parent directory, that is the directory where this
            ** AChain is "in". !
            */
           
            dir_changed = TRUE;
            
            if (ac->an_Parent)
            {
                CurrentDir(ac->an_Parent->an_Lock);
                if (ac->an_Parent->an_Flags & DDF_PatternBit)
                {
                    ac->an_Lock = Lock(ac->an_Parent->an_Info.fib_FileName, SHARED_LOCK);
                }
                else
                {
                    ac->an_Lock = Lock(ac->an_Parent->an_String, SHARED_LOCK);
                }

                if (!ac->an_Lock)
                {
                    error = IoErr();
                    goto done;
                }
            }
#if !MATCHFUNCS_NO_DUPLOCK
            else
            {
                ac->an_Lock = DupLock(origdir);

                if (!ac->an_Lock)
                {
                    error = IoErr();
                    goto done;
                }               

            }
#else
            /*
            ** If there was no ac->an_Parent then we are dealing with the
            ** first AChain whose lock was already setup in
            ** Match_BuildAChainList
            */
#endif
            
            CurrentDir(ac->an_Lock);
            
            if (ac->an_Flags & DDF_PatternBit)
            {
                /*
                ** If this is a pattern AChain we first Examine here our
                ** parent directory, so that it then can be traversed with
                ** ExNext
                */
                if (!Examine(ac->an_Lock, &ac->an_Info))
                {
                    error = IoErr();
                    goto done;
                }
                ac->an_Flags |= DDF_ExaminedBit;

            } /* if (ac->an_Flags & DDF_PatternBit) */
            else
            {
                pFileLock lock;
                LONG success;
                
                /*
                ** This is a normal AChain (no pattern). Try to lock it
                ** to see if it exists.
                */
                   
                if (!(lock = Lock(ac->an_String, SHARED_LOCK)))
                {
                    /* It does not exist, so if possible go back one step */
                    
                    if ((AP->ap_Flags & APF_ITSWILD) && (ac->an_Parent))
                    {
                        /* [2] */
                        
                        must_go_back = TRUE;
                    }
                    else
                    {
                        /* if going back is not possible get error code and exit */
                        error = IoErr();
                        goto done;
                    }
                    
                } /* if (!(lock = Lock(ac->an_String, SHARED_LOCK))) */
                else
                {
                    /* The File/Direcory ac->an_String exists */
                    
                    success = Examine(lock, &ac->an_Info);
                    UnLock(lock);

                    if (!success)
                    {
                        /*
                        ** Examine()ing the file/directory did not
                        ** work, although the lock was successful!?.
                        ** Get error code and exit
                        */
                           
                        error = IoErr();
                        goto done;
                    }

                    /*
                    ** This strcpy is necessary, because in case
                    ** of empty ac->an_String("") fib_FileName would
                    ** get parent directory name which it must not!
                    */

                    if (*ac->an_String == '\0')
                    {
                        Strcpy(ac->an_Info.fib_FileName, ac->an_String);
                    }
                    
                    ac->an_Flags |= DDF_ExaminedBit;

                    /*
                    ** If this is a file, but there are still more path
                    ** components to follow then we have to go back one step
                    ** (AChain)
                    */
                    
                    if (ac->an_Child && (ac->an_Info.fib_DirEntryType < 0))
                    {
                        /* [1] */
                        
                        must_go_back = TRUE;
                    }
                
                    /*
                    ** Here we either have found a matching file/directory
                    ** (result) or, if ac->an_Child != NULL we have still to
                    ** continue walking through the AChains until we are in
                    ** the last one. This all happens further below 
                    */
                    
                } /* if (!(lock = Lock(ac->an_String, SHARED_LOCK))) else ... */
                
            } /* if (ac->an_Flags & DDF_PatternBit) else ... */ 

        } /* if (!(ac->an_Flags & DDF_ExaminedBit)) */
        else
        {
            /*
            ** This AChain was already setup.
            **
            ** When an AChain which is *not* a pattern already had
            ** DDF_PatternBit set, then this means ERROR_NO_MORE_ENTRIES, so
            ** we try to go back one step
            */
               
            if (!(ac->an_Flags & DDF_PatternBit))
            {
                /* [4] */
                
                must_go_back = TRUE;
            }
        }
        
        /*
        ** Here we can be sure that the actual AChain is set up; i.e. it will
        ** have ac->an_Lock set correctly and to indicate this
        ** DDF_ExaminedBit was set
        */
           
        CurrentDir(ac->an_Lock);
        
        if (ac->an_Flags & DDF_PatternBit)
        {
            if(ExNext(ac->an_Lock, &ac->an_Info))
            {
                if (MatchPatternNoCase(ac->an_String, ac->an_Info.fib_FileName))
                {
                    /*
                    ** This file matches the pattern in ac->an_String. If
                    ** there are no more AChains to follow then we have
                    ** found a matching file/directory (a result)  -->
                    ** break.
                    */  
                               
                    if (!ac->an_Child)
                    {                   
                        break;
                    }
                    else
                    {
                        if (ac->an_Info.fib_DirEntryType < 0)
                        {
                            /* This is a file, no chance to follow child
                               AChain. Go to top of "for(;;)" loop */
                            continue;                       
                        }
                    }
                    
                } else {
                    /* Did not match. Go to top of "for(;;)" loop */
                    continue;
                }
            }
            else
            {
                error = IoErr();
                if (error != ERROR_NO_MORE_ENTRIES) goto done;
                
                /* [3] */
                
                must_go_back = TRUE;
            }
            
        } /* if (ac->an_Flags & DDF_PatternBit) */
        
        /*
        ** Handle the cases where we must (try to) go back to the previous
        ** AChain.  This can happen if the actual AChain turned out to be a
        ** file although there are still more AChains to follow [1]. Or if
        ** the actual AChain did not exist at all [2]. Or if in a pattern
        ** AChain ExNext() told us that there are no more entries [3]. Or if
        ** we were getting to a normal (no pattern) AChain which was already
        ** set up (DDF_ExaminedBit) [4].
        */
        
        if (must_go_back)
        {
            /* Check if going back is possible at all */
            
            if (!ac->an_Parent)
            {
                error = ERROR_NO_MORE_ENTRIES;
                goto done;
            }
            
            dir_changed = TRUE;
            
            /* Yep. It is possible. So let's cleanup the AChain. */
            
            CurrentDir(ac->an_Parent->an_Lock);
            
            UnLock(ac->an_Lock);
            
            ac->an_Lock = NULL;
            ac->an_Flags &= ~DDF_ExaminedBit;
            
            /* Make ac and AP->ap_Current point to the previous AChain */
            
            AP->ap_Current = ac->an_Parent;
            
            /*
            ** If this was an APF_DODIR Achain (indicated by DDF_AllBit)
            ** then the AChain must be unlinked and freed. And the user
            ** must be informed about the leaving with APF_DIDDIR and
            ** a "result" in AnchorPath which points to the directory which
            ** was leaved.
            */
            
            if (ac->an_Flags & DDF_AllBit)
            {
                AP->ap_Current->an_Child = NULL;
                Match_FreeAChain(ac, DOSBase);
                AP->ap_Flags |= APF_DIDDIR;
                
                /* go out of for(;;) loop --> MakeResult */
                
                break;
            }
            
            ac = AP->ap_Current;
            
        } /* if (must_go_back) */
        else
        {
            if (!ac->an_Child)
            {
                /*
                ** We have reached the last AChain. And this means that
                ** we have found a matching file/directory :-)). Go out of
                ** for(;;) loop --> MakeResult 
                */
                
                break;
            }
            
            ac = ac->an_Child;
            AP->ap_Current = ac;
            
            dir_changed = TRUE; /* CHECKME!!! Really? */
        }

    } /* for(;;) */

    error = Match_MakeResult(AP, DOSBase);
    
done:
    CurrentDir(origdir);
    
    AP->ap_Flags &= ~APF_DODIR;
    
    if (dir_changed)
    {
        AP->ap_Flags |= APF_DirChanged;
    }
    else
    {
        AP->ap_Flags &= ~APF_DirChanged;
    }

    SetIoErr(error); //che: WRONG?!
    return error;
}


