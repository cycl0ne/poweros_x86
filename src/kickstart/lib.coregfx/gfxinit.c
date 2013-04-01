#include "coregfx.h"
#include "vgagfx.h"
#include "pixmap.h"


#define LIBRARY_VERSION_STRING "\0$VER: coregfx.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "coregfx.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static CoreGfxBase *cgfx_Init(CoreGfxBase *CoreGfxBase, UINT32 *segList, APTR SysBase);
APTR cgfx_OpenLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_CloseLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_ExpungeLib(CoreGfxBase *CoreGfxBase);
APTR cgfx_ExtFuncLib(CoreGfxBase *CoreGfxBase);

void SVGA_Init(CoreGfxBase *CoreGfxBase);

static volatile APTR FuncTab[] = 
{
	(void(*)) cgfx_OpenLib,
	(void(*)) cgfx_CloseLib,
	(void(*)) cgfx_ExpungeLib,
	(void(*)) cgfx_ExtFuncLib,

	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(CoreGfxBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)cgfx_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK,
	LIBRARY_VERSION,
	NT_LIBRARY,
	90,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

void SVGA_DrawPixel32(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 c, UINT32 rop);
//CoreGfxBase->DrawMemoryPixel(super, x, y, rp->crp_Foreground, rp->crp_Mode);

void DrawMemoryPixel32(PixMap *dst, UINT32 x, UINT32 y, UINT32 c, UINT32 mode)
{
	register unsigned char *addr = dst->addr + y * dst->pitch + (x << 2);
	*((UINT32*)addr) = c;
}

void __TestView(CoreGfxBase *CoreGfxBase);

static CoreGfxBase *cgfx_Init(CoreGfxBase *CoreGfxBase, UINT32 *segList, APTR SysBase)
{
	CoreGfxBase->Library.lib_OpenCnt = 0;
	CoreGfxBase->Library.lib_Node.ln_Pri = -100;
	CoreGfxBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	CoreGfxBase->Library.lib_Node.ln_Name = (STRPTR)name;
	CoreGfxBase->Library.lib_Version = LIBRARY_VERSION;
	CoreGfxBase->Library.lib_Revision = LIBRARY_REVISION;
	CoreGfxBase->Library.lib_IDString = (STRPTR)&version[7];	
	CoreGfxBase->SysBase	= SysBase;

	CoreGfxBase->VgaGfxBase = OpenLibrary("vgagfx.library", 0);
	if (!CoreGfxBase->VgaGfxBase) DPrintF("Failed to open vgagfx.library\n");
	CoreGfxBase->DrawScreenPixel = (void(*)())SVGA_DrawPixel32;
	CoreGfxBase->DrawMemoryPixel = (void(*)())DrawMemoryPixel32;
	CoreGfxBase->ActiveView = NULL; // Initialize to NULL (No View Opened)
__TestView(CoreGfxBase);
	return CoreGfxBase;
}

static const char EndResident = 0;
