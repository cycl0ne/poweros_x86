#ifndef vgagfx_h
#define vgagfx_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "expansionbase.h"
#include "resident.h"

#include "svga_reg.h"
#include "pci.h"
#include "exec_funcs.h"
#include "expansion_funcs.h"

typedef struct VgaGfxBase {
	struct Library	Library;
	SysBase			*SysBase;
	ExpansionBase	*ExpansionBase;
	PCIAddress		pciAddr;
	UINT32			ioBase;
	UINT32			*fifoMem;
	UINT8			*fbMem;
	UINT32     		fifoSize;
	UINT32     		fbSize;
	UINT32			vramSize;

	UINT32			deviceVersionId;
	UINT32			capabilities;

	UINT32			width;
	UINT32			height;
	UINT32			bpp;
	UINT32			pitch;
} VgaGfxBase;


#endif
