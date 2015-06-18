/**
* File: list.c
* User: Srinivas Nayak
* Date: 2014-12-04
* Time: 06:57 PM
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

#define	VSTRING	"resident 0.1 (04.12.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "DIR/M,PAT/K,KEYS/S,DATES/S,NODATES/S,TO/K,SUB/K,QUICK/S,BLOCK/S,NOHEAD/S,FILES/S,DIRS/S,LFORMAT/K,ALL/S" CMDREV

typedef struct GlobalData
{
	pSysBase	sysBase;
	pDOSBase	dOSBase;
	pUtilBase	utilBase;
} GlobalData_t, *pGlobalData;

struct DirNode
{
    struct MinNode  node;
    STRPTR dirname;
};

typedef struct _Statistics
{
    UINT32 nFiles;
    UINT32 nDirs;
    UINT32 nBlocks;
} Statistics;

enum
{
    OPT_DIR = 0,
    OPT_PAT,
    OPT_KEYS,
    OPT_DATES,
    OPT_NODATES,
    OPT_TO,
    OPT_SUB,
    OPT_QUICK,
    OPT_BLOCK,
    OPT_NOHEAD,
    OPT_FILES,
    OPT_DIRS,
    OPT_LFORMAT,
    OPT_ALL,
    NOOFOPTS
};

#define  MAX_PATH_LEN  1024

#define  BLOCKSIZE  512

DOSCALL main(APTR SysBase)
{
	pGlobalData gd = AllocVec(sizeof (GlobalData_t), MEMF_FAST|MEMF_CLEAR);
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[NOOFOPTS];

    static const STRPTR default_directories[] = {(const STRPTR)"", 0};
    STRPTR   parsedPattern = NULL;
    STRPTR   subpatternStr = NULL;
    APTR     oldOutput = NULL;
    Statistics stats = { 0, 0, 0 };

	gd->sysBase = SysBase;
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		gd->dOSBase = DOSBase;
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			gd->utilBase = UtilBase;
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else
			{
				rc = RETURN_OK;
/*
 * Your console code goes here!
 */

				const STRPTR *directories = (const STRPTR *)opts[OPT_DIR];
				STRPTR  lFormat = (STRPTR)opts[OPT_LFORMAT];
				STRPTR  pattern = (STRPTR)opts[OPT_PAT];
				STRPTR  toFile = (STRPTR)opts[OPT_TO];
				STRPTR  subStr = (STRPTR)opts[OPT_SUB];
				BOOL    files = (BOOL)opts[OPT_FILES];
				BOOL    dirs  = (BOOL)opts[OPT_DIRS];
				BOOL    noDates = (BOOL)opts[OPT_NODATES];
				BOOL    dates = (BOOL)opts[OPT_DATES];
				BOOL    quick = (BOOL)opts[OPT_QUICK];
				BOOL    noHead = (BOOL)opts[OPT_NOHEAD];
				BOOL    block = (BOOL)opts[OPT_BLOCK];
				BOOL    all = (BOOL)opts[OPT_ALL];
				BOOL    keys = (BOOL)opts[OPT_KEYS];

				UINT32   i;		/* Loop variable */

				if (subStr != NULL)
				{
					STRPTR  subStrWithPat;
					UINT32   length = (Strlen(subStr) + sizeof("#?#?"))*2 + 2;

					subStrWithPat = AllocVec(length, MEMF_FAST);

					if (subStrWithPat == NULL)
					{
						rc = IoErr();
						FreeArgs(rdargs);
						PrintFault(rc, "List");

						return RETURN_FAIL;
					}

					Strcpy(subStrWithPat, "#?");
					Strcat(subStrWithPat, subStr);
					Strcat(subStrWithPat, "#?");

					subpatternStr = AllocVec(length, MEMF_FAST);

					if (subpatternStr == NULL ||
						ParsePatternNoCase(subStrWithPat, subpatternStr, length) == -1)
					{
						rc = IoErr();
						FreeVec(subStrWithPat);
						FreeArgs(rdargs);
						PrintFault(rc, "List");

						return RETURN_FAIL;
					}

					FreeVec(subStrWithPat);
				}


				if (pattern != NULL)
				{
					UINT32 length = Strlen(pattern)*2 + 2;

					parsedPattern = AllocVec(length, MEMF_FAST);

					if (parsedPattern == NULL ||
						ParsePatternNoCase(pattern, parsedPattern, length) == -1)
					{
						FreeVec(subpatternStr);
						FreeArgs(rdargs);

						return RETURN_FAIL;
					}
				}

				if (toFile != NULL)
				{
					APTR file = Open(toFile, MODE_NEWFILE);

					if (file == NULL)
					{
						rc = IoErr();
						FreeVec(subpatternStr);
						FreeVec(parsedPattern);
						FreeArgs(rdargs);
						PrintFault(rc, "List");

						return RETURN_FAIL;
					}
					oldOutput = SelectOutput(file);
				}

				if (!files && !dirs)
				{
					files = TRUE;
					dirs = TRUE;
				}

				if (lFormat)
				{
					noHead = TRUE;
				}

				if ((directories == NULL) || (*directories == NULL))
				{
					directories = default_directories;
				}

				for (i = 0; directories[i] != NULL; i++)
				{
					rc = listFile(gd, directories[i], files, dirs, parsedPattern,
									 noHead, lFormat, quick, dates, noDates,
									 block, subpatternStr, all, keys,
									 &stats);
					if (rc != 0)
					{
						break;
					}

				}

				if ((BOOL)opts[OPT_NOHEAD] == FALSE &&
					(BOOL)opts[OPT_LFORMAT] == FALSE &&
					(BOOL)opts[OPT_ALL] &&
					(stats.nFiles || stats.nDirs))
				{
					Printf("\nTOTAL: %ld files - %ld directories - %ld blocks used\n",
						   stats.nFiles, stats.nDirs, stats.nBlocks);
				}


				if (parsedPattern != NULL)
				{
					FreeVec(parsedPattern);
				}

				if (subpatternStr != NULL)
				{
					FreeVec(subpatternStr);
				}

				if (oldOutput != NULL)
				{
					Close(SelectOutput(oldOutput));
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

#define DOSBase gd->dOSBase
#define SysBase gd->sysBase
#define UtilBase gd->utilBase

int listFile(pGlobalData gd, STRPTR filename, BOOL showFiles, BOOL showDirs,
             STRPTR parsedPattern, BOOL noHead, STRPTR lFormat, BOOL quick,
             BOOL dates, BOOL noDates, BOOL block, STRPTR subpatternStr, BOOL all, BOOL keys, Statistics *stats)
{
    struct AnchorPath *ap;
    struct List DirList, FreeDirNodeList;
    struct DirNode *dirnode, *prev_dirnode = NULL;

    UINT32  files = 0;
    UINT32  dirs = 0;
    UINT32  nBlocks = 0;
    INT32  error;

    NewList(&DirList);
    NewList(&FreeDirNodeList);

    do
    {
        ap = AllocVec(sizeof(struct AnchorPath) + MAX_PATH_LEN, MEMF_CLEAR);

        if (ap == NULL)
        {
            return 0;
        }

        ap->ap_Strlen = MAX_PATH_LEN;

        error = MatchFirst(filename, ap);

        /* Explicitly named directory and not a pattern? --> enter dir */

        if (0 == error)
        {
            if (!(ap->ap_Flags & APF_ITSWILD))
            {
                if (ap->ap_Info.fib_DirEntryType >= 0)
                {
                    ap->ap_Flags |= APF_DODIR;

                    if (0 == error)
                    {
                        error = MatchNext(ap);
                    }
                }
            }
        }

        if (0 == error)
        {
            BOOL first = TRUE;

            ap->ap_BreakBits = SIGBREAKF_CTRL_C;
            if (FilePart(ap->ap_Buf) == ap->ap_Buf)
            {
                ap->ap_Flags &= ~APF_DirChanged;
            }

            do
            {
                /*
                ** There's something to show.
                */
                if (!(ap->ap_Flags & APF_DIDDIR))
                {
                    if (ap->ap_Flags & APF_DirChanged)
                    {
                        STRPTR p;
                        UINT8 c;

                        if (!first) printSummary(gd, filename, files, dirs, nBlocks, noHead, TRUE);

                        /* Update global statistics for (possible) ALL option */
                        stats->nFiles += files;
                        stats->nDirs += dirs;
                        stats->nBlocks += nBlocks;

                        files = 0;
                        dirs = 0;
                        nBlocks = 0;

                        p = PathPart(ap->ap_Buf);
                        c = *p;
                        *p = 0;

                        error = printDirHeader(ap->ap_Buf, noHead);

                        *p = c;

                    }

                    error = printFileData(gd,
										  ap,
                                          showFiles,
                                          showDirs,
                                          parsedPattern,
                                          &files,
                                          &dirs,
                                          &nBlocks,
                                          lFormat,
                                          quick,
                                          dates,
                                          noDates,
                                          block,
                                          subpatternStr,
                                          keys);

                    if (all && (ap->ap_Info.fib_DirEntryType >= 0))
                    {
                        if ((dirnode = AllocVec(sizeof(struct DirNode), MEMF_FAST)))
                        {

                            if ((dirnode->dirname = StrDup(ap->ap_Buf)))
                            {
                                Insert(&DirList, (struct Node *)dirnode, (struct Node *)prev_dirnode);

                                prev_dirnode = dirnode;
                            }
                            else
                            {
                                FreeVec(dirnode);
                            }
                        }
                    }
                }

                error = MatchNext(ap);

                first = FALSE;

            } while (0 == error);
        }

        MatchEnd(ap);

        FreeVec(ap);

        if (error == ERROR_BREAK)
        {
            PrintFault(error, NULL);
        }

        if (error == ERROR_NO_MORE_ENTRIES)
        {
            error = 0;
        }

        if ((error == 0) || (error == ERROR_BREAK))
        {
            BOOL printEmpty = !(ap->ap_Flags & APF_ITSWILD);
            printSummary(filename, files, dirs, nBlocks, noHead, printEmpty);
        }

        /* Update global statistics for (possible) ALL option */
        stats->nFiles += files;
        stats->nDirs += dirs;
        stats->nBlocks += nBlocks;

        files = 0;
        dirs = 0;
        nBlocks = 0;

        if (error) break;

        dirnode = (struct DirNode *)RemHead(&DirList);

        if (dirnode != NULL)
        {
            filename = dirnode->dirname;

            prev_dirnode = NULL;

            /* do not free() dirnode, as we reference dirnode->dirname! */

            AddTail(&FreeDirNodeList, (struct Node *)dirnode);
        }
    } while (dirnode);

    while ((dirnode = (struct DirNode *)RemHead(&FreeDirNodeList)))
    {
        FreeVec(dirnode->dirname);
        FreeVec(dirnode);
    }

    return error;
}


void printSummary(pGlobalData gd, CONST_STRPTR dirname, int files, int dirs, int nBlocks,
                    BOOL noHead, BOOL PrintEmpty)
{
    if (noHead) return;

    if (files || dirs)
    {

        if (files > 1)
            Printf("%ld files", files);
        else if (files > 0)
            PutStr("1 file");

        if( files && (dirs || nBlocks) ) PutStr(" - ");

        if (dirs > 1)
            Printf("%ld directories", dirs);
        else if (dirs > 0)
            PutStr("1 directory");

        if( dirs && nBlocks ) PutStr(" - ");

        if (nBlocks > 1)
            Printf("%ld blocks used\n", nBlocks);
        else if (nBlocks > 0)
            PutStr("1 block used\n");
        else PutStr("\n");

    }
    else if (PrintEmpty)
        Printf("Directory \"%s\" is empty\n", dirname);
}

struct lfstruct
{
    struct AnchorPath *ap;
    BOOL    	       isdir;
    STRPTR  	       date;
    STRPTR  	       time;
    STRPTR  	       flags;
    STRPTR  	       filename;
    STRPTR  	       comment;
    UINT32   	       size;
    UINT32   	       key;
};

static void _prbuf(INT32 ch, INT8 **str)
{
	*(*str)++ = (char)ch;
}

static int sprintf(pGlobalData gd, char* str, char *fmt, ...)
{
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) str);
	va_end(pvar);
	return Strlen((STRPTR)str);
}

#define  roundUp(x, bSize) ((x + bSize - 1)/bSize)

int printFileData(pGlobalData gd, struct AnchorPath *ap,
                  BOOL showFiles, BOOL showDirs, STRPTR parsedPattern,
                  UINT32 *files, UINT32 *dirs, UINT32 *nBlocks, STRPTR lFormat,
                  BOOL quick, BOOL dates, BOOL noDates, BOOL block,
                  STRPTR subpatternStr,
                  BOOL keys)
{
    STRPTR  	       filename = ap->ap_Buf;
    BOOL    	       isDir = (ap->ap_Info.fib_DirEntryType >= 0);
    struct DateStamp  *ds = &ap->ap_Info.fib_Date;
    UINT32   	       protection = ap->ap_Info.fib_Protection;
    UINT32   	       size = ap->ap_Info.fib_Size;
    STRPTR  	       filenote = ap->ap_Info.fib_Comment;
    UINT32    	       diskKey = ap->ap_Info.fib_DiskKey;

    int error = 0;

    UINT8 flags[8];

    /* Does the filename match a certain pattern? (ARG_PAT) */
    if (parsedPattern != NULL &&
        !MatchPatternNoCase(parsedPattern, FilePart(filename)))
    {
        return 0;
    }

    /* Does a substring of the filename match a certain pattern? (ARG_SUB) */
    if (subpatternStr != NULL &&
        !MatchPatternNoCase(subpatternStr, FilePart(filename)))
    {
        return 0;
    }

    /* Convert the protection bits to a string */
    flags[0] = protection & FIBF_SCRIPT  ? 's' : '-';
    flags[1] = protection & FIBF_PURE    ? 'p' : '-';
    flags[2] = protection & FIBF_ARCHIVE ? 'a' : '-';

    /* The following flags are high-active! */
    flags[3] = protection & FIBF_READ    ? '-' : 'r';
    flags[4] = protection & FIBF_WRITE   ? '-' : 'w';
    flags[5] = protection & FIBF_EXECUTE ? '-' : 'e';
    flags[6] = protection & FIBF_DELETE  ? '-' : 'd';
    flags[7] = 0x00;

    if (isDir)
    {
        if (showDirs)
        {
            ++*dirs;
            ++*nBlocks; /* dir entry uses 1 block on AROS, 2 on OS31) */

            if (lFormat != NULL)
            {
                struct lfstruct lf = { ap, isDir, "", "", flags, filename,
                                       filenote, size, diskKey};

                printLformat(lFormat, &lf);
                Printf("\n");
            }
            else
            {
                Printf("%-25s ", FilePart(filename));

                if (!quick)
                {
                    Printf("        <Dir> %7s ", flags);
                }

                Printf("\n");
            }
        }
    }
    else if (showFiles)
    {
        ++*files;
        *nBlocks += roundUp(size, BLOCKSIZE);

        if (lFormat != NULL)
        {
            struct lfstruct lf = { ap, isDir, "", "", flags, filename,
                                   filenote, size, diskKey };

            printLformat(lFormat, &lf);
            Printf("\n");
        }
        else
        {
            Printf("%-25s ", FilePart(filename));

            if (!quick)
            {
                if(keys)
                {
                    char key[16];
                    int  fill;
                    int  i;	/* Loop variable */

                    sprintf(gd, key, "%lu", (unsigned long)diskKey);
                    fill = 7 - Strlen(key) - 2;

                    for (i = 0; i < fill; i++)
                    {
                        Printf(" ");
                    }

                    Printf("[%lu] ", diskKey);
                }
                else
                {
                    if (0 != size)
                    {
                        UINT32 filesize = block ? roundUp(size, BLOCKSIZE) : size;

                        char buf[256];
                        buf[255] = '\0';
                        int fill, i;

                        char *bufpos = &buf[254];

                        do
                        {
                            bufpos[0] = '0' + (filesize % 10);
                            filesize /= 10;
                            bufpos--;
                        } while (filesize);

                        fill = 13 - (&buf[254] - bufpos);
                        for (i = 0; i < fill; i++)
                        {
                            Printf(" ");
                        }
                        Printf("%s ", &bufpos[1]);
                    }
                    else
                    {
                        Printf("  empty ");
                    }
                }

                Printf("%7s ", flags);
            }


            if (!quick && (*filenote != 0))
            {
                Printf("\n: %s", filenote);
            }

            Printf("\n");
        }
    }

    return error;
}


int printLformat(pGlobalData gd, STRPTR format, struct lfstruct *lf)
{
    STRPTR filename       = FilePart(lf->filename);
    STRPTR temp           = format;
    INT32   substitutePath = 0;
    char c;

    /*
        Whether the path or the filename is substituted for an occurrence
        of %S depends on how many occurrences are in the LFORMAT line, and
        their order, as follows:

        Occurrences of %S   1st         2nd         3rd         4th
        1                   filename
        2                   path        filename
        3                   path        filename    filename
        4                   path        filename    path        filename
    */

    while ( ( substitutePath < 4 ) && ( '\0' != (c = *temp++) ) )
    {
        if ( '%' == c )
            if ( 'S' == ToUpper(*temp++) )
                substitutePath++;
    }
    if ( substitutePath == 3 )
        substitutePath = 2;

    while ('\0' != (c = *format++))
    {
        if ('%' == c)
        {
            switch (ToUpper(*format++))
            {
                /* File comment */
            case 'C':
                Printf(lf->comment);
                break;

                /* Modification date */
            case 'D':
                Printf(lf->date);
                break;

                /* Modification time */
            case 'T':
                Printf(lf->time);
                break;

                /* File size in blocks of BLOCKSIZE bytes */
            case 'B':
                if (lf->isdir)
                {
                    Printf("Dir");
                }
                else
                {
                    UINT32 tmp = roundUp(lf->size, BLOCKSIZE);

                    /* File is 0 bytes? */
                    if (tmp == 0)
                    {
                        Printf("empty");
                    }
                    else
                    {
                        Printf("%lu", tmp);
                    }
                }

                break;

                /* Path incl. volume name*/
            case 'F':
                {
                    UINT8 buf[256];

                    if (NameFromLock(lf->ap->ap_Current->an_Lock, buf, 256))
                    {
                        int len;

                        Printf(buf);

                        len = Strlen(buf);
                        if ((len > 0) && (buf[len - 1] != ':') && (buf[len - 1] != '/'))
                        {
                            Printf("/");
                        }
                    }
                }

                break;

                /* File attributes (flags) */
            case 'A':
                Printf(lf->flags);
                break;

                /* Disk block key */
            case 'K':
                Printf("[%lu]", lf->key);
                break;

                /* File size */
            case 'L':
                if (lf->isdir)
                {
                    Printf("Dir");
                }
                else
                {
                    if (lf->size == 0)
                    {
                        Printf("empty");
                    }
                    else
                    {
                        UINT32 filesize = lf->size;

                        char buf[256];
                        buf[255] = '\0';
                        int fill, i;

                        char *bufpos = &buf[254];

                        do
                        {
                            bufpos[0] = '0' + (filesize % 10);
                            filesize /= 10;
                            bufpos--;
                        } while (filesize);

                        fill = 13 - (&buf[254] - bufpos);
                        for (i = 0; i < fill; i++)
                        {
                            Printf(" ");
                        }
                        Printf("%s ", &bufpos[1]);
                    }
                }

                break;

                /* File name without extension */
            case 'M':
                {
                    STRPTR lastPoint = Strrchr(filename, '.');

                    if (lastPoint != NULL)
                    {
                        *lastPoint = 0;
                    }

                    Printf(filename);

                    /* Resurrect filename in case we should print it once
                       more */
                    if (lastPoint != NULL)
                    {
                        *lastPoint = '.';
                    }
                }

                break;

                /* Filename or Path name */
            case 'S':
                if ( (--substitutePath == 3) || (substitutePath == 1) )
                {
                    STRPTR end = FilePart(lf->filename);
                    UINT8  token = *end;

                    *end = '\0';

                    Printf(lf->filename);

                    /* Restore pathname */
                    *end = token;

                    break;
                }
                /* Fall through */
            case 'N':
                Printf(filename);
                break;

                /* File extension */
            case 'E':
                {
                    STRPTR extension = Strrchr(filename, '.');

                    if (extension != NULL)
                    {
                        Printf(extension);
                    }
                }

                break;

                /* Path name, but without volume */
            case 'P':
                {
                    STRPTR end = FilePart(lf->filename);
                    UINT8  token = *end;

                    *end = 0;

                    Printf(lf->filename);

                    /* Restore pathname */
                    *end = token;
                }

                break;

            case 0:
                return 0;
                break;

            default:
                Printf("%%%lc", *format);
                break;
            }
        }
        else
        {
            Printf("%lc", c);
        }
    }

    return 0;
}

int printDirHeader(pGlobalData gd, STRPTR dirname, BOOL noHead)
{
    if (!noHead)
    {
        Printf("Directory \"%s\":\n", dirname);
    }

    return RETURN_OK;
}
