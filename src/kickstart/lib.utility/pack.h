#ifndef pack_h
#define pack_h

#define	PSTB_SIGNED 31
#define PSTB_UNPACK 30	  /* Note that these are active low... */
#define PSTB_PACK   29	  /* Note that these are active low... */
#define	PSTB_EXISTS 26    /* Tag exists bit true flag hack...  */

#define	PSTF_SIGNED (1L << PSTB_SIGNED)
#define PSTF_UNPACK (1L << PSTB_UNPACK)
#define PSTF_PACK   (1L << PSTB_PACK)

#define	PSTF_EXISTS (1L << PSTB_EXISTS)


/*****************************************************************************/


#define PKCTRL_PACKUNPACK 0x00000000
#define PKCTRL_PACKONLY   0x40000000
#define PKCTRL_UNPACKONLY 0x20000000

#define PKCTRL_BYTE       0x80000000
#define PKCTRL_WORD       0x88000000
#define PKCTRL_LONG       0x90000000

#define PKCTRL_UBYTE      0x00000000
#define PKCTRL_UWORD      0x08000000
#define PKCTRL_ULONG      0x10000000

#define PKCTRL_BIT        0x18000000
#define PKCTRL_FLIPBIT    0x98000000

#define PK_BITNUM1(flg) ((flg) == 0x01 ? 0 : (flg) == 0x02 ? 1 : (flg) == 0x04 ? 2 : (flg) == 0x08 ? 3 : (flg) == 0x10 ? 4 : (flg) == 0x20 ? 5 : (flg) == 0x40 ? 6 : 7)
#define PK_BITNUM2(flg) ((flg < 0x100 ? PK_BITNUM1(flg) : 8+PK_BITNUM1(flg >> 8)))
#define PK_BITNUM(flg) ((flg < 0x10000 ? PK_BITNUM2(flg) : 16+PK_BITNUM2(flg >> 16)))
#define PK_WORDOFFSET(flg) ((flg) < 0x100 ? 1 : 0)
#define PK_LONGOFFSET(flg) ((flg) < 0x100  ? 3 : (flg) < 0x10000 ? 2 : (flg) < 0x1000000 ? 1 : 0)
#define PK_CALCOFFSET(type,field) ((UINT32)(&((struct type *)0)->field))


#define PACK_STARTTABLE(tagbase)                           (tagbase)
#define PACK_NEWOFFSET(tagbase)                            (-1L),(tagbase)
#define PACK_ENDTABLE                                      0
#define PACK_ENTRY(tagbase,tag,type,field,control)         (control | ((tag-tagbase) << 16L) | PK_CALCOFFSET(type,field))
#define PACK_BYTEBIT(tagbase,tag,type,field,control,flags) (control | ((tag-tagbase) << 16L) | PK_CALCOFFSET(type,field) | (PK_BITNUM(flags) << 13L))
#define PACK_WORDBIT(tagbase,tag,type,field,control,flags) (control | ((tag-tagbase) << 16L) | (PK_CALCOFFSET(type,field)+PK_WORDOFFSET(flags)) | ((PK_BITNUM(flags)&7) << 13L))
#define PACK_LONGBIT(tagbase,tag,type,field,control,flags) (control | ((tag-tagbase) << 16L) | (PK_CALCOFFSET(type,field)+PK_LONGOFFSET(flags)) | ((PK_BITNUM(flags)&7) << 13L))

#endif
