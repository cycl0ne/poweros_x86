#include "vgagfx.h"
#include "vmware.h"

#define GUEST_OS_OTHER 					0x5000 + 10

void SVGA_SetMode(VgaGfxBase *VgaGfxBase, UINT32 nWidth, UINT32 nHeight, UINT32 nBpp );
void SVGA_Enable(VgaGfxBase *VgaGfxBase);
void SVGA_Disable(VgaGfxBase *VgaGfxBase);
void SVGA_Init(VgaGfxBase *VgaGfxBase);
void SVGA_InitFifo(VgaGfxBase *VgaGfxBase);
void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) ;
void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) ;

#define SysBase			VgaGfxBase->SysBase
#define ExpansionBase	VgaGfxBase->ExpansionBase

UINT32 SVGA_ReadReg(VgaGfxBase *VgaGfxBase, UINT32 index)
{
   WRITE_PORT_ULONG(VgaGfxBase->ioBase + SVGA_INDEX_PORT, index);
   return READ_PORT_ULONG(VgaGfxBase->ioBase + SVGA_VALUE_PORT);
}

void SVGA_WriteReg(VgaGfxBase *VgaGfxBase, UINT32 index, UINT32 value)
{
   WRITE_PORT_ULONG(VgaGfxBase->ioBase + SVGA_INDEX_PORT, index);
   WRITE_PORT_ULONG(VgaGfxBase->ioBase + SVGA_VALUE_PORT, value);
}

//------------------------------------

static void fifoSync( VgaGfxBase *VgaGfxBase ) 
{
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_SYNC, 1 );
	while ( SVGA_ReadReg(VgaGfxBase, SVGA_REG_BUSY ) );
}

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

	SVGA_WriteReg(VgaGfxBase, SVGA_REG_ENABLE, TRUE );
	if ( SVGA_ReadReg(VgaGfxBase, SVGA_REG_ENABLE ) != 1 ) {
		Alert((1<<31), "Enabling Card failed.");
	}

	SVGA_InitFifo(VgaGfxBase);

	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_X, 0 );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_Y, 0 );
	SVGA_WriteReg(VgaGfxBase, SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_HIDE );	
	
DPrintF("FB: %x, Fifo: %x, FifoSz: %x\n", VgaGfxBase->fbMem, VgaGfxBase->fifoMem, VgaGfxBase->fifoSize);
	SVGA_SetMode(VgaGfxBase, 1024, 768, 32);
	
	for (int i = 0; i< 1024*768*4; i++) {
//		VgaGfxBase->fbMem[i] = 0x77;
	}
	SVGA_FillRect(VgaGfxBase, 0x77777777, 0, 0, 500, 380);

//	SVGA_FillRect(VgaGfxBase, 0xFFFFFF00, 0, 381, 1024, 768);
//	SVGA_UpdateRect(VgaGfxBase, 0, 0, 1024, 768);
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
}

void SVGA_FillRect(VgaGfxBase *VgaGfxBase, UINT32 color, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) 
{
	writeFifo(VgaGfxBase, SVGA_CMD_RECT_FILL );
	writeFifo(VgaGfxBase, color );
	writeFifo(VgaGfxBase, x );
	writeFifo(VgaGfxBase, y );
	writeFifo(VgaGfxBase, width );
	writeFifo(VgaGfxBase, height );
}

void SVGA_UpdateRect(VgaGfxBase *VgaGfxBase, UINT32 x, UINT32 y, UINT32 width, UINT32 height ) 
{
	writeFifo(VgaGfxBase, SVGA_CMD_UPDATE );
	writeFifo(VgaGfxBase, x );
	writeFifo(VgaGfxBase, y );
	writeFifo(VgaGfxBase, width );
	writeFifo(VgaGfxBase, height );
}


