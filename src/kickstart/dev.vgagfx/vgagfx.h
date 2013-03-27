#ifndef vgagfx_h
#define vgagfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "expansionbase.h"

#define IOF_QUEUED (1<<4)
#define IOF_CURRENT (1<<5)
#define IOF_SERVICING (1<<6)
#define IOF_DONE (1<<7)

#define DUB_STOPPED (1<<0)

typedef struct VgaGfxBase {
	struct Device	Device;
	struct Unit		Unit;
	SysBase			*SysBase;
	ExpansionBase	*ExpansionBase;
	struct Interrupt	*IS;  // Interrupt Code
} VgaGfxBase;

extern void (*gfxCmdVector[])(struct IOStdReq *, VgaGfxBase *);
extern INT8 gfxCmdQuick[];

void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

#endif
