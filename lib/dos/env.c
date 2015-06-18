/**
 * @file env.c
 *
 * Variable Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

static void _AddVar(pDOSBase DOSBase, struct List *list, struct LocalVar *nlv)
{
	struct LocalVar *lv;
	char *name = nlv->lv_Node.ln_Name;

	ForeachNode(list, lv)
	{
		if (Stricmp(name,lv->lv_Node.ln_Name) < 0)
		{
			Insert(list,&(nlv->lv_Node),lv->lv_Node.ln_Pred);
			return;
		}
	}
	AddTail(list,&(nlv->lv_Node));
}

static INT32 dos_int_MakeDir(pDOSBase DOSBase, char *path)
{
	char *newname = NULL, *end;
	pFileLock lock;
	INT32 i, rc = FALSE;;

	end = PathPart(path);
	if (end == path) return rc;

	newname = AllocVec((end+1) - path, MEMF_FAST);
	if (!newname) return rc;

	for (i = 0; i < end-path; i++) newname[i] = path[i];
	newname[i] = '\0';

	if ((lock = CreateDir(newname)))
	{
		UnLock(lock);
		rc = DOSIO_TRUE;
	} else {
		if (IoErr() == ERROR_OBJECT_NOT_FOUND) rc = dos_int_MakeDir(DOSBase, newname);
	}
	if (newname) FreeVec(newname);
	return rc;
}

INT32 dos_DeleteVar(pDOSBase DOSBase, const STRPTR name, UINT32 flags)
{
	return SetVar(name, NULL, 0, flags);
}

struct LocalVar * dos_FindVar(pDOSBase DOSBase, const STRPTR name, UINT32 flags)
{
	struct LocalVar *lv;
	struct Process	*pr;

    if (name != NULL)
    {
		pr = FindProcess(NULL);
		
		ForeachNode(&pr->pr_LocalVars,  lv)
		{
			if ((lv->lv_Node.ln_Type == (flags & 0xff)) &&
				(Stricmp(name,lv->lv_Node.ln_Name) == SAME))
			{
				return lv;
			}
		}
	}
	SetIoErr(ERROR_OBJECT_NOT_FOUND);
	return NULL;
}

INT32 dos_GetVar(pDOSBase DOSBase, const STRPTR name, UINT8 *buffer, INT32 size, UINT32 flags)
{
	struct LocalVar *lv;
	struct FileHandle *fh;
	struct FileLock *dirlock;
	struct FileLock *olddir;
	INT32 len,  fsize;
	INT32 err;
	
	if (size == 0)
	{
		SetIoErr(ERROR_BAD_NUMBER);
		return -1;
	}
	
	if (name && buffer)
	{
		if(!(flags & GVF_GLOBAL_ONLY))
		{
			lv = FindVar(name, flags);

			if (lv)
			{
				len = ((UINT32)size < lv->lv_Len) ? (UINT32)size : lv->lv_Len;
				CopyMem(lv->lv_Value, buffer, len);

				if (!(flags & GVF_BINARY_VAR))
				{
					INT32 j = 0;
					while ((buffer[j] != '\n') && (j < len)) j++;
					if (j == size) j = size - 1;

					buffer[j]= '\0';
					size = j;
				} else if (!(flags & GVF_DONT_NULL_TERM))
				{
					if (len == size) len = size - 1;
					buffer[len] = '\0';
					size = len;
				} else
					size = len;
				SetIoErr(lv->lv_Len);
				return size;
			}
		}
		
		if ((flags & 0xff) == LV_VAR && !(flags & GVF_LOCAL_ONLY))
		{
			INT32 ret = -1;

			//ret = getvar_from(name, "ENV:", buffer, size, flags, DOSBase);
			if (ret >= 0) return ret;
			//ret = getvar_from(name, "ENVARC:", buffer, size, flags, DOSBase);
			if (ret >= 0) return ret;

		}
	}
	SetIoErr(ERROR_OBJECT_NOT_FOUND);
	return -1;
		
#if 0
		if (!(dirlock = Lock("ENV:", SHARED_LOCK))) return -1;
		olddir = CurrentDir(dirlock);

		len = -1;
		if ((fh = Open(name, MODE_OLDFILE)))
		{
			buffer[0] = '\0';
			len = Read(fh, buffer, size);
			if (len > 0)
			{
				if (!(flags & GVF_DONT_NULL_TERM)) 
				{
					if (len == size) len--;
					buffer[len] = '\0';
				}

				if (!(flags & GVF_BINARY_VAR))
				{
					UINT8 *s = buffer;

					while (*s)
					{
						if (*s++ == '\n') *--s = '\0';
					}
					len = s - buffer;
				}
			}

			if (len >= 0 && Seek(fh,0L,OFFSET_END) >= 0)
			{
				fsize = Seek(fh,0L,OFFSET_END);
			} else
				len = -1;
		}

		err = IoErr();
		if (fh) Close(fh);
		UnLock(CurrentDir(olddir));

		if (len >= 0) err = fsize;
		SetIoErr(err);
	}
#endif
	return len;
}

INT32 dos_SetVar(pDOSBase DOSBase, const STRPTR name, UINT8 *buffer, INT32 size, UINT32 flags)
{
	struct LocalVar *lv;
	struct FileHandle *fh;
	struct FileLock *dirlock;
	struct FileLock *olddir;
	INT32 rc = DOSIO_FALSE, err;

	if (buffer && size == -1) size = Strlen((STRPTR)buffer);


	if (!(flags & GVF_GLOBAL_ONLY))
	{
		UINT8 *newvalue = NULL;

		if (buffer && size)
			if (!(newvalue = AllocVec(size,0L))) return DOSIO_FALSE;

		lv = FindVar(name, flags);
		if (lv) 
		{
			FreeVec(lv->lv_Value);

			if (buffer == NULL)
			{
				Remove(&(lv->lv_Node));
				FreeVec(lv);
				return DOSIO_TRUE;
			}
			
	fill_in:
			lv->lv_Value = newvalue;
			lv->lv_Len   = size;
			lv->lv_Flags = flags & GVF_BINARY_VAR;
			if (size) CopyMem(buffer, newvalue, size);
			return DOSIO_TRUE;
		}
		
		if (buffer == NULL)
		{
			goto do_global;
		}

		/* doesn't exist - create it as local - may override global */
		if (!(lv = AllocVec(sizeof(*lv) + Strlen(name) + 1, MEMF_FAST|MEMF_CLEAR)))
		{
			if (newvalue) FreeVec(newvalue);
			return DOSIO_FALSE;
		}

		lv->lv_Node.ln_Type = flags & 0xff;
		lv->lv_Node.ln_Name = ((char *) lv) + sizeof(*lv);
		Strcpy(lv->lv_Node.ln_Name,name);
		struct Process  *pr = (struct Process *)FindTask(NULL);
		_AddVar(DOSBase, (struct List *) &(pr->pr_LocalVars), lv);
		goto fill_in;

	}


do_global:
	if ((flags & GVF_LOCAL_ONLY) || ((flags & 0xff) != LV_VAR))
	{
		SetIoErr(ERROR_OBJECT_NOT_FOUND);
		return DOSIO_FALSE;
	}

	if (!(dirlock = Lock("ENV:", SHARED_LOCK))) return DOSIO_FALSE;

	err = 0;
	while (dirlock)
	{
		olddir = CurrentDir(dirlock);

		if (buffer == NULL)
		{
			rc = DeleteFile(name);
			goto free_dir;
		}

		if ((fh = Open(name,MODE_READWRITE)))
		{
			if (Write(fh,buffer,size) == size)
			{
				if (SetFileSize(fh,0,OFFSET_CURRENT) != -1) rc = DOSIO_TRUE;
			}
			if (!Close(fh)) rc = DOSIO_FALSE;
		} else {
			err = IoErr();
			if (err != ERROR_OBJECT_NOT_FOUND)
			{
				UnLock(CurrentDir(olddir));
				break;
			}

			if (dos_int_MakeDir(DOSBase, name))
			{
				CurrentDir(olddir);
				continue;
			} else
				UnLock(CurrentDir(olddir));
		}
free_dir:
		UnLock(CurrentDir(olddir));
		dirlock = NULL;
		if (flags & GVF_SAVE_VAR)
		{
			flags &= ~GVF_SAVE_VAR;
			dirlock = Lock("ENVARC:",SHARED_LOCK);
		}
	}

	SetIoErr(err);
	return rc;
}



