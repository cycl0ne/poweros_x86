/*
*   Copyright © 2004, Claus Herrmann. All rights reserved.
*   $Id: scanpcibus.c,v 1.2 2004/09/11 16:33:17 cycl0ne Exp $
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
#include "io_ports.h"
#include "pci.h"
#include "expansion_funcs.h"
#include "exec_funcs.h"

#define SysBase ExpansionBase->SysBase


static INT32 pci_int_FillPCINode(struct ExpansionBase *ExpansionBase, struct PCINode *pciNode, INT32 nBusNum, INT32 nDevNum, INT32 nFncNum)
{
	INT32 i;

    if (pciNode == NULL) return FALSE;

	pciNode->nBus = nBusNum;
	pciNode->nDevice = nDevNum;
	pciNode->nFunction = nFncNum;

	pciNode->nVendorID = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_VENDOR_ID, 2 );
	pciNode->nDeviceID = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_DEVICE_ID, 2 );
	pciNode->nCommand = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_COMMAND, 2 );
	pciNode->nStatus = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_STATUS, 2 );
	pciNode->nRevisionID = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_REVISION, 1 );

	pciNode->nClassApi = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_CLASS_API, 1 );
	pciNode->nClassBase = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_CLASS_BASE, 1 );
	pciNode->nClassSub = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_CLASS_SUB, 1 );

	pciNode->nCacheLineSize = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_LINE_SIZE, 1 );
	pciNode->nLatencyTimer = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_LATENCY, 1 );
	pciNode->nHeaderType = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_HEADER_TYPE, 1 );

	pciNode->nBase0 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 0, 4 );
	pciNode->nBase1 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 4, 4 );
	pciNode->nBase2 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 8, 4 );
	pciNode->nBase3 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 12, 4 );
	pciNode->nBase4 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 16, 4 );
	pciNode->nBase5 = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_BASE_REGISTERS + 20, 4 );

	pciNode->nCISPointer = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_CARDBUS_CIS, 4 );
	pciNode->nSubSysVendorID = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_SUBSYSTEM_VENDOR_ID, 2 );
	pciNode->nSubSysID = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_SUBSYSTEM_ID, 2 );
	pciNode->nExpROMAddr = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_ROM_BASE, 4 );
	pciNode->nCapabilityList = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_CAPABILITY_LIST, 1 );
	pciNode->nInterruptLine = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_INTERRUPT_LINE, 1 );
	pciNode->nInterruptPin = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_INTERRUPT_PIN, 1 );
	pciNode->nMinDMATime = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_MIN_GRANT, 1 );
	pciNode->nMaxDMALatency = ReadPCIConfig( nBusNum, nDevNum, nFncNum, PCI_MAX_LATENCY, 1 );


	/* Reserve memory */
/*
	for ( i = 0; i < 6; i++ )
	{
		uint32 nAddr = *( &psInfo->u.h0.nBase0 + i );

		if ( ( nAddr & PCI_ADDRESS_SPACE ) || ( nAddr == 0x00000000 ) || ( nAddr == 0xffffffff ) )
			continue;
			
		nAddr = nAddr & PCI_ADDRESS_MEMORY_32_MASK;
		
		alloc_physical( &nAddr, true, get_pci_memory_size( nBusNum, nDevNum, nFncNum, i ) );
	}
*/
	return( 0 );
}

void exp_ScanPCIBus(struct ExpansionBase *ExpansionBase, INT32 nBusNum, INT32 nBridgeFrom, INT32 nBusDev)
{
	//PCI_Entry_s *psInfo;
	struct PCINode *pciNode;
	
	INT32 nDeviceNumberPerBus = ( ExpansionBase->g_nPCIMethod & PCI_METHOD_1 ) ? 32 : 16;
	INT32 nDev, nFnc;
	UINT32 nVendorID;
	UINT8 nHeaderType = 0;

	/* Allocate Resources for the bus */
	ExpansionBase->g_nPCINumBusses++;

    ExpansionBase->pciBus[nBusNum].PCIDeviceNumber = nBusDev;
    ExpansionBase->pciBus[nBusNum].PrimaryBus      = nBridgeFrom;
    ExpansionBase->pciBus[nBusNum].SecondaryBus    = nBusNum;

	DPrintF( "PCI: Scanning Bus %d\n", nBusNum );

	/* Look for devices on this bus */
	for ( nDev = 0; nDev < nDeviceNumberPerBus; nDev++ )
	{
		for ( nFnc = 0; nFnc < 8; nFnc++ )
		{
			nVendorID = ReadPCIConfig( nBusNum, nDev, nFnc, PCI_VENDOR_ID, 2);
			/* One device has been found */
			if ( nVendorID != 0xffff && nVendorID != 0x0000 )
			{
				/* If it is not a multifunction device than we do not have to continue scanning */
				if ( nFnc == 0 )
				{
					nHeaderType = ReadPCIConfig( nBusNum, nDev, nFnc, PCI_HEADER_TYPE, 1);
				}
				else
				{
					if ( ( nHeaderType & PCI_MULTIFUNCTION ) == 0 )
						continue;
				}

				/* Allocate resources for the new device */
                pciNode = AllocVec(sizeof(struct PCINode), MEMF_PUBLIC|MEMF_CLEAR);

				if ( pciNode != NULL )
				{
				    
					pci_int_FillPCINode(ExpansionBase, pciNode, nBusNum, nDev, nFnc);

					if ( ExpansionBase->g_nPCINumDevices < MAX_PCI_DEVICES )
					{
						AddTail(&ExpansionBase->pciBus[nBusNum].pci_Node, (struct Node *)&pciNode->pci_Node);
						ExpansionBase->g_nPCINumDevices++;
						DPrintF( "PCI: Device %d VendorID: %x DeviceID: %x at %d:%d:%d\n", ExpansionBase->g_nPCINumDevices - 1, pciNode->nVendorID, pciNode->nDeviceID, nBusNum, nDev, nFnc );
					}
					else
					{
						DPrintF( "WARNING : To many PCI devices!\n" );
					}
				}

				/* Register device */


				/* Look if the device is a bridge */
				if ( nHeaderType & PCI_HEADER_BRIDGE )
				{
					//psInfo->nHandle = register_device( "", PCI_BUS_NAME );
					/*claim_device( -1, psInfo->nHandle, "PCI->PCI/AGP Bridge", DEVICE_SYSTEM );*/
					/* Disable bridge */
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_COMMAND, 2, ReadPCIConfig( nBusNum, nDev, nFnc, PCI_COMMAND, 2) & ~( PCI_COMMAND_IO | PCI_COMMAND_MEMORY ));

					/* Write new values */
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_BUS_PRIMARY, 1, nBusNum);
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_BUS_SECONDARY, 1, ExpansionBase->g_nPCINumBusses);
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_BUS_SUBORDINATE, 1, 0xff);
					ScanPCIBus( ExpansionBase->g_nPCINumBusses, nBusNum, ExpansionBase->g_nPCINumDevices - 1);
					/* Note : g_nPCINumBusses - 1 because the number of busses will be increased by 
					   pci_scan_bus() */
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_BUS_SUBORDINATE, 1, ExpansionBase->g_nPCINumBusses - 1);
					/* Enable bridge */
					WritePCIConfig( nBusNum, nDev, nFnc, PCI_COMMAND, 2, ReadPCIConfig( nBusNum, nDev, nFnc, PCI_COMMAND, 2) | ( PCI_COMMAND_IO | PCI_COMMAND_MEMORY ) );
				}
			}
		}
	}
	DPrintF( "PCI: Scan of bus finished\n" );
}


