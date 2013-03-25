/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: findpcidevice.c,v 1.2 2004/09/11 16:33:17 cycl0ne Exp $
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

struct PCINode *exp_FindPCIDevice(struct ExpansionBase *ExpansionBase, INT32 venid, INT32 devid)
{
    INT32 i;
    struct PCIBus *bus;
    struct PCINode *node;
 
    for (i=0; i<MAX_PCI_BUSSES;i++)
    {
        bus = &ExpansionBase->pciBus[i];
        if (!IsListEmpty(&bus->pci_Node))
        {
            ForeachNode(&bus->pci_Node, node)
            {
                if (node->nVendorID==venid && node->nDeviceID==devid) return node;
            }
        }
    }
    return NULL;
}

