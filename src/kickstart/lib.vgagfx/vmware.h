#include "asm.h"
static inline void memset(void *dest, UINT8 value, UINT32 size)
{
   asm volatile ("cld; rep stosb" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static inline UINT32 ReadReg(VgaGfxBase *VgaGfxBase, UINT32 index)
{
	outl(VgaGfxBase->indexPort, index);
	return inl(VgaGfxBase->valuePort);
}

static inline void WriteReg(VgaGfxBase *VgaGfxBase, UINT32 index, UINT32 value)
{
	outl(VgaGfxBase->indexPort, index);
	outl(VgaGfxBase->valuePort, value);
}

#define offsetof(type, member)  ((UINT32)(&((type*)NULL)->member))

//#define SVGA_REG_VRAM_SIZE 				15
//#define SVGA_REG_HOST_BITS_PER_PIXEL	28
//#define GUEST_OS_OTHER 					0x5000 + 10
//#define SVGA_CURSOR_ON_HIDE				0x0

void SVGA_FifoInit(VgaGfxBase *VgaGfxBase);
void SVGA_FifoStart(VgaGfxBase *VgaGfxBase);
void SVGA_FifoStop(VgaGfxBase *VgaGfxBase);
void SVGA_FifoSync(VgaGfxBase *VgaGfxBase);
void SVGA_FifoBeginWrite(VgaGfxBase *VgaGfxBase);
void SVGA_FifoWrite(VgaGfxBase *VgaGfxBase, UINT32 value);
void SVGA_FifoEndWrite(VgaGfxBase *VgaGfxBase);
void SVGA_FifoUpdateFullscreen(VgaGfxBase *VgaGfxBase);
