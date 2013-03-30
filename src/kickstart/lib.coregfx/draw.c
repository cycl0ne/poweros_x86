#include "coregfx.h"
#include "rastport.h"
#include "layers.h"

void UnlockLayer(Layer *l)
{
}

void LockLayer(Layer *l)
{
}
#if 0
UINT32 cgfx_DrawPoint(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, UINT32 x, UINT32 y)
{
	Layer *l = rp->crp_Layer;

	if (l)
	{
		ClipRect *cr;
		LockLayer(l);
		if (cr = l->ClipRect)
		{			
			x += l->bounds.MinX;
			y += l->bounds.MinY;
			x -= l->Scroll_X;
			y -= l->Scroll_Y;

			for(cr; cr!= NULL; cr = cr->Next)
			{
				if (cr->bounds.MinX <= x && cr->bounds.MaxX >=x 
					&& cr->bounds.MinY <= y && cr->bounds.MaxX >=y)
				{
					if (!cr->lobs)
					{
						// Onscreen
						SVGA_DrawPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
						UnlockLayer(l);
						return 0;
					} else
					{
						// Offscreen
						if (!cr->BitMap) 
						{
							// Simple handling, nothing to do.
							UnlockLayer(l);
							return -1;
						}
						// DrawOffscreen (smart refresh)
						//GfxBase->Driver->DrawPixel(GfxBase->Driver, cr->BitMap, x, y, rp->FgPen);
						MEM_DrawPixel(cr->BitMap, x, y, rp->crp_Foreground, rp->crp_Mode);
						UnlockLayer(l);
						return 0;
					}
				}
			}
			PixMap *super;
			if (super = l->SuperBitMap)
			{
				x += l->Scroll_X;
				y += l->Scroll_Y;
				x -= l->bounds.MinX;
				y -= l->bounds.MinY;
				for(cr = l->SuperClipRect; cr != NULL; cr = cr->Next)
				{
					if (cr->bounds.MinX <= x && cr->bounds.MaxX >=x 
						&& cr->bounds.MinY <= y && cr->bounds.MaxX >=y)
					{
						//DrawPixel(super, x, y, color);
						//GfxBase->Driver->DrawPixel(GfxBase->Driver, super, x, y, rp->FgPen);
						SVGA_DrawPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
						UnlockLayer(l);
						return 0;
					}	
				}
			}
		}
		UnlockLayer(l);
		return -1;
	} else
	{
		SVGA_DrawPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
		//GfxBase->Driver->DrawPixel(GfxBase->Driver, rp->BitMap, x, y, rp->FgPen);
	}
	//FixCursor(rp);
	return 0;
}
#endif 
