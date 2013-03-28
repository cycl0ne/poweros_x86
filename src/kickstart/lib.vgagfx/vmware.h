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

#define offsetof(type, member)  ((UINT32)(&((type*)NULL)->member))

#define SVGA_REG_VRAM_SIZE 				15
#define SVGA_REG_HOST_BITS_PER_PIXEL	28
#define GUEST_OS_OTHER 					0x5000 + 10
#define SVGA_CURSOR_ON_HIDE				0x0

/*
 *  Macros to compute variable length items (sizes in 32-bit words)
 */

#define SVGA_PIXMAP_SIZE(w,h,d) ((( ((w)*(d))+31 ) >> 5) * (h))
#define SVGA_PIXMAP_SCANLINE_SIZE(w,d) (( ((w)*(d))+31 ) >> 5)

/*
 *  Increment from one scanline to the next of a bitmap or pixmap
 */
#define SVGA_PIXMAP_INCREMENT(w,d) ((( ((w)*(d))+31 ) >> 5) * sizeof (uint32))

/*
 *  Commands in the command FIFO
 */

#define         SVGA_CMD_UPDATE                   1
        /* FIFO layout:
           X, Y, Width, Height */

#define         SVGA_CMD_RECT_FILL                2
        /* FIFO layout:
           Color, X, Y, Width, Height */

#define         SVGA_CMD_RECT_COPY                3
        /* FIFO layout:
           Source X, Source Y, Dest X, Dest Y, Width, Height */

#define         SVGA_CMD_DEFINE_BITMAP            4
        /* FIFO layout:
           Pixmap ID, Width, Height, <scanlines> */

#define         SVGA_CMD_DEFINE_BITMAP_SCANLINE   5
        /* FIFO layout:
           Pixmap ID, Width, Height, Line #, scanline */

#define         SVGA_CMD_DEFINE_PIXMAP            6
        /* FIFO layout:
           Pixmap ID, Width, Height, Depth, <scanlines> */

#define         SVGA_CMD_DEFINE_PIXMAP_SCANLINE   7
        /* FIFO layout:
           Pixmap ID, Width, Height, Depth, Line #, scanline */

#define         SVGA_CMD_RECT_BITMAP_FILL         8
        /* FIFO layout:
           Bitmap ID, X, Y, Width, Height, Foreground, Background */

#define         SVGA_CMD_RECT_PIXMAP_FILL         9
        /* FIFO layout:
           Pixmap ID, X, Y, Width, Height */

#define         SVGA_CMD_RECT_BITMAP_COPY        10
        /* FIFO layout:
           Bitmap ID, Source X, Source Y, Dest X, Dest Y,
           Width, Height, Foreground, Background */

#define         SVGA_CMD_RECT_PIXMAP_COPY        11
        /* FIFO layout:
           Pixmap ID, Source X, Source Y, Dest X, Dest Y, Width, Height */

#define         SVGA_CMD_FREE_OBJECT             12
        /* FIFO layout:
           Object (pixmap, bitmap, ...) ID */

#define         SVGA_CMD_RECT_ROP_FILL           13
         /* FIFO layout:
            Color, X, Y, Width, Height, ROP */

#define         SVGA_CMD_RECT_ROP_COPY           14
         /* FIFO layout:
            Source X, Source Y, Dest X, Dest Y, Width, Height, ROP */

#define         SVGA_CMD_RECT_ROP_BITMAP_FILL    15
         /* FIFO layout:
            ID, X, Y, Width, Height, Foreground, Background, ROP */

#define         SVGA_CMD_RECT_ROP_PIXMAP_FILL    16
         /* FIFO layout:
            ID, X, Y, Width, Height, ROP */

#define         SVGA_CMD_RECT_ROP_BITMAP_COPY    17
         /* FIFO layout:
            ID, Source X, Source Y,
            Dest X, Dest Y, Width, Height, Foreground, Background, ROP */

#define         SVGA_CMD_RECT_ROP_PIXMAP_COPY    18
         /* FIFO layout:
            ID, Source X, Source Y, Dest X, Dest Y, Width, Height, ROP */

#define        SVGA_CMD_DEFINE_CURSOR            19
       /* FIFO layout:
          ID, Hotspot X, Hotspot Y, Width, Height,
          Depth for AND mask, Depth for XOR mask,
          <scanlines for AND mask>, <scanlines for XOR mask> */

#define        SVGA_CMD_DISPLAY_CURSOR           20
       /* FIFO layout:
          ID, On/Off (1 or 0) */

#define        SVGA_CMD_MOVE_CURSOR              21
       /* FIFO layout:
          X, Y */
