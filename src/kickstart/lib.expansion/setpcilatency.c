/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: setpcilatency.c,v 1.1 2004/09/12 18:24:02 cycl0ne Exp $
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
#include "expansion_funcs.h"


void exp_SetPCILatency(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, UINT8 nLatency)
{
	WritePCIConfig( nBusNum, nDevNum, nFncNum, PCI_LATENCY, 1, nLatency );
}


