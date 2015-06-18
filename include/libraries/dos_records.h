#ifndef DOS_RECORD_H
#define DOS_RECORD_H

#include "dos_io.h"

struct RecordLock {
	pFileHandle	rec_FH;		/* filehandle */
	UINT32	rec_Offset;	/* offset in file */
	UINT32	rec_Length;	/* length of file to be locked */
	UINT32	rec_Mode;	/* Type of lock */
};

//
//
#define REC_EXCLUSIVE		0
#define REC_EXCLUSIVE_IMMED	1
#define REC_SHARED		2
#define REC_SHARED_IMMED	3

#endif
