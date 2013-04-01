#include "coregfx.h"
#include "rastport.h"
#include "layers.h"

void UnlockLayer(Layer *l){};
void LockLayer(Layer *l){};

static void DrawPixel(PixMap *pix, UINT32 x, UINT32 y, UINT32 col, UINT32 mode)
{
	switch (pix->bpp)
	{
		case 32:
		{
			register unsigned char *addr = pix->addr + y * pix->pitch + (x << 2);
			*((UINT32*)addr) = col;
			break;
		}
		default:
		break;
	}
}

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
						//CoreGfxBase->DrawScreenPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
						DrawPixel(rp->crp_PixMap, x, y, rp->crp_Foreground, rp->crp_Mode);
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
						//CoreGfxBase->DrawMemoryPixel(cr->BitMap, x, y, rp->crp_Foreground, rp->crp_Mode);
						DrawPixel(cr->BitMap, x, y, rp->crp_Foreground, rp->crp_Mode);
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
						//GfxBase->Driver->DrawPixel(GfxBase->Driver, super, x, y, rp->FgPen);
						DrawPixel(super, x, y, rp->crp_Foreground, rp->crp_Mode);
						//CoreGfxBase->DrawMemoryPixel(super, x, y, rp->crp_Foreground, rp->crp_Mode);
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
		DrawPixel(rp->crp_PixMap, x, y, rp->crp_Foreground, rp->crp_Mode);
		//CoreGfxBase->DrawScreenPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
		//SVGA_DrawPixel(CoreGfxBase->VgaGfxBase, x, y, rp->crp_Foreground, rp->crp_Mode);
	}
	//FixCursor(rp);
	return 0;
}
 
