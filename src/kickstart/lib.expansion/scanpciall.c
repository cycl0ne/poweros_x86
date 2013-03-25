/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: scanpciall.c,v 1.3 2004/09/11 16:55:25 cycl0ne Exp $
*
*   NAME
*	FindPCIDevice -- Find a given PCI Device on the PCI Bus
*
*   SYNOPSIS
*	FindPCIDevice(LONG venid, LONG devid)
*
*   FUNCTION
*	This Function scans the cached PCI List for the given 
*   PCI Device. 
*
*   INPUTS
*	venid - Vendor ID to be looked for
*   devid - Device ID to be looked for
*
*   BUGS
*
*   SEE ALSO
*	ScanBus
*
*/
#include "expansionbase.h"
#include "pci.h"
#include "expansion_funcs.h"
#include "exec_funcs.h"

#define SysBase ExpansionBase->SysBase

void exp_ScanPCIAll(struct ExpansionBase *ExpansionBase)
{
	if ( ExpansionBase->g_nPCIMethod == 0 ) return;
	/* Scan first bus */
	ScanPCIBus(0, -1, -1);
	
	DPrintF( "PCI: %i devices detected\n", ExpansionBase->g_nPCINumDevices );
}

