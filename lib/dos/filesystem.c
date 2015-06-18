/**
* File: /filesystem．c
* User: cycl0ne
* Date: 2014-10-24
* Time: 04:20 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "dosbase_private.h"
#define DoPkt2(a,b,c,d)		DoPkt(a, b, (INT32)c, (INT32)d,0,0,0)

static DOSIO int_HandlerAction(pDOSBase DOSBase, STRPTR name, INT32 arg1, INT32 arg2, INT32 action)
{
	pHandlerProc hp = NULL;
	DOSIO		 ret = 0;
	while(1)
	{
		hp = ObtainHandler(name);
		if (!hp) break;
		ret = DoPkt2(hp->hp_Port, action, arg1, arg2);
		//if (!ret && IoErr() == ERROR_OBJECT_NOT_FOUND) continue;		
		//this code needs to be activated with the error-report
		//if (ret) break;//ErrorReport
		ReleaseHandler(hp);
		hp = NULL;
		return ret;
	}

	ReleaseHandler(hp);
	return DOSIO_FALSE;
}

DOSIO dos_AddBuffers(pDOSBase DOSBase, STRPTR name, INT32 number)
{
	return int_HandlerAction(DOSBase, name, number, 0, ACTION_MORE_CACHE);
}

DOSIO dos_Inhibit(pDOSBase DOSBase, STRPTR name, INT32 on)
{
	return int_HandlerAction(DOSBase, name, on, 0, ACTION_INHIBIT);	
}

DOSIO dos_Rename(pDOSBase DOSBase, STRPTR fromname, STRPTR toname)
{
	pHandlerProc	fromHP, toHP;
	DOSIO			rc = 0; 

	while(1)
	{
		fromHP = ObtainHandler(fromname);
		if (fromname == NULL) break;
		toHP = ObtainHandler(toname);
		if (toname == NULL) break;
		
		if (fromHP->hp_Port != toHP->hp_Port)
		{
			SetIoErr(ERROR_RENAME_ACROSS_DEVICES);
			break;
		}
		rc = DoPkt(fromHP->hp_Port, ACTION_RENAME_OBJECT, (INT32)fromHP->hp_Lock, (INT32)fromname, (INT32)toHP->hp_Lock, (INT32)toname, 0);
//		if (rc) break; //TODO: ERR_REPORT()
		break;
	}
	if (fromHP) ReleaseHandler(fromHP);
	if (toHP) ReleaseHandler(toHP);
	return rc;
}

DOSIO dos_IsInteractive(pDOSBase DOSBase, pFileHandle fh)
{
	return (fh->fh_Type ? DOSIO_TRUE : DOSIO_FALSE);
}

DOSIO dos_IsFileSystem(pDOSBase DOSBase, STRPTR name)
{
	DOSIO rc;
	
	if ((!Strnicmp(name, "NIL:", 4)) ||
		(!Strnicmp(name, "CONSOLE:", 4)))
	{
		SetIoErr(ERROR_ACTION_NOT_KNOWN);
		return DOSIO_FALSE;
	}
	
	rc = int_HandlerAction(DOSBase, name, 0, 0, ACTION_IS_FILESYSTEM);
	return rc;
}

DOSIO dos_SameDevice(pDOSBase DOSBase, pFileLock lock1, pFileLock lock2)
{
	// 1. All Handlers are PURE, 2. Every Handler is spawned per Partition/Disk
	// == so the Fl_Handler needs to be exact identical  = they are same!
	if (lock1->fl_Handler == lock2->fl_Handler) return DOSIO_TRUE;
	return DOSIO_FALSE;	
}

#define GET(c)    c=FGetC(Input());
#define UNGET() UnGetC(Input(),-1);
#define EOF ENDSTREAMCH

INT32 dos_ReadItem(pDOSBase DOSBase, STRPTR buffer, INT32 maxchars, APTR input)
{
	STRPTR	b = buffer;
	INT32	c;

	//input validation
	if (buffer == NULL)
		return ITEM_NOTHING;

	if (!maxchars)
	{
		*b = 0;
		return ITEM_NOTHING;
	}

	//eat all empty spaces
	do
	{
		GET(c);
	}
	while (c==' '||c=='\t');

	//if empty command given, return
	if(c=='\0'||c=='\n'||c==EOF||c==';')
	{
		*b=0;
		if (c=='\0'||c=='\n'||c==';')
			UNGET();
		return ITEM_NOTHING;
	}
	else if(c=='=')
	{
		/* Found '='. Return it. */
		*b=0;
		return ITEM_EQUAL;
	}
	else if(c=='\"')
	{
		/* Quoted item found. Convert Contents. */
		for(;;)
		{
			if(!maxchars)
			{
				b[-1]=0;
				return ITEM_NOTHING;
			}
			maxchars--;

			GET(c);

			/* Convert ** to *, *" to ", *n to \n and *e to 0x1b. */
			if(c=='*')
			{
				GET(c);

				/* Check for premature end of line. */
				if(c=='\0'||c=='\n'||c==EOF)
				{
					if(c=='\0'||c=='\n')
						UNGET();
					*b=0;
					return ITEM_ERROR;
				}
				else if(c=='n'||c=='N')
					c='\n';
				else if(c=='e'||c=='E')
					c=0x1b;
			}
			else if(c=='\0'||c=='\n'||c==EOF)
			{
				if(c=='\0'||c=='\n')
					UNGET();
				*b=0;
				return ITEM_ERROR;
			}
			else if(c=='\"')
			{
				/* " ends the item. */
				*b=0;
				return ITEM_QUOTED;
			}
			*b++=c;
		}
	}
	else
	{
		/* Unquoted item. Store first character. */
		if(!maxchars)
		{
			b[-1]=0;
			return ITEM_ERROR;
		}
		maxchars--;
		*b++=c;
		/* Read up to the next terminator. */
		for(;;)
		{
			if(!maxchars)
			{
				b[-1]=0;
				return ITEM_ERROR;
			}
			maxchars--;
			GET(c);

			/* Check for terminator */
			if(c=='\0'||c==' '||c=='\t'||c=='\n'||c=='='||c==EOF||c==';')
			{
				if (c=='\0'||c=='\n'||c==';') //UnGetC(Input(), c);
					UNGET();
				*b=0;
				return ITEM_UNQUOTED;
			}
			*b++=c;
		}
	}
	return 0;
}
