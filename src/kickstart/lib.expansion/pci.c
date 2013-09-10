#include "expansionbase.h"

#include "exec_funcs.h"
#include "expansion_protos.h"
#include "expansion_funcs.h"
#include "asm.h"

#include "pci.h"

static __inline__ void
IO_Out8(UINT16 port, UINT8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out16(UINT16 port, UINT16 value)
{
   __asm__ __volatile__ ("outw %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out32(UINT16 port, UINT32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static __inline__ UINT8
IO_In8(UINT16 port)
{
   UINT8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT16
IO_In16(UINT16 port)
{
   UINT16 value;
   __asm__ __volatile__ ("inw %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT32
IO_In32(UINT16 port)
{
   UINT32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

#define PCI_MAX_BUSSES          0x20
#define PCI_REG_CONFIG_ADDRESS  0xCF8
#define PCI_REG_CONFIG_DATA     0xCFC

#define offsetof(type, member)  ((UINT32)(&((type*)NULL)->member))

static inline UINT32 PCIConfigPackAddress(const PCIAddress *addr, UINT16 offset)
{
   const UINT32 enableBit = 0x80000000UL;

   return (((UINT32)addr->bus << 16) |
           ((UINT32)addr->device << 11) |
           ((UINT32)addr->function << 8) |
           offset | enableBit);
}

UINT32 PCI_ConfigRead32(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   return IO_In32(PCI_REG_CONFIG_DATA);
}

UINT16 PCI_ConfigRead16(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   return IO_In16(PCI_REG_CONFIG_DATA);
}

UINT8 PCI_ConfigRead8(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   return IO_In8(PCI_REG_CONFIG_DATA);
}

void PCI_ConfigWrite32(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT32 data)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   IO_Out32(PCI_REG_CONFIG_DATA, data);
}

void PCI_ConfigWrite16(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT16 data)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   IO_Out16(PCI_REG_CONFIG_DATA, data);
}

void PCI_ConfigWrite8(ExpansionBase *ExpBase, const PCIAddress *addr, UINT16 offset, UINT8 data)
{
   IO_Out32(PCI_REG_CONFIG_ADDRESS, PCIConfigPackAddress(addr, offset));
   IO_Out8(PCI_REG_CONFIG_DATA, data);
}


/*
 * PCI_ScanBus --
 *
 *    Scan the PCI bus for devices. Before starting a scan,
 *    the caller should zero the PCIScanState structure.
 *    Every time this function is called, it returns the next
 *    device in sequence.
 *
 *    Returns TRUE if a device was found, leaving that device's
 *    vendorId, productId, and address in 'state'.
 *
 *    Returns FALSE if there are no more devices.
 */

BOOL PCI_ScanBus(ExpansionBase *ExpBase, PCIScanState *state)
{
   PCIConfigSpace config;

   for (;;) {
      config.words[0] = PCI_ConfigRead32(ExpBase, &state->nextAddr, 0);

      state->addr = state->nextAddr;

      if (++state->nextAddr.function == 0x8) {
         state->nextAddr.function = 0;
         if (++state->nextAddr.device == 0x20) {
            state->nextAddr.device = 0;
            if (++state->nextAddr.bus == PCI_MAX_BUSSES) {
               return FALSE;
            }
         }
      }

      if (config.words[0] != 0xFFFFFFFFUL) {
         state->vendorId = config.vendorId;
         state->deviceId = config.deviceId;
         return TRUE;
      }
   }
}


/*
 * PCI_FindDevice --
 *
 *    Scan the PCI bus for a device with a specific vendor and device ID.
 *
 *    On success, returns TRUE and puts the device address into 'addrOut'.
 *    If the device was not found, returns FALSE.
 */

BOOL PCI_FindDevice(ExpansionBase *ExpBase, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut)
{
   PCIScanState busScan = {};

   while (PCI_ScanBus(ExpBase, &busScan)) {
      if (busScan.vendorId == vendorId && busScan.deviceId == deviceId) {
         *addrOut = busScan.addr;
         return TRUE;
      }
   }

   return FALSE;
}


/*
 * PCI_SetBAR --
 *
 *    Set one of a device's Base Address Registers to the provided value.
 */

void PCI_SetBAR(ExpansionBase *ExpBase, const PCIAddress *addr, INT32 index, UINT32 value)
{
   PCI_ConfigWrite32(ExpBase, addr, offsetof(PCIConfigSpace, BAR[index]), value);
}


/*
 * PCI_GetBARAddr --
 *
 *    Get the current address set in one of the device's Base Address Registers.
 *    We mask off the lower bits that are not part of the address.  IO bars are
 *    4 byte aligned so we mask lower 2 bits, and memory bars are 16-byte aligned
 *    so we mask the lower 4 bits.
 */
#define SysBase ExpBase->SysBase

UINT32 PCI_GetBARAddr(ExpansionBase *ExpBase, const PCIAddress *addr, INT32 index)
{	
	UINT32 bar = PCI_ConfigRead32(ExpBase, addr, offsetof(PCIConfigSpace, BAR[index]));
	UINT32 mask = (bar & PCI_CONF_BAR_IO) ? 0x3 : 0xf;
	return bar & ~mask;
}


/*
 * PCI_SetMemEnable --
 *
 *    Enable or disable a device's memory and IO space. This must be
 *    called to enable a device's resources after setting all
 *    applicable BARs. Also enables/disables bus mastering.
 */

void PCI_SetMemEnable(ExpansionBase *ExpBase, const PCIAddress *addr, BOOL enable)
{
   UINT16 command = PCI_ConfigRead16(ExpBase, addr, offsetof(PCIConfigSpace, command));

   /* Mem space enable, IO space enable, bus mastering. */
   const UINT16 flags = 0x0007;

   if (enable) {
      command |= flags;
   } else {
      command &= ~flags;
   }

   PCI_ConfigWrite16(ExpBase, addr, offsetof(PCIConfigSpace, command), command);
}


UINT8 PCI_GetIntrLine(ExpansionBase *ExpBase, const PCIAddress *addr)
{
	UINT8 line = PCI_ConfigRead8(ExpBase, addr, offsetof(PCIConfigSpace, intrLine));
	return line;
}

UINT8 PCI_GetIntrPin(ExpansionBase *ExpBase, const PCIAddress *addr)
{
	UINT8 pin = PCI_ConfigRead8(ExpBase, addr, offsetof(PCIConfigSpace, intrPin));
	return pin;
}

/*
 * PCI_FindDeviceByUnit --
 *
 *    Scan the PCI bus for a device with a specific vendor and device ID and unit.
 *
 *    On success, returns TRUE and puts the device address into 'addrOut'.
 *    If the device was not found, returns FALSE.
 */

BOOL PCI_FindDeviceByUnit(ExpansionBase *ExpBase, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut, INT32 unit)
{
	PCIScanState busScan = {};

	while (PCI_ScanBus(ExpBase, &busScan))
	{
		if (busScan.vendorId == vendorId && busScan.deviceId == deviceId)
		{
			unit--;
			if(unit != -1)
			{
				continue;
			}
			*addrOut = busScan.addr;
			return TRUE;
		}
	}

	return FALSE;
}
