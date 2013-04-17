#include "intuitionbase.h"

#define SysBase IBase->ib_SysBase

UINT32 intu_LockIBase(IntuitionBase *IBase, UINT32 lock)
{
	if (!lock)
	{
		ObtainSemaphore(&IBase->ib_Locks[ISTATELOCK]);
		ObtainSemaphore(&IBase->ib_Locks[IBASELOCK]);
	} else
	{
		ObtainSemaphore(&IBase->ib_Locks[lock]);
	}
	return lock;
}

void intu_UnlockIBase(IntuitionBase *IBase, UINT32 lock)
{
	if (!lock)
	{
		ReleaseSemaphore(&IBase->ib_Locks[IBASELOCK]);
		ReleaseSemaphore(&IBase->ib_Locks[ISTATELOCK]);
	} else
	{
		ReleaseSemaphore(&IBase->ib_Locks[lock]);
	}
}
