/**
 * @file libraries.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

APTR lib_InitResident(SysBase *SysBase, struct Resident *resident, APTR segList)
{
	struct init {
        UINT32 dSize;
        APTR vectors;
        APTR structure;
        UINT32 (*init)();
	};
	
	struct init *init = (struct init *)resident->rt_Init;
	struct Library *library = NULL;
	
	if(resident->rt_MatchWord != RTC_MATCHWORD || resident->rt_MatchTag != resident) 
	{
		KPrintF("ERROR in RESIDENT\n");
		return NULL;
	}

	// Depending on the autoinit flag...
	if(resident->rt_Flags & RTF_AUTOINIT)
	{
    	library = MakeLibrary(init->vectors, init->structure, init->init, init->dSize,(UINT32) segList);

	    if(library != NULL)
	    {
    	    library->lib_Node.ln_Type = resident->rt_Type;
	        library->lib_Node.ln_Name = resident->rt_Name;
	        library->lib_Version      = resident->rt_Version;
	        library->lib_IDString     = resident->rt_IdString;
	        library->lib_Flags	      = LIBF_SUMUSED|LIBF_CHANGED;

			switch(resident->rt_Type)
			{
			case NT_DEVICE:
				AddDevice((struct Device *)library);
				break;
			case NT_LIBRARY:
				AddLibrary(library);
				break;
			case NT_RESOURCE:
//        			AddResource(library);
				break;
			}
		}
	} else
	{
		//KPrintF("Starting Init, w/o AUTOINIT [%x]\n", library);
		library = (((struct Library*(*)(pLibrary ,APTR, pSysBase)) init->init)(library, (APTR)segList, SysBase));
	}
	return library;
}

SysCall lib_InitResidentCode(struct SysBase *SysBase, UINT32 startClass)
{
	pResidentNode res;
	switch (startClass&~RTF_AUTOINIT)
	{
		case RTF_AFTERDOS:
			KPrintF("--------------RTF_AFTERDOS---------------\n");
			break;
		case RTF_COLDSTART:
			KPrintF("--------------RTF_COLDSTART--------------\n");
			break;
		case RTF_SINGLETASK:
			KPrintF("--------------RTF_SINGLETASK-------------\n");
			break;
		case RTF_TESTCASE:
			KPrintF("--------------RTF_TESTCASE---------------\n");
			break;
	}

	ForeachNode(&SysBase->ResidentList, res)
	{
        if ((res->rn_Resident->rt_Flags & startClass) == startClass)
        {
			KPrintF("InitResident %s (%x)\n", res->rn_Resident->rt_Name,res->rn_Resident);
            if (InitResident(res->rn_Resident, NULL)== NULL)
            {
				KPrintF("[INIT] InitResidentCode Scanner Failed \n");
				KPrintF("on Resident [%x] name: %s\n", res->rn_Resident, res->rn_Resident->rt_Name);
				KPrintF("System is not halted, but no new modules loaded\n");
				return SYSERR;
				//for(;;);
            }
		}
	}
	return OK;
}

pResidentNode lib_FindResident(struct SysBase *SysBase, STRPTR name)
{
	return(pResidentNode) FindName(&SysBase->ResidentList, name);
}
