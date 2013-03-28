#ifndef vgagfx_h
#define vgagfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "expansionbase.h"

#include "svga_reg.h"
#include "pci.h"

#define IOF_QUEUED (1<<4)
#define IOF_CURRENT (1<<5)
#define IOF_SERVICING (1<<6)
#define IOF_DONE (1<<7)

#define DUB_STOPPED (1<<0)

#define IS_PRIORITY		0


struct GfxUnit {
	struct Unit	unit;
	PCIAddress pciAddr;
	UINT32     ioBase;
	UINT32    *fifoMem;
	UINT8     *fbMem;
	UINT32     fifoSize;
	UINT32     fbSize;
	UINT32     vramSize;

	UINT32     deviceVersionId;
	UINT32     capabilities;

	UINT32     width;
	UINT32     height;
	UINT32     bpp;
	UINT32     pitch;

	struct {
	  UINT32  reservedSize;
	  BOOL    usingBounceBuffer;
	  UINT8   bounceBuffer[1024 * 1024];
	  UINT32  nextFence;
	} fifo;
#if 0
	volatile struct {
	  UINT32        pending;
	  UINT32        switchContext;
	  IntrContext   oldContext;
	  IntrContext   newContext;
	  UINT32        count;
	} irq;
#endif
};

typedef struct VgaGfxBase {
	struct Device	Device;
	struct GfxUnit	Unit;
	SysBase			*SysBase;
	ExpansionBase	*ExpansionBase;
	struct Interrupt	*IS;  // Interrupt Code
} VgaGfxBase;

extern void (*gfxCmdVector[])(struct IOStdReq *, VgaGfxBase *);
extern INT8 gfxCmdQuick[];

void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

#endif
