#ifndef PCI_H
#define PCI_H

#include "types.h"

#define PCI_CONF_BAR_IO          0x01
#define PCI_CONF_BAR_64BIT       0x04
#define PCI_CONF_BAR_PREFETCH    0x08

typedef union PCIConfigSpace {
   UINT32 words[16];
   struct {
      UINT16 vendorId;
      UINT16 deviceId;
      UINT16 command;
      UINT16 status;
      UINT16 revisionId;
      UINT8  subclass;
      UINT8  classCode;
      UINT8  cacheLineSize;
      UINT8  latTimer;
      UINT8  headerType;
      UINT8  BIST;
      UINT32 BAR[6];
      UINT32 cardbusCIS;
      UINT16 subsysVendorId;
      UINT16 subsysId;
      UINT32 expansionRomAddr;
      UINT32 reserved0;
      UINT32 reserved1;
      UINT8  intrLine;
      UINT8  intrPin;
      UINT8  minGrant;
      UINT8  maxLatency;
   };
} __attribute__ ((__packed__)) PCIConfigSpace;

typedef struct PCIAddress {
   UINT8 bus, device, function;
} PCIAddress;

typedef struct PCIScanState {
   UINT16     vendorId;
   UINT16     deviceId;
   PCIAddress nextAddr;
   PCIAddress addr;
} PCIScanState;

#endif
