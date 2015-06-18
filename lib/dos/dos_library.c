
#include "dosbase_private.h"

APTR dos_OpenLib(pDOSBase DOSBase)
{
	if (DOSBase == NULL)
	{
		//g_SysBase = 
		//Initcode here :)		
	}
	DOSBase->dos_LibNode.lib_OpenCnt++;
	return DOSBase;
}

APTR dos_CloseLib(pDOSBase DOSBase)
{
	DOSBase->dos_LibNode.lib_OpenCnt--;
	return NULL;
}

APTR dos_ExpungeLib(pDOSBase DOSBase)
{
	return NULL;
}

APTR dos_ExtFuncLib(pDOSBase DOSBase)
{
	return NULL;
}

