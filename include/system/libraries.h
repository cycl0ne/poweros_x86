#ifndef libraries_h
#define libraries_h

#include "types.h"
#include "lists.h"

typedef struct Library
{
	Node	lib_Node;
	UINT16	lib_OpenCnt;
	UINT16	lib_Flags;
	UINT16	lib_NegSize;
	UINT16	lib_PosSize;
	UINT16	lib_Version;
	UINT16	lib_Revision;
	UINT32	lib_Sum;
	STRPTR	lib_IDString;
} Library, Library_t, *pLibrary;

#define LIB_OPEN    1
#define LIB_CLOSE   2
#define LIB_EXPUNGE 3
#define LIB_EXTFUNC 4 /* for future expansion */

#define LIBF_OK       0
#define LIBF_SUMMING  1
#define LIBF_CHANGED  2
#define LIBF_SUMTAMP  4
#define LIBF_BROKE    4
#define LIBF_DELEXP   8
#define LIBF_SUMUSED 16

#endif