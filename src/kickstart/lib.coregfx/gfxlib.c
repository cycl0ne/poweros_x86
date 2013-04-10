#include "coregfx.h"

APTR cgfx_OpenLib(CoreGfxBase *CoreGfxBase)
{
	CoreGfxBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(CoreGfxBase);
}

APTR cgfx_CloseLib(CoreGfxBase *CoreGfxBase)
{
	CoreGfxBase->Library.lib_OpenCnt--;
	return CoreGfxBase;
}

APTR cgfx_ExpungeLib(CoreGfxBase *CoreGfxBase)
{
	return CoreGfxBase;
}

APTR cgfx_ExtFuncLib(CoreGfxBase *CoreGfxBase)
{
	return CoreGfxBase;
}
