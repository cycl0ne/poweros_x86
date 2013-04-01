#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"

#define SysBase CoreGfxBase->SysBase

#if 0
typedef struct CRastPort {
	PixMap	*crp_Pixmap;
	Layer	*crp_Layer;
	UINT32	crp_Mode;
	UINT32	crp_Dashmask;
	UINT32	crp_Dashcount;
	UINT32	crp_Fillmode;
	BOOL	crp_useBg;
	UINT32	crp_Foreground;
	UINT32	crp_ForegroundRGB;
	UINT32	crp_Background;
	UINT32	crp_BackgroundRGB;
} CRastPort;
#endif

struct RastPort *gfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm)
{
	struct RastPort *rp = AllocVec(sizeof(struct CRastPort), MEMF_CLEAR|MEMF_FAST);
	if (rp)
	{
//		SetMode(rp, ROP_COPY);
//		SetFillMode(rp, FILL_SOLID);
//		SetAPen(rp, RGB(255,255,255));
//		SetBPen(rp, RGB(0,0,0));
//		SetUseBackground(rp, TRUE);
//		SetDash(rp, 0,0);
		//SetStippleBitmap(0,0,0);
	}
	return rp;
}

void gfx_FreeRastPort(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, BOOL flag)
{
	if (flag); //FreeBitmap;
	FreeVec(rp);
}

