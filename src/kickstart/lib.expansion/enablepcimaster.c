/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: enablepcimaster.c,v 1.2 2004/09/12 19:37:17 cycl0ne Exp $
*
*   NAME
*	EnablePCIMaster -- Enable the PCI Master
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   BUGS
*
*   SEE ALSO
*
*/

#include "expansionbase.h"
#include "pci.h"
#include "expansion_funcs.h"

void exp_EnablePCIMaster(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum)
{
	WritePCIConfig( nBusNum, nDevNum, nFncNum, PCI_COMMAND, 2, 
				ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_COMMAND, 2 )
				| PCI_COMMAND_MASTER);
}

