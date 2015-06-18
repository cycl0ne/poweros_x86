/**
 * @file libraries.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

struct Library *lib_OpenLibrary(SysBase *SysBase, STRPTR libName, UINT32 version)
{
	struct Library *library;
	if (libName == NULL) return NULL;
	Forbid();
	library = (struct Library *) FindName(&SysBase->LibList, libName);
	Permit();

	if(library!=NULL)
	{
		if(library->lib_Version>=version)
		{
			library = _LIBCALL(library, LIB_OPEN, pLibrary, (pLibrary), (library));
			//library = (((struct Library *(*)(struct Library *)) _GETVECADDR(library, LIB_OPEN))(library));
		}
		else library = NULL;
	}
	return library;
}

SysCall lib_CloseLibrary(SysBase *SysBase,struct Library *library)
{
	if (library == NULL) return SYSERR;
	Forbid();
	_LIBCALL(library, LIB_CLOSE, void, (pLibrary), (library));
	//(((void(*)(struct Library *)) _GETVECADDR(library,LIB_CLOSE))(library));
	Permit();
	return OK;
}

SysCall lib_AddLibrary(SysBase *SysBase,struct Library *library)
{
	library->lib_Node.ln_Type=NT_LIBRARY;
	library->lib_Flags|=LIBF_CHANGED;
	SumLibrary(library);

	Forbid();
	Enqueue(&SysBase->LibList,&library->lib_Node);
	Permit();
	return OK;
}

SysCall lib_RemLibrary(SysBase *SysBase, struct Library *library)
{
	if (library == NULL) return NULL;
	SysCall ret;
	Forbid();
	Remove(&library->lib_Node);
	Permit();
	ret = _LIBCALL(library, LIB_EXPUNGE, uint32_t, (pLibrary), (library));
	//ret = (((UINT32(*)(struct Library *)) _GETVECADDR(library, LIB_EXPUNGE))(library));
	return ret;
}

void lib_SumLibrary(SysBase *SysBase, struct Library *library)
{
	//TODO: Write some nice code here. ;)
	return;
}

SysCall lib_DisposeLibrary(SysBase *SysBase, struct Library* library)
{
	if(library)
	{
		library = (struct Library *)((char *)library - library->lib_NegSize);
		FreeVec(library);
		return OK;
	}
	return SYSERR;
}

APTR lib_SetFunction(struct SysBase *SysBase, struct Library *library, INT32 funcOffset, APTR newFunction)
{
	APTR ret;
	UINT32 *vecaddr;
	
	Forbid();
	library->lib_Flags |= LIBF_CHANGED;
	vecaddr = (UINT32 *)((UINT32)library + funcOffset);
	ret     = (APTR *)*(UINT32 *)(((UINT32)vecaddr));
	*(UINT32 *)(((UINT32)vecaddr)) = (UINT32)newFunction;
	Permit();
	SumLibrary(library);
	return ret;
}

static inline UINT32 _CountFunc(APTR functionArray)
{
	UINT32 n=0;
	void **fp=(void **)functionArray;
	/* -1 terminates the array */
	while(*fp!=(void *)-1)
	{
		fp++;
		n++;
	}
	return n*4; //Evil, on 64 bit this should be 8 ! :-P
}

struct Library *lib_MakeLibrary(SysBase *SysBase, APTR funcTable, APTR structInit, UINT32(*libInit)(struct Library*,APTR, struct SysBase*), UINT32 dataSize, UINT32 segList)
{
	pLibrary library;
	UINT32 negativeLibrarySize = _CountFunc(funcTable);

	library=(pLibrary)AllocVec(dataSize+negativeLibrarySize,MEMF_FAST);    //MEMF_CLEAR);

	/* Initilize the library */
	if(library != NULL)
	{
		/* Get real library base */
		library = (pLibrary)((char *)library+negativeLibrarySize);

		/* Build jumptable */
		MakeFunctions(library, funcTable); // Function Pointers only
		if (structInit!=NULL) CopyMem(structInit, library, dataSize);

		library->lib_NegSize=(UINT16)negativeLibrarySize;  // Negsize
		library->lib_PosSize=(UINT16)dataSize; // and DataSize the correct Values

		//DPrintF("library negative size = %u\n", negativeLibrarySize);
		//DPrintF("library = %p\n", library);
		//DPrintF("library positive size = %u\n", dataSize);

		if(libInit != NULL)
		{
			library = (((struct Library*(*)(pLibrary ,APTR, pSysBase)) libInit)(library, (APTR)segList, SysBase));
		}

	}
	return library;
}

SysCall lib_MakeFunctions(SysBase *SysBase, APTR target, APTR functionArray)
{
	INT32 n = 1;
	APTR vector;
	void **fp = (void **)functionArray;

	while(*fp != (void*)-1)
	{
		vector = (APTR)((UINT32)target-n*4); // EVIL on 64bit, this should be 8!
		*((UINT32*)vector) = (UINT32) *fp;
		fp++;
		n++;
	}
	return SYSERR;
}


