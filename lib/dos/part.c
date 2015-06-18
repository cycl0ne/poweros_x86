/**
 * @file part.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

INT32 dos_SplitName(pDOSBase DOSBase, STRPTR name, INT32 seperator, STRPTR buf, INT32 oldpos, INT32 size)
{
	size--;
	name += oldpos;
	
	while(*name != seperator && *name && size)
	{
		size--;
		*buf++ = *name++;
		oldpos ++;
    }

    *buf = 0;

    if (*name == seperator) return oldpos + 1;
    return -1;
}

STRPTR dos_FilePart(pDOSBase DOSBase, const STRPTR path)
{
	STRPTR p;
	p = PathPart(path);
	if (*p == '/') p++;
	return p;
}

STRPTR dos_PathPart (pDOSBase DOSBase, const STRPTR path)
{
	STRPTR p;

	if ((p = Strrchr(path,'/')) == NULL)
	{
		if ((p = Strchr(path,':')) == NULL)
			return path;
		else
			return p+1;
	}
	if ((p == path)	|| 	
	    (*(p-1) == '/') ||
	    (*(p-1) == ':'))	
		p++;

	return p;
}

DOSIO dos_AddPart(pDOSBase DOSBase, STRPTR dirname, STRPTR filename, UINT32 size )
{
	STRPTR p;
	BOOL	slash = FALSE;
	
	if (Strchr(filename, ':'))
	{
		if (*filename == ':')
		{
			p = Strchr(dirname, ':');
			if (!p) p = dirname;
		} else
			p = dirname;
	} else 
	{
		p = dirname + Strlen(dirname);
		if (*dirname)
		{
			if (*(p-1) != ':' && *(p-1) != '/')
			{
				slash = TRUE;
				*p++ = '/'; 
			}
		}
	}
	if ((p-dirname) + Strlen(filename) + 1 > size)
	{
		if (slash) *--p = '\0';	
		SetIoErr(ERROR_LINE_TOO_LONG);
		return FALSE;
	}

	Strcpy(p,filename);
	return -1;
}

