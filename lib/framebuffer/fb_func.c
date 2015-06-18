/**
* File: /fb_func．c
* User: cycl0ne
* Date: 2014-11-20
* Time: 10:14 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "framebuffer.h"
#include "expansion_interface.h"

#define SysBase FBBase->fb_SysBase

void setSubDriver(pSD psd, pSubDriver subdriver);
pSubDriver selectSubDriver(pSD psd);

/*
* OpenFD
* CloseFD
* GetScreenInfo
* SetPalette
*/


static INT32 calcPitch(INT32 bpp, INT32 width)
{
	INT32 pitch;

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
		return 0;
	}

	/* right align pitch to DWORD boundary*/
	pitch = (pitch + 3) & ~3;
	return pitch;
}

static INT32 setDataFormat(pSD psd)
{
	int data_format = 0;

	switch(psd->pixtype) 
	{
		case PF_TRUECOLOR8888:
			data_format = IF_BGRA8888;
			break;
		case PF_TRUECOLORABGR:
			data_format = IF_RGBA8888;
			break;
		case PF_TRUECOLOR888:
			data_format = IF_BGR888;
			break;
		case PF_TRUECOLOR565:
			data_format = IF_RGB565;
			break;
		case PF_TRUECOLOR555:
			data_format = IF_RGB555;
			break;
		case PF_TRUECOLOR1555:
				data_format = IF_RGB1555;
			break;
		case PF_TRUECOLOR332:
			data_format = IF_RGB332;
			break;
		case PF_TRUECOLOR233:
			data_format = IF_BGR233;
			break;
		case PF_PALETTE:
			switch (psd->bpp) 
			{
			case 8:
				data_format = IF_PAL8;
				break;
			case 4:
				data_format = IF_PAL4;
				break;
			case 2:
				data_format = IF_PAL2;
				break;
			case 1:
				data_format = IF_PAL1;
				break;
			}
			break;
	}

	return data_format;
}

static void outports(unsigned short _port, unsigned short _data) {
	asm volatile ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

static unsigned short inports(unsigned short _port) {
	unsigned short rv;
	asm volatile ("inw %1, %0" : "=a" (rv) : "dN" (_port));
	return rv;
}

#define PREFERRED_VY 4096

static BOOL openfb(pFBBase FBBase, uint32_t resolution_x, uint32_t resolution_y, uint32_t bpp)
{

	APTR ExpansionBase = OpenLibrary("expansion.library", 0);
	if (ExpansionBase == NULL)
	{
		KPrintF("Error in loading expansion\n");
		return 0;
	}
	outports(0x1CE, 0x00);
	UINT16 i = inports(0x1CF);
	if (i < 0xB0C0 || i > 0xB0C6) 
	{
		KPrintF("Error in Framebuffer.library [i]\n", i);
		CloseLibrary(ExpansionBase);
		return 0;
	}
	outports(0x1CF, 0xB0C4);
	i = inports(0x1CF);
	/* Disable VBE */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x00);
	/* Set X resolution to 1024 */
	outports(0x1CE, 0x01);
	outports(0x1CF, resolution_x);
	/* Set Y resolution to 768 */
	outports(0x1CE, 0x02);
	outports(0x1CF, resolution_y);
	/* Set bpp to 32 */
	outports(0x1CE, 0x03);
	outports(0x1CF, bpp);
	/* Set Virtual Height to stuff */
	outports(0x1CE, 0x07);
	outports(0x1CF, PREFERRED_VY);
	/* Re-enable VBE */
	outports(0x1CE, 0x04);
	outports(0x1CF, 0x41);
	
//	UINT8 *buffer = NULL;
//	PCIAddress addr;
	
	if (!PCIFindDevice(0x1234, 0x1111, &FBBase->fb_PCIAddr))
	{
		outports(0x1CE, 0x04);
		outports(0x1CF, 0x00);
		KPrintF("Error: %x\n", FBBase->fb_PCIAddr);
		CloseLibrary(ExpansionBase);
		return 0;
	}
	
	FBBase->fb_Buffer = (UINT8*)PCIGetBARAddr(&FBBase->fb_PCIAddr, 0);
	FBBase->fb_Resolution_X = resolution_x;
	FBBase->fb_Resolution_Y = resolution_y;
	FBBase->fb_Resolution_B = bpp;
//MemSet(FBBase->fb_Buffer, 0xff, 1024*768*4);	
	//KPrintF("Buffer: %x", buffer);
	CloseLibrary(ExpansionBase);
	return 1;
}

pSD	fb_Open(pFBBase FBBase, int32_t w, int32_t h, int32_t bpp)
{
	INT32		type, visual;
	pSubDriver	subdriver;
	KPrintF("fb_Open\n");

	if (! openfb(FBBase, w, h, bpp)) return NULL;
	
	pSD			psd = AllocVec(sizeof(PixMap_t), MEMF_FAST|MEMF_CLEAR);
	
	if (psd)
	{
		KPrintF("Psd alloced\n");
		psd->portrait	= PORTRAIT_NONE;
		psd->xres		= psd->xvirtres = FBBase->fb_Resolution_X;
		psd->yres		= psd->yvirtres = FBBase->fb_Resolution_Y;
		psd->bpp		= FBBase->fb_Resolution_B;
		psd->ncolors	= (psd->bpp >= 24)? (1<<24): (1<<psd->bpp);
		psd->pitch		= calcPitch(psd->bpp, psd->xres);
		psd->size		= psd->yres * psd->pitch;
		psd->flags		= PMF_SCREEN;
	
		switch (psd->bpp)
		{
			case 16:
				//if (fb_var.green.length == 5)
				//	psd->pixtype = PF_TRUECOLOR555;
				//else
					psd->pixtype = PF_TRUECOLOR565;
				//break;
			case 18:
			case 24:
				psd->pixtype = PF_TRUECOLOR888;
				break;
			case 32:
				psd->pixtype = PF_TRUECOLOR8888;
				//psd->pixtype = PF_TRUECOLORABGR;
				break;
			default:
				psd->pixtype = PF_PALETTE;
		}
		psd->data_format	= setDataFormat(psd);
		subdriver			= selectSubDriver(psd);

		setSubDriver(psd, subdriver);
		psd->addr			= FBBase->fb_Buffer;
	}
	return psd;
}

void fb_Close(pFBBase FBBase, pSD psd)
{
	if (psd==NULL) return;
	if (!(psd->flags&PMF_SCREEN)) return;
//	FreeV
}

pSD fb_AllocateMemGC(pFBBase FBBase, pSD psd)
{
	pSD	mempsd;


	mempsd = AllocVec(sizeof(PixMap_t), MEMF_FAST|MEMF_CLEAR);
	if (!mempsd) return NULL;

	CopyMem(psd, mempsd, sizeof(PixMap_t));

	/* initialize*/
	mempsd->flags = PMF_MEMORY;			/* reset PSF_SCREEN or PSF_ADDRMALLOC flags*/
	mempsd->portrait = PORTRAIT_NONE; /* don't rotate offscreen pixmaps*/
	mempsd->addr = NULL;
	mempsd->Update = NULL;				/* no external updates required for mem device*/
	mempsd->palette = NULL;				/* don't copy any palette*/
	mempsd->palsize = 0;
	mempsd->transcolor = NOCOLOR;		/* no transparent colors unless set by image loader*/

	return mempsd;
}

static void SetPortraitSubdriver(pSD psd)
{
	pSubDriver	subdriver;
	
	switch (psd->portrait) 
	{
	case PORTRAIT_NONE:
	default:
		subdriver = psd->orgsubdriver;
		break;
	case PORTRAIT_LEFT:
		subdriver = psd->left_subdriver;
		break;
	case PORTRAIT_RIGHT:
		subdriver = psd->right_subdriver;
		break;
	case PORTRAIT_DOWN:
		subdriver = psd->down_subdriver;
		break;
	}
	setSubDriver(psd, subdriver);
}


BOOL fb_MapMemGC(pFBBase FBBase, pSD mempsd, int32_t w, int32_t h, int32_t planes, 
								int32_t bpp, int32_t data_format, uint32_t pitch,
								int32_t size, void *addr)
{
	pSubDriver	subdriver;
	
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
	subdriver = selectSubDriver(mempsd);
	if (!subdriver) return FALSE;
	SetPortraitSubdriver(mempsd);
	return 1;
}

void fb_FreeMemGC(pFBBase FBBase, pSD psd)
{
	if (!(psd->flags & PMF_MEMORY)) return;
	if (psd->addr && (psd->flags & PMF_ADDRMALLOC)) FreeVec(psd->addr);
	if (psd->palette) FreeVec(psd->palette);
	FreeVec(psd);
}

#if 0
typedef struct ScreenDevice {
	INT32		xres;
	INT32		yres;
	INT32		xvirtres;
	INT32		yvirtres;
	INT32		planes;
	INT32		bpp;
	INT32		linelen;
	INT32		size;
	INT32		ncolors;
	INT32		pixtype;
	INT32		flags;
	void*		addr;

	pCoreFont	builtin_fonts;
	INT32		portrait;
	pSubDriver	orgsubdriver;
	
}ScreenDevice_t, *pSD;
#endif

