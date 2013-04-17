#include "types.h"

#define ULONG UINT32
#define BYTE	INT8
#define UCHAR	UINT8
#define USHORT	UINT16
#define	DWORD	INT32
#define PULONG	UINT32*
#define PVOID	APTR

static inline void memcpy(void *dest, const void *src, UINT32  size)
{
   asm volatile ("cld; rep movsb" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static __inline__ void WRITE_PORT_ULONG(UINT16 port, UINT32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static __inline__  UINT32 READ_PORT_ULONG(UINT16 port)
{
   UINT32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA    0xcfc
#define PCI_IO  1
#define PCI_MEM 2
#define SVGA_INDEX_PORT      0x0
#define SVGA_VALUE_PORT      0x1
#define VMWARE_VENDOR_ID 0x15AD
#define SVGA_VERSION_2     2
#define SVGA_ID_2          SVGA_MAKE_ID(SVGA_VERSION_2)
#define SVGA_VERSION_1     1
#define SVGA_ID_1          SVGA_MAKE_ID(SVGA_VERSION_1)
#define SVGA_MAGIC         0x900000
#define SVGA_MAKE_ID(ver)  (SVGA_MAGIC << 8 | (ver))
#define SVGA_VERSION_0     0
#define SVGA_ID_0          SVGA_MAKE_ID(SVGA_VERSION_0)
enum {
   SVGA_REG_ID = 0,
   SVGA_REG_ENABLE = 1,
   SVGA_REG_WIDTH = 2,
   SVGA_REG_HEIGHT = 3,
   SVGA_REG_MAX_WIDTH = 4,
   SVGA_REG_MAX_HEIGHT = 5,
   SVGA_REG_DEPTH = 6,
   SVGA_REG_BITS_PER_PIXEL = 7,
   SVGA_REG_PSEUDOCOLOR = 8,
   SVGA_REG_RED_MASK = 9,
   SVGA_REG_GREEN_MASK = 10,
   SVGA_REG_BLUE_MASK = 11,
   SVGA_REG_BYTES_PER_LINE = 12,
   SVGA_REG_FB_START = 13,
   SVGA_REG_FB_OFFSET = 14,
   SVGA_REG_VRAM_SIZE = 15,
   SVGA_REG_FB_SIZE = 16,

   SVGA_REG_CAPABILITIES = 17,
   SVGA_REG_MEM_START = 18,
   SVGA_REG_MEM_SIZE = 19,
   SVGA_REG_CONFIG_DONE = 20,
   SVGA_REG_SYNC = 21,
   SVGA_REG_BUSY = 22,
   SVGA_REG_GUEST_ID = 23,
   SVGA_REG_CURSOR_ID = 24,
   SVGA_REG_CURSOR_X = 25,
   SVGA_REG_CURSOR_Y = 26,
   SVGA_REG_CURSOR_ON = 27,
   SVGA_REG_HOST_BITS_PER_PIXEL = 28,
   SVGA_REG_TOP = 30,
   SVGA_PALETTE_BASE = 1024
};
#define PCI_SPACE_MEMORY 0
#define PCI_SPACE_IO     1
#define    SVGA_CMD_INVALID_CMD         0
#define    SVGA_CMD_UPDATE         1
#define    SVGA_CMD_RECT_FILL         2
#define    SVGA_CMD_RECT_COPY         3
#define    SVGA_CMD_DEFINE_BITMAP         4
#define    SVGA_CMD_DEFINE_BITMAP_SCANLINE   5
#define    SVGA_CMD_DEFINE_PIXMAP         6
#define    SVGA_CMD_DEFINE_PIXMAP_SCANLINE   7
#define    SVGA_CMD_RECT_BITMAP_FILL      8
#define    SVGA_CMD_RECT_PIXMAP_FILL      9
#define    SVGA_CMD_RECT_BITMAP_COPY     10
#define    SVGA_CMD_RECT_PIXMAP_COPY     11
#define    SVGA_CMD_FREE_OBJECT        12
#define    SVGA_CMD_RECT_ROP_FILL           13
#define    SVGA_CMD_RECT_ROP_COPY           14
#define    SVGA_CMD_RECT_ROP_BITMAP_FILL    15
#define    SVGA_CMD_RECT_ROP_PIXMAP_FILL    16
#define    SVGA_CMD_RECT_ROP_BITMAP_COPY    17
#define    SVGA_CMD_RECT_ROP_PIXMAP_COPY    18
#define   SVGA_CMD_DEFINE_CURSOR        19
#define   SVGA_CMD_DISPLAY_CURSOR        20
#define   SVGA_CMD_MOVE_CURSOR        21
#define SVGA_CMD_DEFINE_ALPHA_CURSOR      22
#define SVGA_CMD_DRAW_GLYPH               23
#define SVGA_CMD_DRAW_GLYPH_CLIPPED       24
#define   SVGA_CMD_UPDATE_VERBOSE             25
#define SVGA_CMD_SURFACE_FILL             26
#define SVGA_CMD_SURFACE_COPY             27
#define SVGA_CMD_SURFACE_ALPHA_BLEND      28
#define   SVGA_CMD_MAX           29
#define SVGA_CMD_MAX_ARGS                 12
#define    SVGA_FIFO_MIN         0
#define    SVGA_FIFO_MAX         1
#define    SVGA_FIFO_NEXT_CMD   2
#define    SVGA_FIFO_STOP         3

typedef struct PCI_BASE
{
   BYTE  Space;
   BYTE  Type;
   BYTE  Prefetch;
   DWORD Address;
}PCI_BASE, *PPCI_BASE;

typedef struct PCI_DEVICE_INFO
{
   USHORT DeviceId;
   USHORT VendorId;
   int      ClassCode;
   int      SubClass;
   int    Bus;
   int    Device;
   int    Function;
   PCI_BASE Bases[6];
}PCI_DEVICE_INFO, *PPCI_DEVICE_INFO;

#pragma pack(push, 1)

typedef struct PCI_CONFIG
{
  UCHAR Reg    : 8;
  UCHAR Func   : 3;
  UCHAR Dev    : 5;
  UCHAR Bus    : 8;
  UCHAR Rsvd   : 7;
  UCHAR Enable : 1;
}PCI_CONFIG, *PPCI_CONFIG;

#pragma pack(pop)

ULONG PciReadConfig(ULONG bus, ULONG device,
               ULONG function, ULONG reg)
{
   PCI_CONFIG c;
   PULONG n;
   USHORT base;
   c.Enable = 1;
   c.Rsvd = 0;
   c.Bus = bus;
   c.Dev = device;
   c.Func = function;
   c.Reg = reg & 0xfc;
   n = (PULONG)((PVOID)&c);
   WRITE_PORT_ULONG(0xcf8, *n);
   base = 0xcfc + (reg & 0x03);
   return READ_PORT_ULONG(base);
}


VOID PciWriteConfig(ULONG bus, ULONG device,
                 ULONG function, ULONG reg,
                 ULONG value)
{
   PCI_CONFIG c;
   PULONG n;
   USHORT base;
   c.Enable = 1;
   c.Rsvd = 0;
   c.Bus = bus;
   c.Dev = device;
   c.Func = function;
   c.Reg = reg & 0xfc;
   n = (PULONG)((PVOID)&c);
   WRITE_PORT_ULONG(0xcf8, *n);
   base = 0xcfc + (reg & 0x03);
   WRITE_PORT_ULONG(base, value);
}

VOID PciGetDeviceInfo(PPCI_DEVICE_INFO info,
                 ULONG bus,
                 ULONG device,
                 ULONG function)
{
   ULONG r;

   r = PciReadConfig(bus, device, function, 0);
   info->VendorId = r & 0xffff;
   info->DeviceId = (r >> 16) & 0xffff;
   r = PciReadConfig(bus, device, function, 8);
   info->ClassCode = r >> 24;
   info->SubClass  = (r >> 16) & 0xff;
   info->Bus = bus;
   info->Device = device;
   info->Function = function;
}

VOID PciReadBases(PPCI_DEVICE_INFO Info)
{
   int i;
   ULONG l;

   for (i = 0; i < 6; i++) {
      l = PciReadConfig(Info->Bus, Info->Device, Info->Function, 0x10 + i*4);
      if (l & 0x01) {
         Info->Bases[i].Space    = PCI_SPACE_IO;
         Info->Bases[i].Type     = 0;
         Info->Bases[i].Prefetch = 0;
         Info->Bases[i].Address  = l & (~0x03);
      }
      else {
         Info->Bases[i].Space    = PCI_SPACE_MEMORY;
         Info->Bases[i].Type     = (l >> 1)&0x03;
         Info->Bases[i].Prefetch = (l >> 3)&0x01;
         Info->Bases[i].Address  = l & (~0x0f);
      }
   }
}


BOOL PciFindDeviceByVendorId(USHORT VendorId,
                      int index,
                      PPCI_DEVICE_INFO Info)
{
   PCI_DEVICE_INFO info;
   int bus;
   int fun;
   int dev;

   for (bus = 0; bus < 256; bus++) {
      for (dev = 0; dev < 32; dev++) {
         for (fun = 0; fun < 8; fun++) {
            PciGetDeviceInfo(&info, bus, dev, fun);
            if (info.VendorId == VendorId) {
               if (!index) {
                  Info->Bus = bus;
                  Info->Device = dev;
                  Info->Function = fun;
                  Info->DeviceId = info.DeviceId;
                  Info->VendorId = info.VendorId;
                  Info->ClassCode = info.ClassCode;
                  Info->SubClass = info.SubClass;
                  return TRUE;
               }
               else
                  index--;
            }
         }
      }
   }
   return FALSE;
}

USHORT VmwSvgaIndex;
USHORT VmwSvgaValue;
PULONG  VmwFramebuffer;
PULONG  VmwFifo;

BOOL DetectVmwareVideoAdapter(PPCI_DEVICE_INFO Info)
{
   PCI_DEVICE_INFO dev;
   int index = 0;

   while (1) {
      if (!PciFindDeviceByVendorId(VMWARE_VENDOR_ID, index, &dev))
         break;
      else {
         if (dev.DeviceId == 0x405) {
            if (Info != NULL)
               memcpy(Info, &dev, sizeof(PCI_DEVICE_INFO));
            return TRUE;
         }
         index++;
      }
   }
   return FALSE;
}

VOID VmwSvgaOut(USHORT Index, ULONG Value)
{
   WRITE_PORT_ULONG(VmwSvgaIndex, Index);
   WRITE_PORT_ULONG(VmwSvgaValue, Value);
}

ULONG VmwSvgaIn(USHORT Index)
{
   WRITE_PORT_ULONG(VmwSvgaIndex, Index);
   return READ_PORT_ULONG(VmwSvgaValue);
}
#include "exec_funcs.h"

BOOL VmwSetVideoMode(ULONG Width, ULONG Height, ULONG Bpp, APTR SysBase)
{
	PCI_DEVICE_INFO dev;
	if (DetectVmwareVideoAdapter(&dev))
   {
      //ULONG fb;
      PciReadBases(&dev);
      VmwSvgaIndex = dev.Bases[0].Address + SVGA_INDEX_PORT;
      VmwSvgaValue = dev.Bases[0].Address + SVGA_VALUE_PORT;

	DPrintF("%x, %x\n", VmwSvgaIndex,VmwSvgaValue);
      
      VmwSvgaOut(SVGA_REG_ID, SVGA_ID_2);
      if (VmwSvgaIn(SVGA_REG_ID) !=  SVGA_ID_2)
      {
         VmwSvgaOut(SVGA_REG_ID, SVGA_ID_1);
         if (VmwSvgaIn(SVGA_REG_ID) != SVGA_ID_1)
         {
            VmwSvgaOut(SVGA_REG_ID, SVGA_ID_0);
            if (VmwSvgaIn(SVGA_REG_ID) != SVGA_ID_0)
            {
               return FALSE;
            }
         }
      }

      VmwFramebuffer = (PULONG)VmwSvgaIn(SVGA_REG_FB_START);

      VmwSvgaIn(SVGA_REG_FB_SIZE);
      VmwFifo = (PULONG)VmwSvgaIn(SVGA_REG_MEM_START);
      VmwSvgaIn(SVGA_REG_MEM_SIZE);

	DPrintF("%x, %x, %x\n", VmwFramebuffer,VmwFifo, VmwSvgaIn(SVGA_REG_ID));


      VmwSvgaOut(SVGA_REG_WIDTH, Width);
      VmwSvgaOut(SVGA_REG_HEIGHT, Height);
      VmwSvgaOut(SVGA_REG_BITS_PER_PIXEL, Bpp);

      /*
       * Read additional informations.
       */
      VmwSvgaIn(SVGA_REG_FB_OFFSET);
      VmwSvgaIn(SVGA_REG_BYTES_PER_LINE);
      VmwSvgaIn(SVGA_REG_DEPTH);
      VmwSvgaIn(SVGA_REG_PSEUDOCOLOR);
      VmwSvgaIn(SVGA_REG_RED_MASK);
      VmwSvgaIn(SVGA_REG_GREEN_MASK);
      VmwSvgaIn(SVGA_REG_BLUE_MASK);


      VmwSvgaOut(SVGA_REG_ENABLE, 1);


      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

