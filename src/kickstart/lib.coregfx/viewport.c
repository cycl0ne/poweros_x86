#include "coregfx.h"
#include "view.h"

#define SysBase CoreGfxBase->SysBase

APTR SVGA_SetDisplayMode(APTR VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp);

static inline void memset32(void *dest, UINT32 value, UINT32 size)
{
   asm volatile ("cld; rep stosl" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

struct ViewPort *cgfx_CreateVPort(CoreGfxBase *CoreGfxBase, PixMap *pix, INT32 xOffset, INT32 yOffset)
{
	if (!pix) return NULL;
	struct ViewPort *vp = AllocVec(sizeof(struct View), MEMF_FAST);
	if (vp)
	{
		vp->Next	= NULL;
		vp->PixMap	= pix;
		vp->DWidth	= pix->xres;
		vp->DHeight = pix->yres;
		vp->DxOffset= xOffset;
		vp->DyOffset= yOffset;
	}
}

struct View *cgfx_CreateView(CoreGfxBase *CoreGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 bpp)
{
	struct View *view = AllocVec(sizeof(struct View), MEMF_FAST);
	if (view)
	{
		view->vp	= NULL;
		view->width = nWidth;
		view->height= nHeight;
		view->bpp	= bpp;
		view->fbAddr= NULL;
		view->fbSize= (nWidth * nHeight * (bpp>>3))>>2; // divide 4 for faster memory copy
	}
	return view;
}

UINT32 cgfx_MakeVPort(CoreGfxBase *CoreGfxBase, struct View *view, struct ViewPort *vp)
{
	view->vp = vp;
	return 0;
}

static inline void memcpy32(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsl" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

void cgfx_LoadView(CoreGfxBase *CoreGfxBase, struct View *view)
{
	if (view)
	{
		if (CoreGfxBase->ActiveView)
		{
			struct View 	*aview= (struct View *)CoreGfxBase->ActiveView;
			struct ViewPort *avp  = aview->vp;
			struct PixMap 	*apix = avp->PixMap;

			// We have the Screen open, so copy FB to the Old ViewPort
			// TODO: check for other ViewPorts connected to this VIEW.
			
			// Restore old Bitmap
			apix->addr = avp->oldPMAddr;
			// Copy FB content to Bitmap
			memcpy32(apix->addr, aview->fbAddr, aview->fbSize);
			// Clear old Framebuffer View
			memset32(aview->fbAddr, 0x00, aview->fbSize);			
		}
		// Activate new View
		DPrintF("SVGA_SetDisplayMode(%d, %d, %d)\n", view->width, view->height, view->bpp);
		view->fbAddr = SVGA_SetDisplayMode(CoreGfxBase->VgaGfxBase, view->width, view->height, view->bpp);
		// Copy new View to FB
		memcpy32(view->fbAddr, view->vp->PixMap->addr, view->fbSize);
		// Save old addr of Pixmap
		view->vp->oldPMAddr = view->vp->PixMap->addr;
		view->vp->PixMap->addr = view->fbAddr;
		// Store the new View
		CoreGfxBase->ActiveView = view;
	} else
	{
		DPrintF("TODO: Shutdown View\n");
		struct View 	*aview= (struct View *)CoreGfxBase->ActiveView;
		struct ViewPort *avp  = aview->vp;
		struct PixMap 	*apix = avp->PixMap;

		// We have the Screen open, so copy FB to the Old ViewPort
		// TODO: check for other ViewPorts connected to this VIEW.
		
		// Restore old Bitmap
		apix->addr = avp->oldPMAddr;
		// Copy FB content to Bitmap
		memcpy32(apix->addr, aview->fbAddr, aview->fbSize);
		// Clear old Framebuffer View
		DPrintF("Shutdown View: Clear Screen %x/%x\n", aview->fbAddr, aview->fbSize);
		memset32(aview->fbAddr, 0x00000000, aview->fbSize);
		CoreGfxBase->ActiveView = NULL;
	}
}

PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 bpp, UINT32 flags, APTR pixels);

void __TestView(CoreGfxBase *CoreGfxBase)
{
	struct PixMap *pix	= cgfx_AllocPixMap(CoreGfxBase, 640, 480, 32, FPM_Displayable, NULL);
	DPrintF("cgfx_AllocPixMap() = %x\n", pix->addr);
	if (pix) memset32(pix->addr, 0xFFFFFFFF, pix->memsize/4);

	DPrintF("cgfx_CreateView()\n");
	struct View *view	= cgfx_CreateView(CoreGfxBase, 640, 480, 32);
	struct ViewPort *vp = cgfx_CreateVPort(CoreGfxBase, pix, 0, 0);
	cgfx_MakeVPort(CoreGfxBase, view, vp);
	DPrintF("LoadView()\n");
	cgfx_LoadView(CoreGfxBase, view);

	for (int i=0; i<0x10000000; i++);
	
	struct PixMap *pix2	= cgfx_AllocPixMap(CoreGfxBase, 640, 480, 32, FPM_Displayable, NULL);
	DPrintF("cgfx_AllocPixMap() = %x\n", pix2->addr);
	if (pix2) memset32(pix2->addr, 0xFFFF0000, pix->memsize/4);

	DPrintF("cgfx_CreateView()\n");
	struct View *view2	= cgfx_CreateView(CoreGfxBase, 640, 480, 32);
	struct ViewPort *vp2 = cgfx_CreateVPort(CoreGfxBase, pix2, 0, 0);
	cgfx_MakeVPort(CoreGfxBase, view2, vp2);
	DPrintF("LoadView()\n");
	cgfx_LoadView(CoreGfxBase, view2);

	for (int i=0; i<0x10000000; i++);
	cgfx_LoadView(CoreGfxBase, view);
	for (int i=0; i<0x10000000; i++);
	cgfx_LoadView(CoreGfxBase, view2);
	for (int i=0; i<0x10000000; i++);
	cgfx_LoadView(CoreGfxBase, view);
	for (int i=0; i<0x10000000; i++);
	cgfx_LoadView(CoreGfxBase, view2);	
	for (int i=0; i<0x10000000; i++);
	cgfx_LoadView(CoreGfxBase, NULL);	
}

/*

Original:
* Bitmap allokiert, in rasinfo kopiert, viewport/view gebaut und loadview gemacht. = bitmap wird angezeigt.

Heute:
* PixMap allokiert, in CreateVPort gehängt, MakeVPort aufgerufen, LoadView ->hier muessen wir bei einem fb tricksen
* 
 
 */

