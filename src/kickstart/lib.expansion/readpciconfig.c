/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: readpciconfig.c,v 1.3 2004/09/11 16:55:25 cycl0ne Exp $
*
*
*/

#include "expansionbase.h"
#include "io_ports.h"
#include "pci.h"
#include "expansion_funcs.h"
#include "exec_funcs.h"

#define SysBase ExpansionBase->SysBase


UINT32 exp_ReadPCIConfig(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum, INT32 nOffset, INT32 nSize)
{
	UINT32 nFlags;
	UINT32 nValue = 0;

	if ( 2 == nSize || 4 == nSize || 1 == nSize )
	{
		if ( ExpansionBase->g_nPCIMethod & PCI_METHOD_1 )
		{
			//spinlock_cli( &g_sPCILock, nFlags );
			outl( 0x80000000 | ( nBusNum << 16 ) | ( nDevNum << 11 ) | ( nFncNum << 8 ) | ( nOffset & ~3 ), 0x0cf8 );
			switch ( nSize )
			{
			case 1:
				nValue = inb( 0x0cfc + ( nOffset & 3 ) );
				break;
			case 2:
				nValue = inw( 0x0cfc + ( nOffset & 2 ) );
				break;
			case 4:
				nValue = inl( 0x0cfc );
				break;
			default:
			    break;
			}
			//spinunlock_restore( &g_sPCILock, nFlags );
			return ( nValue );
		}
		else if ( ExpansionBase->g_nPCIMethod & PCI_METHOD_2 )
		{
			if ( nDevNum >= 16 )
			{
				DPrintF( "PCI: read_pci_config() with an invalid device number\n" );
			}
			//spinlock_cli( &g_sPCILock, nFlags );
			outb( ( 0xf0 | ( nFncNum << 1 ) ), 0x0cf8 );
			outb( nBusNum, 0x0cfa );
			switch ( nSize )
			{
			case 1:
				nValue = inb( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 2:
				nValue = inw( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 4:
				nValue = inl( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			default:
			    break;
			}
			outb( 0x0, 0x0cf8 );
			//spinunlock_restore( &g_sPCILock, nFlags );
			return ( nValue );
		}
		else
		{
			DPrintF( "PCI: read_pci_config() called without PCI present\n" );
		}
	}
	else
	{
		DPrintF( "PCI: Invalid size %d passed to read_pci_config()\n", nSize );
	}
	return ( 0 );
}


