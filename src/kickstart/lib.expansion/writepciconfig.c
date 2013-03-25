/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: writepciconfig.c,v 1.2 2004/09/11 16:33:17 cycl0ne Exp $
*
*
*/

#include "expansionbase.h"
#include "io_ports.h"
#include "pci.h"
#include "expansion_funcs.h"
#include "exec_funcs.h"

#define SysBase ExpansionBase->SysBase

BOOL exp_WritePCIConfig(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nOffset, INT32 nSize, UINT32 nValue)
{
	UINT32 nFlags;

	if ( 2 == nSize || 4 == nSize || 1 == nSize )
	{
		if ( ExpansionBase->g_nPCIMethod & PCI_METHOD_1 )
		{
			outl( 0x80000000 | ( nBusNum << 16 ) | ( nDevNum << 11 ) | ( nFncNum << 8 ) | ( nOffset & ~3 ), 0x0cf8 );
			switch ( nSize )
			{
			case 1:
				outb( nValue, 0x0cfc + ( nOffset & 3 ) );
				break;
			case 2:
				outw( nValue, 0x0cfc + ( nOffset & 2 ) );
				break;
			case 4:
				outl( nValue, 0x0cfc );
				break;
			default:
				return ( -1 );
			}
			return ( 0 );
		}
		else if ( ExpansionBase->g_nPCIMethod & PCI_METHOD_2 )
		{
			if ( nDevNum >= 16 )
			{
				DPrintF( "[EXPANSION] write_pci_config() with an invalid device number\n" );
			}
			outb( ( 0xf0 | ( nFncNum << 1 ) ), 0x0cf8 );
			outb( nBusNum, 0x0cfa );
			switch ( nSize )
			{
			case 1:
				outb( nValue, ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 2:
				outw( nValue, ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 4:
				outl( nValue, ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			default:
				return ( -1 );
			}
			outb( 0x0, 0x0cf8 );
			return ( 0 );
		}
		else
		{
			DPrintF( "[EXPANSION] write_pci_config() called without PCI present\n" );
		}
	}
	else
	{
		DPrintF( "[EXPANSION] Invalid size %d passed to write_pci_config()\n", nSize );
	}
	return ( -1 );
}


