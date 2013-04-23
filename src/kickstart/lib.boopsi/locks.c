#include "boopsibase.h"
#include "boopsi.h"
#include "exec_funcs.h"

#define SysBase BOOPSIBase->SysBase

void boopsi_LockClassList(pBOOPSI BOOPSIBase)
{
	ObtainSemaphore(&BOOPSIBase->LockClass);
}

void boopsi_UnlockClassList(pBOOPSI BOOPSIBase)
{
	ReleaseSemaphore(&BOOPSIBase->LockClass);
}

