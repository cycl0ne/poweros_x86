#include "coregfx.h"
#include "pixmap.h"
#include "memory.h"
#include "exec_funcs.h"

#define SysBase CoreGfxBase->SysBase
BOOL SVGA_SelectSubdriver(PixMap *psd);

static void gen_getscreeninfo(struct CRastPort *rp, pCGfxScreenInfo psi)
{
	PixMap *psd = rp->crp_PixMap;
	psi->rows = psd->yvirtres;
	psi->cols = psd->xvirtres;
	psi->planes = psd->planes;
	psi->bpp = psd->bpp;
	psi->data_format = psd->data_format;
	psi->ncolors = psd->ncolors;
	psi->fonts = 4; //FIXME: NUMBER_FONTS;
	psi->portrait = psd->portrait;
	psi->fbdriver = TRUE;	/* running fb driver, can direct map*/
	psi->pixtype = psd->pixtype;

	switch (psd->data_format) {
	case IF_BGRA8888:
		psi->rmask = RMASKBGRA;
		psi->gmask = GMASKBGRA;
		psi->bmask = BMASKBGRA;
		psi->amask = AMASKBGRA;
		break;
	case IF_RGBA8888:
		psi->rmask = RMASKRGBA;
		psi->gmask = GMASKRGBA;
		psi->bmask = BMASKRGBA;
		psi->amask = AMASKRGBA;
	case IF_BGR888:
		psi->rmask = RMASKBGR;
		psi->gmask = GMASKBGR;
		psi->bmask = BMASKBGR;
		psi->amask = AMASKBGR;
		break;
	case IF_RGB565:
		psi->rmask = RMASK565;
		psi->gmask = GMASK565;
		psi->bmask = BMASK565;
		psi->amask = AMASK565;
		break;
	case IF_RGB555:
		psi->rmask = RMASK555;
		psi->gmask = GMASK555;
		psi->bmask = BMASK555;
		psi->amask = AMASK555;
		break;
	case IF_RGB332:
		psi->rmask = RMASK332;
		psi->gmask = GMASK332;
		psi->bmask = BMASK332;
		psi->amask = AMASK332;
		break;
	case IF_BGR233:
		psi->rmask = RMASK233;
		psi->gmask = GMASK233;
		psi->bmask = BMASK233;
		psi->amask = AMASK233;
		break;
	case PF_PALETTE:
	default:
		psi->amask 	= 0x00;
		psi->rmask 	= 0xff;
		psi->gmask 	= 0xff;
		psi->bmask	= 0xff;
		break;
	}

	//eCos
    //psi->ydpcm = 42; 		/* 320 / (3 * 2.54)*/
    //psi->xdpcm = 38; 		/* 240 / (2.5 * 2.54)*/
	//psp
    //psi->ydpcm = 120;
    //psi->xdpcm = 120;
	if(psd->yvirtres > 480) {	//FIXME update
		/* SVGA 800x600*/
		psi->xdpcm = 33;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 33;	/* assumes screen height of 18 cm*/
	} else if(psd->yvirtres > 350) {
		/* VGA 640x480*/
		psi->xdpcm = 27;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 27;	/* assumes screen height of 18 cm*/
        } else if(psd->yvirtres <= 240) {
		/* half VGA 640x240 */
		psi->xdpcm = 14;        /* assumes screen width of 24 cm*/ 
		psi->ydpcm =  5;
	} else {
		/* EGA 640x350*/
		psi->xdpcm = 27;	/* assumes screen width of 24 cm*/
		psi->ydpcm = 19;	/* assumes screen height of 18 cm*/
	}
}

static int CalcMemGCAlloc(PixMap *psd, int width, int height, int planes, int bpp, unsigned int *psize, unsigned *ppitch)
{
	unsigned int pitch;

	if(!planes)
		planes = psd->planes;
	if(!bpp)
		bpp = psd->bpp;
	/* 
	 * swap width and height in left/right portrait modes,
	 * so imagesize is calculated properly
	 */
	if(psd->portrait & (PORTRAIT_LEFT|PORTRAIT_RIGHT)) {
		int tmp = width;
		width = height;
		height = tmp;
	}

	/* use 4bpp linear for VGA 4 planes memdc format*/
	if(planes == 4)
		bpp = 4;

	/* compute pitch: bytes per line*/
	switch(bpp) {
	case 1:
		pitch = (width+7)/8;
		break;
	case 2:
		pitch = (width+3)/4;
		break;
	case 4:
		pitch = (width+1)/2;
		break;
	case 8:
		pitch = width;
		break;
	case 16:
		pitch = width * 2;
		break;
	case 18:
	case 24:
		pitch = width * 3;
		break;
	case 32:
		pitch = width * 4;
		break;
	default:
		*ppitch = *psize = 0;
		return 0;
	}

	/* right align pitch to DWORD boundary*/
	pitch = (pitch + 3) & ~3;

	*psize = pitch * height;
	*ppitch = pitch;
	return 1;
}

static BOOL mapmemgc(PixMap *mempsd, INT32 w, INT32 h, int planes, int bpp, int data_format, unsigned int pitch, int size, void *addr)
{
	/* pixmaps are always in non-portrait orientation*/
	mempsd->xres = w;
	mempsd->yres = h;		

	mempsd->xvirtres = w;
	mempsd->yvirtres = h;
	mempsd->planes = planes;
	mempsd->bpp = bpp;
	mempsd->data_format = data_format;
	mempsd->pitch = pitch;
	mempsd->size = size;
	mempsd->addr = addr;

	/* select and init hw compatible framebuffer subdriver for pixmap drawing*/

	if (!SVGA_SelectSubdriver(mempsd)) return 0;
	return 1;
}

static inline void
memset(void *dest, UINT8 value, UINT32 size)
{
   asm volatile ("cld; rep stosb" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}


PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 format, UINT32 flags, APTR pixels, int palsize)
{
	PixMap *pmd;
	int 	bpp, planes=1, data_format, pixtype;
	unsigned int size, pitch;
   
	if (width <= 0 || height <= 0) return NULL;

/*
	bpp = rootpsd->bpp;
	data_format = rootpsd->data_format;
	pixtype = rootpsd->pixtype;
	planes = rootpsd->planes;
*/
	switch (format) 
	{
	case 0:			/* default, return framebuffer compatible pixmap*/
		break;
	case 32:		/* match framebuffer format if running 32bpp, else RGBA*/
		if (bpp == 32) break;
		/* else fall through - create RGBA8888 pixmap*/
	case IF_RGBA8888:
		bpp = 32;
		data_format = format;
		pixtype = PF_TRUECOLORABGR;
		break;
	case IF_BGRA8888:
		bpp = 32;
		data_format = format;
		pixtype = PF_TRUECOLOR8888;
		break;
	/*case MWIF_PAL1:*/				/* MWIF_PAL1 is MWIF_MONOBYTEMSB*/
	case IF_MONOBYTEMSB:			/* ft2 non-alias*/
	case IF_MONOBYTELSB:			/* t1lib non-alias*/
	case IF_MONOWORDMSB:			/* core mwcfont, pcf*/
		bpp = 1;
		data_format = format;
		pixtype = PF_PALETTE;
		break;
	case IF_PAL2:
		bpp = 2;
		data_format = format;
		pixtype = PF_PALETTE;
		break;
	case IF_PAL4:
		bpp = 4;
		data_format = format;
		pixtype = PF_PALETTE;
		break;
	case IF_PAL8:
		bpp = 8;
		data_format = format;
		pixtype = PF_PALETTE;
		break;
	case IF_RGB1555:
		bpp = 16;
		data_format = format;
		pixtype = PF_TRUECOLOR1555;
		break;
	case IF_RGB555:
		bpp = 16;
		data_format = format;
		pixtype = PF_TRUECOLOR555;
		break;
	case IF_RGB565:
		bpp = 16;
		data_format = format;
		pixtype = PF_TRUECOLOR565;
		break;
	case IF_RGB888:
		bpp = 24;
		data_format = format;
		pixtype = PF_TRUECOLOR888;
		break;
	default:
		DPrintF("AllocPixmap: unsupported format %08x\n", format);
		return NULL;	/* fail*/
	}

	PixMap	*mempsd;

	pmd = AllocVec(sizeof(PixMap), MEMF_FAST);
	if (!pmd)
	{
		DPrintF("AllocVec on Pixmap failed.\n");
		return NULL;
	}
DPrintF("Got Memory for Pixmap: %x\n",pmd);
	pmd->flags = PSF_MEMORY;			/* reset PSF_SCREEN or PSF_ADDRMALLOC flags*/
	pmd->portrait = PORTRAIT_NONE; /* don't rotate offscreen pixmaps*/
	pmd->addr = NULL;
	//pmd->Update = NULL;				/* no external updates required for mem device*/
	pmd->palette = NULL;				/* don't copy any palette*/
	pmd->palsize = 0;
	pmd->transcolor = NOCOLOR;		/* no transparent colors unless set by image loader*/

	CalcMemGCAlloc(pmd, width, height, planes, bpp, &size, &pitch);
	
	if (!pixels) 
	{
		DPrintF("Allocate Pixels..........");
		pixels = AllocVec(size, MEMF_FAST);
		DPrintF("Ok\n");
//		memset(pixels, 0x0, size);
		if (pixels == NULL) 
		{
			DPrintF("EMERGENCY!! NO MEMORY FOR PIXMAP!! size: %d, %x\n", size, size);
			for(;;);
		}
		pmd->flags |= PSF_ADDRMALLOC;
		DPrintF("Allocated Pixels Addr: %x (size: %d)\n", pixels, size);
	}
	if (!pixels) 
	{
		FreeVec(pmd);
		return NULL;
	}

	if (palsize && (pmd->palette = AllocVec(palsize*sizeof(CgfxPalEntry), MEMF_CLEAR|MEMF_FAST)) == NULL)
	{
		FreeVec(pmd);
		return NULL;		
	}
	pmd->palsize = palsize;
 
	mapmemgc(pmd, width, height, planes, bpp, data_format, pitch, size, pixels);

	pmd->pixtype = pixtype;		/* save pixtype for proper colorval creation*/
	pmd->ncolors = (pmd->bpp >= 24)? (1 << 24): (1 << pmd->bpp);
	pmd->_GetScreenInfo = gen_getscreeninfo;
	pmd->basedata = CoreGfxBase->VgaGfxBase;
	return pmd;
}

#if 0

PSD
GdCreatePixmap(PSD rootpsd, MWCOORD width, MWCOORD height, int format, void *pixels, int palsize)
{



	/* Allocate space for pixel values */
	if (!pixels) {
		pixels = calloc(size, 1);
		pmd->flags |= PSF_ADDRMALLOC;
	}
	if (!pixels) {
err:
		pmd->FreeMemGC(pmd);
		return NULL;
	}
 
	/* allocate palette*/
	if (palsize && (pmd->palette = calloc(palsize*sizeof(MWPALENTRY), 1)) == NULL)
		goto err;
	pmd->palsize = palsize;
 
	pmd->MapMemGC(pmd, width, height, planes, bpp, data_format, pitch, size, pixels);
	pmd->pixtype = pixtype;		/* save pixtype for proper colorval creation*/
	pmd->ncolors = (pmd->bpp >= 24)? (1 << 24): (1 << pmd->bpp);

	return pmd;
}


PixMap *cgfx_AllocPixMap(CoreGfxBase *CoreGfxBase, UINT32 width, UINT32 height, UINT32 bpp, UINT32 flags, APTR pixels)
{
	if (width <= 0 || height <= 0) return NULL;

	PixMap *ret = AllocVec(sizeof(PixMap), MEMF_FAST|MEMF_CLEAR);
	if (ret)
	{
		ret->flags = FPM_AllocVec|FPM_Memory;
		ret->xres = ret->xvirtres = width;
		ret->yres = ret->yvirtres = height;
		ret->bpp  = bpp;
		ret->pitch = width * (bpp>>3);
		ret->memsize = width * height * (bpp>>3);


		if (flags & FPM_Displayable)
		{
			ret->flags |= (FPM_Displayable|FPM_Framebuffer);
			//ret->addr = (void *)0xfd000000; //HACK !!!! oO !!!!
			// Here we should ask the GFXDriver for Memory. 
			// But we will fall through, since we have only a FB Driver
		}
		
		if (pixels)	
		{
			ret->addr = pixels;
		} else 
		{
			ret->flags |= FPM_AllocAddr;
			ret->addr = AllocVec(ret->memsize, MEMF_FAST|MEMF_CLEAR);
		}
		if (!ret->addr) 
		{
			FreeVec(ret);
			ret = NULL;
		}
	}
	return ret;
}

void cgfx_FreePixMap(CoreGfxBase *CoreGfxBase, PixMap *pix)
{
	if (pix == NULL) return;
	if (pix->flags & FPM_AllocAddr)	FreeVec(pix->addr);
	if (pix->flags & FPM_AllocVec)	FreeVec(pix);
}

BOOL BltPixMap(CoreGfxBase *CoreGfxBase, PixMap *dstpix, UINT32 dstx, UINT32 dsty, UINT32 width, UINT32 height, PixMap *srcpix, UINT32 srcx, UINT32 srcy)
{
	if ((srcpix == NULL) || (dstpix == NULL)) return FALSE;
	if (srcpix->bpp != dstpix->bpp) return FALSE;
	
	if (srcx < 0) 
	{
		width += srcx;
		dstx -= srcx;
		srcx = 0;
	}
	if (srcy < 0) 
	{
		height += srcy;
		dsty -= srcy;
		srcy = 0;
	}
	if (srcx + width > srcpix->xvirtres) width = srcpix->xvirtres - srcx;
	if (srcy + height > srcpix->yvirtres) height = srcpix->yvirtres - srcy;
	
	
}
#endif 
