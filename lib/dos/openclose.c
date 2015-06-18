/**
 * @file openclose.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

// the whole file is now not used anymore.
#if 0

static void _InitFH(struct FileHandle *fh, UINT32 type)
{
	fh->fh_Flags = 0; //FHF_EXTEND;
	fh->fh_Interactive = FALSE;
	fh->fh_Type	= NULL;
	fh->fh_Pos		= -1;
	fh->fh_End		= -1;
	fh->fh_Buf 		= NULL;
	fh->fh_Func1	= NULL;
	fh->fh_Func2	= NULL;
	fh->fh_Func3	= NULL;
	fh->fh_Arg1		= NULL;
	fh->fh_Arg2		= NULL;
	fh->fh_BufSize	= 0;
}

//unbuffered Open

pFileHandle dos_Open(pDOSBase DOSBase, STRPTR string, INT32 mode)
{
	pProcess	this= FindProcess(NULL);
	if (this == NULL) return NULL;
	
	pFileHandle	fh = AllocVec(sizeof(FileHandle), MEMF_FAST);//AllocDosObject(DOS_FILEHANDLE, NULL);
	if (fh)
	{
		char		name[256];
		BOOL 		again;
		UINT32		len = Strlen(string);
		INT32		ok = 0;
		pHandlerProc dvp = NULL;

		if (len<256)
		{
			Strcpy(name, string);
			do 
			{
				again = FALSE;
				_InitFH(fh, mode);
				if (!Stricmp("CONSOLE:",name))
				{
					fh->fh_Type = this->pr_ConsoleTask;
					if (fh->fh_Type)
						DoPkt(fh->fh_Type, mode, (INT32)fh, 0, (INT32)name, 0, 0);
					else
					{
						return fh;
					}
				} else if (!Stricmp("NIL:",name))
				{
					return fh;
				} else
				{				
					do
					{
						dvp = ObtainHandler(name);
						if (!dvp) break;
						
						fh->fh_Type = dvp->hp_Port;
						ok = DoPkt(fh->fh_Type, mode, (INT32)fh, (INT32)dvp->hp_Lock, (INT32)name, 0, 0);
					} while(!ok && IoErr() == ERROR_OBJECT_NOT_FOUND && mode != MODE_NEWFILE);

					if (dvp)
					{
						if (!ok && IoErr() == ERROR_IS_SOFT_LINK)
						{
							char buf[256];
							if (ReadLink(dvp->hp_Port, dvp->hp_Lock, (UINT8*) name, (UINT8*)buf, 256) >= 0)
							{
								Strcpy(name, buf);
								again = TRUE;
							}
						}
						ReleaseHandler(dvp);
					}
				}
			} while(again);
			
			if (ok)	return fh;
			ReleaseHandler(dvp);			
		} else
			SetIoErr(ERROR_LINE_TOO_LONG);		
	} else
		SetIoErr(ERROR_NO_FREE_STORE);	
	return NULL;
}
#endif
#if 0
// Unbuffered Close (not used anymore
DOSIO dos_Close(pDOSBase DOSBase, pFileHandle fh)
{
	DOSIO	ret = DOSIO_FALSE;
	if (!fh)
	{
		SetIoErr(ERROR_INVALID_LOCK);
		return ret;
	}
#if 0
// SANITY CHECK
	if (fh_Flags & FHF_CLOSED)
	{
		SetIoErr(ERROR_FILE_NOT_OBJECT);
		return ret;	
	}
#endif
	INT32 	res2 = IoErr();
	while(1)
	{
		ret = DoPkt(fh->fh_Type, ACTION_END,(INT32)fh->fh_Arg1, 0, 0, 0, 0);
		if (ret || ErrorReport(IoErr(), REPORT_STREAM, (INT32) fh, NULL))
		{
			//fh->fh_Flags |= FHF_CLOSED;
			FreeVec(fh);
			if (ret) SetIoErr(res2);
			return ret;
		}
	}
}

#endif

