/**
* File: /expansion．h
* User: cycl0ne
* Date: 2014-11-18
* Time: 12:24 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef expansion_H
#define expansion_H
#include "types.h"
#include "libraries.h"
#include "lists.h"
#include "semaphores.h"
#include "dos_io.h"

#include "exec_interface.h"

typedef struct ExpansionBase 
{
	struct Library			Library;
	APTR					lib_SysBase;
	APTR					DosBase;
	UINT32					Flags;
	struct SignalSemaphore	BoardListLock;
	List					BoardList; // Holds the PCI/PCIe Board list (not used on Raspi)
	struct SignalSemaphore	MountListLock;
	List					MountList;
} ExpansionBase_t, *pExpansionBase;

struct ExpDosNode
{
	STRPTR	dosName;	//HD0/DF0....
	STRPTR	execName;	//trackdisk.device
	UINT32	unitNum;	// for the ExecDevice
	UINT32	unitFlags;
	pDosEnvec dosEnv;
};

#endif
