#include "vgagfx.h"
#include "asm.h"
#include "vmware.h"

#define GUEST_OS_OTHER 					0x5000 + 10

void SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp );
void SVGA_Enable(VgaGfxBase *VgaGfxBase);
void SVGA_Disable(VgaGfxBase *VgaGfxBase);
void SVGA_Init(VgaGfxBase *VgaGfxBase);
void SVGA_InitFifo(VgaGfxBase *VgaGfxBase);
void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) ;
void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, INT32 x, INT32 y, INT32 width, INT32 height ) ;

#define SysBase			VgaGfxBase->SysBase
#define ExpansionBase	VgaGfxBase->ExpansionBase

UINT32 SVGA_ReadReg(VgaGfxBase *VgaGfxBase, UINT32 index)
{
   outl(VgaGfxBase->ioBase + SVGA_INDEX_PORT, index);
   return inl(VgaGfxBase->ioBase + SVGA_VALUE_PORT);
}

void SVGA_WriteReg(VgaGfxBase *VgaGfxBase, UINT32 index, UINT32 value)
{
   outl(VgaGfxBase->ioBase + SVGA_INDEX_PORT, index);
   outl(VgaGfxBase->ioBase + SVGA_VALUE_PORT, value);
}

//------------------------------------

static void fifoSync( VgaGfxBase *VgaGfxBase ) 
{
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_SYNC, 1 );
	while ( SVGA_ReadReg(VgaGfxBase, SVGA_REG_BUSY ) );
}

void writeFifo(VgaGfxBase *VgaGfxBase, UINT32 value)
{
    UINT32 *tmp = VgaGfxBase->fifoMem;
	/* Need to sync? */
	if ((tmp[SVGA_FIFO_NEXT_CMD] + sizeof(UINT32) == tmp[SVGA_FIFO_STOP])
		|| (tmp[SVGA_FIFO_NEXT_CMD] == tmp[SVGA_FIFO_MAX] - sizeof(UINT32) &&
			tmp[SVGA_FIFO_STOP] == tmp[SVGA_FIFO_MIN]))
	{
		fifoSync(VgaGfxBase);
	}

	tmp[tmp[SVGA_FIFO_NEXT_CMD] / sizeof(UINT32)] = value;
	if(tmp[SVGA_FIFO_NEXT_CMD] == tmp[SVGA_FIFO_MAX] - sizeof(UINT32))
	{
		tmp[SVGA_FIFO_NEXT_CMD] = tmp[SVGA_FIFO_MIN];
	}
	else
	{
		tmp[SVGA_FIFO_NEXT_CMD] += sizeof(UINT32);
	}
}


#if 0
static void writeFifo(VgaGfxBase *VgaGfxBase, UINT32 value ) 
{
	if (
		( VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] + sizeof( UINT32 ) == VgaGfxBase->fifoMem[ SVGA_FIFO_STOP ] ) ||
		( VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] == VgaGfxBase->fifoMem[ SVGA_FIFO_MAX ] - sizeof( UINT32 ) &&
		VgaGfxBase->fifoMem[ SVGA_FIFO_STOP ] == VgaGfxBase->fifoMem[ SVGA_FIFO_MIN ] ) ) {
		fifoSync(VgaGfxBase);
	}

	VgaGfxBase->fifoMem[ VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] / sizeof( UINT32 ) ] = value;
	if ( VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] == VgaGfxBase->fifoMem[ SVGA_FIFO_MAX ] - sizeof( UINT32 ) ) {
		VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] = VgaGfxBase->fifoMem[ SVGA_FIFO_MIN ];
	} else {
		VgaGfxBase->fifoMem[ SVGA_FIFO_NEXT_CMD ] += sizeof( UINT32 );
	}
}
#endif
//------------------------------------

/*
inline BOOL SVGA_HasFIFOCap(VgaGfxBase *VgaGfxBase, UINT32 cap)
{
	return (VgaGfxBase->fifoMem[SVGA_FIFO_CAPABILITIES] & cap) != 0;
}
*/

inline BOOL SVGA_IsFIFORegValid(VgaGfxBase *VgaGfxBase, UINT32 reg)
{
	return VgaGfxBase->fifoMem[SVGA_FIFO_MIN] > (reg << 2);
}

void SVGA_Init(VgaGfxBase *VgaGfxBase)
{
	PCISetMemEnable(&VgaGfxBase->pciAddr, TRUE);
	VgaGfxBase->ioBase 	= 			PCIGetBARAddr(&VgaGfxBase->pciAddr, 0);
	VgaGfxBase->fbMem		= (void*)	PCIGetBARAddr(&VgaGfxBase->pciAddr, 1);
	VgaGfxBase->fifoMem	= (void*)	PCIGetBARAddr(&VgaGfxBase->pciAddr, 2);
	//VgaGfxBase->fifoMem		= (UINT32*)	SVGA_ReadReg(VgaGfxBase, SVGA_REG_MEM_START );
	VgaGfxBase->deviceVersionId = SVGA_ID_2;
	do {
		SVGA_WriteReg(VgaGfxBase, SVGA_REG_ID, VgaGfxBase->deviceVersionId);
		if (SVGA_ReadReg(VgaGfxBase, SVGA_REG_ID) == VgaGfxBase->deviceVersionId) {
			break;
		} else {
			VgaGfxBase->deviceVersionId--;
		}
	} while (VgaGfxBase->deviceVersionId >= SVGA_ID_0);

	if (VgaGfxBase->deviceVersionId < SVGA_ID_0) {
		Alert((1<<31), "Error negotiating SVGA device version.");
	}

	VgaGfxBase->vramSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_VRAM_SIZE);
	VgaGfxBase->fbSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_FB_SIZE);
	VgaGfxBase->fifoSize = SVGA_ReadReg(VgaGfxBase, SVGA_REG_MEM_SIZE);

	if (VgaGfxBase->deviceVersionId >= SVGA_ID_1) {
		VgaGfxBase->capabilities = SVGA_ReadReg(VgaGfxBase, SVGA_REG_CAPABILITIES);
	}
		
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_GUEST_ID, GUEST_OS_OTHER );
	if (SVGA_ReadReg(VgaGfxBase, SVGA_REG_GUEST_ID) != GUEST_OS_OTHER) {
		Alert((1<<31), "Guest OS Failed.");
	}

	SVGA_SetMode(VgaGfxBase, 640, 480, 32);

	SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE );
	if ( SVGA_ReadReg(VgaGfxBase, SVGA_REG_ENABLE ) != 1 ) {
		Alert((1<<31), "Enabling Card failed.");
	}

	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_X, 0 );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_Y, 0 );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_HIDE );	

//DPrintF("hbpp: %d\n", SVGA_ReadReg(VgaGfxBase, SVGA_REG_HOST_BITS_PER_PIXEL ));
//DPrintF("ConfDone: %x, RegDone: %x\n", SVGA_ReadReg(VgaGfxBase, SVGA_REG_CONFIG_DONE ),SVGA_ReadReg(VgaGfxBase, SVGA_REG_ENABLE ));
//DPrintF("FB: %x, Fifo: %x, FifoSz: %x\n", VgaGfxBase->fbMem, VgaGfxBase->fifoMem, VgaGfxBase->fifoSize);

//	SVGA_FillRect(VgaGfxBase, 0xFFFFFF00, 0, 381, 1024, 768);
//	SVGA_UpdateRect(VgaGfxBase, 0, 0, 640, 480);
//	SVGA_WaitForFB(VgaGfxBase);
//	SVGA_SetMode(VgaGfxBase, 800, 600, 32);
//	SVGA_InitFifo(VgaGfxBase);
//	SVGA_FillRect(VgaGfxBase, 0x00FF0000ul, 0, 0, 640, 480);
}

void SVGA_Disable(VgaGfxBase *VgaGfxBase)
{
   SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, FALSE);
}

/*
 * 	sVmWare.pnFifoBase[ SVGA_FIFO_MIN ] = 4 * sizeof( uint32 );
	sVmWare.pnFifoBase[ SVGA_FIFO_MAX ] = sVmWare.nFifoSize & ~3;
	sVmWare.pnFifoBase[ SVGA_FIFO_NEXT_CMD ] = 4 * sizeof( uint32 );
	sVmWare.pnFifoBase[ SVGA_FIFO_STOP ] = 4 * sizeof( uint32 );
*/

void SVGA_InitFifo(VgaGfxBase *VgaGfxBase)
{
	VgaGfxBase->fifoMem[SVGA_FIFO_MIN]		= 4 * sizeof( UINT32 ); //SVGA_FIFO_NUM_REGS * sizeof(UINT32);
	VgaGfxBase->fifoMem[SVGA_FIFO_MAX]		= VgaGfxBase->fifoSize & ~3;
	VgaGfxBase->fifoMem[SVGA_FIFO_NEXT_CMD]	= 4 * sizeof( UINT32 ); //VgaGfxBase->fifoMem[SVGA_FIFO_MIN];
	VgaGfxBase->fifoMem[SVGA_FIFO_STOP] 	= 4 * sizeof( UINT32 ); //VgaGfxBase->fifoMem[SVGA_FIFO_MIN];

	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CONFIG_DONE, TRUE);
	SVGA_ReadReg(VgaGfxBase, SVGA_REG_CONFIG_DONE);

	//vmwareWriteReg( SVGA_REG_CONFIG_DONE, 1 );
	//vmwareReadReg( SVGA_REG_CONFIG_DONE );
	//SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE);
}

void SVGA_Enable(VgaGfxBase *VgaGfxBase)
{
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE);
}

void SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp ) 
{
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, FALSE );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_WIDTH, nWidth );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_HEIGHT, nHeight );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_BITS_PER_PIXEL, nBpp );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_GUEST_ID, GUEST_OS_OTHER );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE );
#if 0
	GFXCDBase->m_regEnable = vmwareReadReg(SVGA_REG_ENABLE, GFXCDBase);
	GFXCDBase->m_regWidth = vmwareReadReg(SVGA_REG_WIDTH, GFXCDBase);
	GFXCDBase->m_regHeight = vmwareReadReg(SVGA_REG_HEIGHT, GFXCDBase);
	GFXCDBase->m_regBitsPerPixel = vmwareReadReg(SVGA_REG_BITS_PER_PIXEL, GFXCDBase);
	GFXCDBase->m_regDepth = vmwareReadReg(SVGA_REG_DEPTH, GFXCDBase);
	GFXCDBase->m_regFbOffset = vmwareReadReg(SVGA_REG_FB_OFFSET, GFXCDBase);
	GFXCDBase->m_regFbSize = vmwareReadReg(SVGA_REG_FB_SIZE, GFXCDBase);
	GFXCDBase->m_regBytesPerLine = vmwareReadReg(SVGA_REG_BYTES_PER_LINE, GFXCDBase);
	GFXCDBase->m_regPseudoColor = vmwareReadReg(SVGA_REG_PSEUDOCOLOR, GFXCDBase);
	GFXCDBase->m_regRedMask = vmwareReadReg(SVGA_REG_RED_MASK, GFXCDBase);
	GFXCDBase->m_regGreenMask = vmwareReadReg(SVGA_REG_GREEN_MASK, GFXCDBase);
	GFXCDBase->m_regBlueMask = vmwareReadReg(SVGA_REG_BLUE_MASK, GFXCDBase);
	UINT32			width;
	UINT32			height;
	UINT32			bpp;
	UINT32			pitch;
#endif

	VgaGfxBase->width = SVGA_ReadReg(VgaGfxBase, SVGA_REG_WIDTH);
	VgaGfxBase->height = SVGA_ReadReg(VgaGfxBase, SVGA_REG_HEIGHT);
	VgaGfxBase->bpp = SVGA_ReadReg(VgaGfxBase, SVGA_REG_DEPTH);
	VgaGfxBase->pitch = SVGA_ReadReg(VgaGfxBase, SVGA_REG_BYTES_PER_LINE); // Hand: nWidth*(nBpp/8);//
	VgaGfxBase->offset = SVGA_ReadReg(VgaGfxBase, SVGA_REG_FB_OFFSET);
//	DPrintF("[VGFX]SetMode %d %d\n",SVGA_ReadReg(VgaGfxBase, SVGA_REG_BYTES_PER_LINE),VgaGfxBase->pitch);
	SVGA_InitFifo(VgaGfxBase);
}

void SVGA_WaitForFB(VgaGfxBase *VgaGfxBase)
{
	fifoSync(VgaGfxBase);
}


void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) 
{
	writeFifo(VgaGfxBase, SVGA_CMD_RECT_FILL );
	writeFifo(VgaGfxBase, color );
	writeFifo(VgaGfxBase, x );
	writeFifo(VgaGfxBase, y );
	writeFifo(VgaGfxBase, width );
	writeFifo(VgaGfxBase, height );
//	DPrintF("[VGFX]FillRect Col: %x\n", color);
}

void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, INT32 x, INT32 y, INT32 width, INT32 height ) 
{
	writeFifo(VgaGfxBase, SVGA_CMD_UPDATE );
	writeFifo(VgaGfxBase, x );
	writeFifo(VgaGfxBase, y );
	writeFifo(VgaGfxBase, width );
	writeFifo(VgaGfxBase, height );
}


