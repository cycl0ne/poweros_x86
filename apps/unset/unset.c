/**
* File: /stack．c
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

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"UnSet 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME" CMDREV
#define OPT_NAME    0
#define OPT_STRING  1
#define OPT_COUNT   2

//DOSCALL cmd_unset(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_ERROR;
	INT32 	opts[OPT_COUNT];

	INT32	mode =  GVF_LOCAL_ONLY|LV_VAR;
	INT32	len;
	UINT8 	buffer[256];
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else if (opts[OPT_NAME] && (Strlen((STRPTR) opts[OPT_NAME])>255))
			{
				PrintFault(120,NULL);
			} else
			{
				rc = RETURN_OK;
				
				if (opts[OPT_NAME])
				{
					
				} else
				{
					pProcess this = FindProcess(NULL);
					pLocalVar lv  = NULL;
					ForeachNode((pList)&this->pr_LocalVars, lv)
					{
						if (CheckSignal(SIGBREAKF_CTRL_C)) 
						{
							PrintFault(304,0);
							break;
						}
						if ((lv->lv_Flags & GVF_BINARY_VAR)) 
						{
							Strcpy(buffer,"[BINARY]");
							len=8;
						} else {
							len = MIN(lv->lv_Len, 255);
							CopyMem(lv->lv_Value,buffer,len);
							buffer[len] = '\0';
						}
						for (INT32 i; i < len; i++)
						{
							if ( (buffer[i] == 0x1b) || (buffer[i] == 0x9b)) buffer[i] = 127;	
						}
						if ((mode&255) == (lv->lv_Node.ln_Type)&255)
							Printf("%-17s %s\n", lv->lv_Node.ln_Name,buffer);
					}
				}
				FreeArgs(rdargs);
			}
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	return rc;
}

