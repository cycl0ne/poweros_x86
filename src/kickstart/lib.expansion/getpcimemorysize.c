/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: getpcimemorysize.c,v 1.1 2004/09/12 18:24:02 cycl0ne Exp $
*
*   NAME
*	GetPCIMemorySize -- Get PCI Memory Size
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
*	ScanBus
*
*/

#include "expansionbase.h"
#include "pci.h"
#include "expansion_funcs.h"


static UINT32 pci_size( UINT32 base, UINT32 mask )
{
	UINT32 size = base & mask;
	return size & ~( size - 1 );
}

UINT32 exp_GetPCIMemorySize(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nResource )
{
	UINT32 nSize, nBase;
	INT32 nOffset = PCI_BASE_REGISTERS + nResource * 4;

	nBase = ReadPCIConfig( nBusNum, nDevNum, nFncNum, nOffset, 4 );
	WritePCIConfig( nBusNum, nDevNum, nFncNum, nOffset, 4, ~0 );
	nSize = ReadPCIConfig( nBusNum, nDevNum, nFncNum, nOffset, 4 );
	WritePCIConfig( nBusNum, nDevNum, nFncNum, nOffset, 4, nBase );

	if ( !nSize || nSize == 0xffffffff ) return 0;
	if ( nBase == 0xffffffff )	nBase = 0;

	if ( !( nSize & PCI_ADDRESS_SPACE ) )
	{
		return pci_size( nSize, PCI_ADDRESS_MEMORY_32_MASK );
	}
	else
	{
		return pci_size( nSize, PCI_ADDRESS_IO_MASK & 0xffff );
	}
	return 0;
}


