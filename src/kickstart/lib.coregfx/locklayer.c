#include "coregfx.h"
#include "layers.h"
#include "exec_funcs.h"

#define SysBase CoreGfxBase->SysBase

void cgfx_UnlockLayer(CoreGfxBase *CoreGfxBase, struct Layer *l)
{
	ReleaseSemaphore(&l->Lock);
}

void cgfx_LockLayer(CoreGfxBase *CoreGfxBase, struct Layer *l)
{
	ObtainSemaphore(&l->Lock);
}
