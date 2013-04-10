#include "vgagfx.h"

APTR vglib_OpenLib(VgaGfxBase *VgaGfxBase)
{
	VgaGfxBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(VgaGfxBase);
}

APTR vglib_CloseLib(VgaGfxBase *VgaGfxBase)
{
	VgaGfxBase->Library.lib_OpenCnt--;
	return NULL;
}

APTR vglib_ExpungeLib(VgaGfxBase *VgaGfxBase)
{
	return NULL;
}

APTR vglib_ExtFuncLib(VgaGfxBase *VgaGfxBase)
{
	return NULL;
}
