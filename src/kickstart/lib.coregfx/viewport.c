#include "coregfx.h"
#include "view.h"

#define SysBase CoreGfxBase->SysBase

APTR SVGA_SetDisplayMode(APTR VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp);
struct CRastPort *gfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm);

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
		vp->RastPort = gfx_InitRastPort(CoreGfxBase, pix);
		vp->PixMap	= pix;
		vp->DWidth	= pix->xres;
		vp->DHeight = pix->yres;
		vp->DxOffset= xOffset;
		vp->DyOffset= yOffset;
		//DPrintF("RP: %x ----\n", vp->RastPort);
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

//PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 bpp, UINT32 flags, APTR pixels);;
PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 format, UINT32 flags, APTR pixels, int palsize);
struct CRastPort *gfx_InitRastPort(CoreGfxBase *CoreGfxBase, struct PixMap *bm);
void cgfx_Line(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint) ;
void cgfx_Point(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y);
void cgfx_FillRect(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 width, INT32 height);

#include "cursor.h"

void __TestView(CoreGfxBase *CoreGfxBase)
{
	struct PixMap *pix	= cgfx_AllocPixMap(CoreGfxBase, 640, 480, IF_BGRA8888, FPM_Displayable, NULL,0 );
	struct CRastPort *rp = gfx_InitRastPort(CoreGfxBase, pix);
	DPrintF("cgfx_AllocPixMap() = %x\n", pix->addr);
	if (pix) memset32(pix->addr, 0x0, pix->size/4);

	DPrintF("cgfx_CreateView()\n");
	struct View *view	= cgfx_CreateView(CoreGfxBase, 640, 480, 32);
	struct ViewPort *vp = cgfx_CreateVPort(CoreGfxBase, pix, 0, 0);
	cgfx_MakeVPort(CoreGfxBase, view, vp);
	DPrintF("LoadView()\n");
	cgfx_LoadView(CoreGfxBase, view);

//	for (int i=0; i<0x10000000; i++);
	
//	struct PixMap *pix2	= cgfx_AllocPixMap(CoreGfxBase, 640, 480, 32, FPM_Displayable, NULL,0);
//	DPrintF("cgfx_AllocPixMap() = %x\n", pix2->addr);
//	if (pix2) memset32(pix2->addr, 0xFFFF0000, pix->size/4);

//	DPrintF("cgfx_CreateView()\n");
//	struct View *view2	= cgfx_CreateView(CoreGfxBase, 640, 480, 32);
//	struct ViewPort *vp2 = cgfx_CreateVPort(CoreGfxBase, pix2, 0, 0);
//	cgfx_MakeVPort(CoreGfxBase, view2, vp2);
//	DPrintF("LoadView()\n");
//	cgfx_LoadView(CoreGfxBase, view2);

//	for (int i=0; i<0x10000000; i++);
//	cgfx_LoadView(CoreGfxBase, view);
//	for (int i=0; i<0x10000000; i++);
//	cgfx_LoadView(CoreGfxBase, view2);
//	for (int i=0; i<0x10000000; i++);
//	cgfx_LoadView(CoreGfxBase, view);
//	for (int i=0; i<0x10000000; i++);
//	cgfx_LoadView(CoreGfxBase, view2);	
//	for (int i=0; i<0x10000000; i++);
//	cgfx_LoadView(CoreGfxBase, NULL);	
//void cgfx_Line(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 y1, INT32 x2, INT32 y2, BOOL bDrawLastPoint) 
	SetForegroundColor(rp, RGB(255, 0, 0));
	cgfx_Point(CoreGfxBase, rp, 100, 100);
	cgfx_Line(CoreGfxBase, rp, 10, 10, 200, 200, TRUE);

	for (int i = 10; i< 640; i+=10) 
	{
		cgfx_Line(CoreGfxBase, rp, i, 10, 200, 200, TRUE);
	}

	SetForegroundColor(rp, RGB(0, 255, 0));
	for (int i = 10; i< 480; i+=5) 
		cgfx_Line(CoreGfxBase, rp, 10, i, 200, 200, TRUE);

	SetForegroundColor(rp, RGB(0, 0, 255));
	for (int i = 10; i< 640; i+=10) 
		cgfx_Line(CoreGfxBase, rp, 10, 480, i, 200, TRUE);

	SetForegroundColor(rp, RGB(255, 255, 255));
	SetMode(rp, ROP_XOR);
	cgfx_FillRect(CoreGfxBase, rp, 50, 50, 100, 100);
	SetMode(rp, ROP_OR);
	cgfx_FillRect(CoreGfxBase, rp, 100, 100, 100, 100);
	SetMode(rp, ROP_INVERT);
	cgfx_FillRect(CoreGfxBase, rp, 150, 150, 100, 100);
//	MoveCursor(0,0);
//DPrintF("Setcursor\n");
//	SetCursor(&arrow);
//	ShowCursor(rp);
}

/*
void cgfx_Point(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)

Original:
* Bitmap allokiert, in rasinfo kopiert, viewport/view gebaut und loadview gemacht. = bitmap wird angezeigt.

Heute:
* PixMap allokiert, in CreateVPort gehängt, MakeVPort aufgerufen, LoadView ->hier muessen wir bei einem fb tricksen
* 
 
 */

