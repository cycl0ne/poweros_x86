#include "types.h"
#include "list.h"
#include "ports.h"
#include "sysbase.h"
#include "device.h"
#include "io.h"
#include "resident.h"

#include "exec_funcs.h"


void INTERN_MakeFunctions(APTR target, APTR functionArray);
UINT32 INTERN_CountFunc(APTR functionArray);

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

	if(resident->rt_MatchWord != RTC_MATCHWORD || resident->rt_MatchTag != resident) return NULL;

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
	        library->lib_Flags	      = LIBF_SUMUSED | LIBF_CHANGED;

			switch(resident->rt_Type)
			{
			case NT_DEVICE:
				AddDevice((struct Device *)library);
				break;
			case NT_LIBRARY:
				AddLibrary(library);
				break;
			case NT_RESOURCE:
				//AddResource(library);
				break;
			}
	    }
    }
    return library;
}

BOOL lib_RomTagScanner(SysBase *SysBase, UINT32 *start, UINT32 *end)
{
    struct List         *mods= &SysBase->ResidentList;
    struct Resident     *res = NULL;
    struct ResidentNode *node = NULL;
    UINT8 *ptr = (UINT8 *)start;

    //DPrintF("RomTagScanner - Start: %x End: %x\n", start, end);

    while( ptr <= (UINT8 *)end)
    {
        res = (struct Resident *)ptr;

        // Check for a Resident Structure
        if (res->rt_MatchWord == RTC_MATCHWORD && res->rt_MatchTag == res)
        {
            DPrintF("RomTagScanner - Found Resident: %s\n",res->rt_Name);
            node = AllocVec(sizeof(struct ResidentNode), MEMF_FAST); //MEMF_CLEAR|MEMF_PUBLIC);
            if (!node) return FALSE;
            node->rn_Resident    = res;
            node->rn_Node.ln_Pri = res->rt_Pri;
            // Enqueue found Resident
            Enqueue(mods, &node->rn_Node);
        }
        ptr++;
    }
    return TRUE;
}


struct Library *lib_MakeLibrary(SysBase *SysBase, APTR funcTable, APTR structInit, UINT32(*libInit)(struct Library*,APTR, struct SysBase*), UINT32 dataSize, UINT32 segList)
{
  struct Library *library;

  UINT32 negativeLibrarySize = INTERN_CountFunc(funcTable);

  library=(struct Library *)AllocVec(dataSize+negativeLibrarySize,MEMF_FAST);//MEMF_CLEAR);

  /* Initilize the library */
  if(library!=NULL)
  {
    /* Get real library base */
    library=(struct Library *)((char *)library+negativeLibrarySize);

    /* Build jumptable */
    MakeFunctions(library, funcTable,(UINT32)NULL); // Function Pointers only

    library->lib_NegSize=(UINT16)negativeLibrarySize;  // Negsize
    library->lib_PosSize=(UINT16)dataSize; // and DataSize the correct Values

    if(structInit!=NULL)
	{
		//InitStruct(structInit,library,0); // Create Structure
	}

    if(libInit!=NULL)
    {
		library = (((struct Library*(*)(struct Library *,APTR, struct SysBase *)) libInit)(library, (APTR)segList, SysBase));
	}

  }
  return library;
}

void lib_MakeFunctions(SysBase *SysBase, APTR target, APTR functionArray, UINT32 funcDispBase)
{
	if(funcDispBase != NULL)
		return;

    /* If FuncDispBase is NULL it's an array of function pointers */
	INTERN_MakeFunctions(target, functionArray);

}

