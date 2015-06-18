/*
*/

#ifndef types_h
#define types_h

#include "stdint.h"

typedef uint8_t		UINT8;
typedef uint16_t	UINT16;
typedef uint32_t	UINT32;
typedef uint64_t	UINT64;

typedef int8_t		INT8;
typedef int16_t		INT16;
typedef int32_t		INT32;
typedef int64_t		INT64;

typedef unsigned long size_t;

typedef void				VOID;
typedef void *				APTR;


typedef float				FLOAT;
typedef	double				DOUBLE;

typedef volatile signed char			VINT8;
typedef volatile signed short int		VINT16;
typedef volatile signed int				VINT32;
typedef volatile signed long long int	VINT64;

typedef volatile unsigned char			VUINT8;
typedef volatile unsigned short int		VUINT16;
typedef volatile unsigned int			VUINT32;
typedef volatile unsigned long long int	VUINT64;


typedef UINT32				UINTPTR;

typedef unsigned int		IPTR;
typedef char *				STRPTR;
typedef const char *		CONST_STRPTR;
typedef unsigned int		BOOL;

typedef UINT32	Signal;
typedef INT32	SysCall;


#define SYSOK		1             /**< system call ok                     */
#define	OK			SYSOK		// Change this old calling and then delete it!
#define SYSNULL		0
#define SYSERR		(-1)           /**< system call failed                 */

#define NULL				0
#define TRUE				1
#define FALSE				0

#define ABS(a)				((a)>0?(a):-(a))
#define MIN(a,b)			((a)<(b)?(a):(b))
#define MAX(a,b)			((a)>(b)?(a):(b))
#define CLAMP(min,x,max)	((x)>(max)?(max):((x)<(min)?(min):(x)))

#define SET_BITS(data, mask)	((data)|=(mask))
#define CLEAR_BITS(data, mask)	((data)&=(~(mask)))
#define CHANGE_BITS(data, mask)	((data)^=(mask))
#define TEST_BITS(data, mask)	(((data)&(mask)) != 0)
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define DMB_IMPL(name) { UINT32 name = 0; __asm__ __volatile__ ("mcr	p15,0,%[t],c7,c10,5\n" :: [t] "r" (name) : "memory"); }
#define DSB_IMPL(name) { UINT32 name = 0; __asm__ __volatile__ ("mcr	p15,0,%[t],c7,c10,4\n" :: [t] "r" (name) : "memory"); }
#define DMB DMB_IMPL(MACRO_CONCAT(__dmb_, __COUNTER__))
#define DSB DSB_IMPL(MACRO_CONCAT(__dsb_, __COUNTER__))

#define READ32(addr) (*(VUINT32*)(addr))
#define READ16(addr) (*(VUINT16*)(addr))
#define READ8(addr) (*(VUINT8*)(addr))

#define WRITE32(addr,value) (*(VUINT32*)(addr) = (VUINT32)(value))
#define WRITE16(addr,value) (*(VUINT16*)(addr) = (VUINT16)(value))
#define WRITE8(addr,value) (*(VUINT8*)(addr) = (VUINT8)(value))

#define FADDR(fptr)  ((APTR) (fptr))

/*
typedef struct JumpVec
{
	VUINT32	vec;
}JumpVec, *pJumpVec;

#define _GET_VEC(v) ((APTR)(*(UINT32*)(v)))
#define _GETJUMPVEC(lib,n)  ((struct JumpVec *)(((UINT8 *)lib)-((n)*4)))
#define _GETVECADDR(lib,n)  (_GET_VEC(_GETJUMPVEC(lib,n)))
*/

// Modcall is for Platforms with different ABI
#define _MODCALL
#define _LIBCALL(lib, n, ret, proto, para) (*(((_MODCALL ret (**) proto)(lib))[-n]))para

#define __GNUC_VA_LIST
typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)

#ifdef __GNUC__
  #define offsetof( type, member ) __builtin_offsetof( type, member )
#else
  #error "OFFSETOF is not internal."
#endif


#endif
