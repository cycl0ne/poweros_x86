/**
 * @file cli_init.c
 *
 * This is the call, that has to be done by the boot shell! Dont call it again!
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

static void * memmove( void * s1, const void * s2, UINT32 n )
{
    char * dest = (char *) s1;
    const char * src = (const char *) s2;
    if ( dest <= src )
    {
        while ( n-- )
            *dest++ = *src++;
    }
    else
    {
        src += n;
        dest += n;
        while ( n-- )
            *--dest = *--src;
    }
    return s1;
}

#define DOSLIB_BUFSIZ 1024
#define DOSLIB_UNGETCBUFSIZE 1

#define EOF -1
#define DOSLIB_FREAD     8u
#define DOSLIB_FWRITE   16u
#define DOSLIB_FAPPEND  32u 
#define DOSLIB_FRW      64u
#define DOSLIB_FBIN    128u

/* Internal flags, made to fit the same status field as the flags above. */
/* -------------------------------------------------------------------------- */
/* free() the buffer memory on closing (false for user-supplied buffer) */
#define DOSLIB_FREEBUFFER   512u
/* stream has encountered error / EOF */
#define DOSLIB_ERRORFLAG   1024u
#define DOSLIB_EOFFLAG     2048u
/* stream is wide-oriented */
#define DOSLIB_WIDESTREAM  4096u
/* stream is byte-oriented */
#define DOSLIB_BYTESTREAM  8192u
/* file associated with stream should be remove()d on closing (tmpfile()) */
#define DOSLIB_DELONCLOSE 16384u
/* stream handle should not be free()d on close (stdin, stdout, stderr) */
#define DOSLIB_STATIC     32768u

#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 4

static DOSIO _FillBuffer(pDOSBase DOSBase, pFileHandle fh, UINT32 size)
{
	UINT32	bytesRead = Read(fh, (UINT8*)fh->fh_buffer, fh->fh_bufsize);
	
	if (IoErr() == DOSIO_FALSE )
	{
		if (bytesRead == 0)
		{
			fh->fh_status |= DOSLIB_EOFFLAG;
//			Printf("EOF\n");
			return EOF;
		}
		fh->fh_pos += bytesRead;
		fh->fh_bufend = bytesRead;
		fh->fh_bufidx = 0;
//		Printf("Read %d bytes\n", bytesRead);
		return 0;	
	} else {
//		Printf("Error in Fillbuffer\n");
		fh->fh_status |= DOSLIB_ERRORFLAG;
		return EOF;
	}
}

static inline UINT32 _GetChars( pDOSBase DOSBase, char * out, UINT32 n, INT32 stopchar, pFileHandle fh )
{
    UINT32 i = 0;
    INT32 c;
    while ( fh->fh_ungetidx > 0 && i != n )
    {
        c = (unsigned char) ( out[ i++ ] = fh->fh_ungetbuf[ --(fh->fh_ungetidx) ] );
        if( c == stopchar ) return i;
    }
    while ( i != n )
    {
		//Printf("bufidx: %d, bufend: %d [%d,%d]\n", fh->fh_bufidx, fh->fh_bufend, i, n);

        while ( fh->fh_bufidx != fh->fh_bufend && i != n) 
        {
            c = (unsigned char) fh->fh_buffer[ fh->fh_bufidx++ ];
			out[ i++ ] = c;
            if( c == stopchar ) return i;
        }

//		Printf("->bufidx: %d, bufend: %d [%d,%d]\n", fh->fh_bufidx, fh->fh_bufend, i, n);
		if (i != n)
//        if ( fh->fh_bufidx == fh->fh_bufend && (!fh->fh_Interactive))
        {
			//Printf("Fillbuffer %d\n", n);
            if( _FillBuffer( DOSBase, fh , n) == -1 ) break;
        }
    }
    return i;
}

static DOSIO _PrepWrite(pDOSBase DOSBase, pFileHandle fh)
{
    if ( ( fh->fh_bufidx < fh->fh_bufend ) || ( fh->fh_ungetidx > 0 ) ||
         ( fh->fh_status & ( DOSLIB_FREAD | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ) ) ||
         ! ( fh->fh_status & ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_FRW ) ) )
    {
		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		//KPrintF("error: \nfh_buidx: %d, fh_bufend: %d, fh_ungetidx: %d, fh_status: %d\n", fh->fh_bufidx, fh->fh_bufend, fh->fh_ungetidx, fh->fh_status);
		//KPrintF("firstbool: %x %x\n", ( DOSLIB_FREAD | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ), ( fh->fh_status & ( DOSLIB_FREAD | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ) ));
		//KPrintF("secondbool: %x %x\n", ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_FRW ), ( fh->fh_status & ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_FRW ) ));
		//for(;;);
        fh->fh_status |= DOSLIB_ERRORFLAG;
        return EOF;
    }
    fh->fh_status |= DOSLIB_FWRITE | DOSLIB_BYTESTREAM;
    return 0;
}

static DOSIO _PrepRead(pDOSBase DOSBase, pFileHandle fh, UINT32 size)
{
    if ( ( fh->fh_bufidx > fh->fh_bufend ) ||
         ( fh->fh_status & ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ) ) ||
         ! ( fh->fh_status & ( DOSLIB_FREAD | DOSLIB_FRW ) ) )
	{
		// SetIoErr errno = EINVAL;
		//KPrintF("error: \nfh_buidx: %d, fh_bufend: %d, fh_ungetidx: %d, fh_status: %d\n", fh->fh_bufidx, fh->fh_bufend, fh->fh_ungetidx, fh->fh_status);
		//KPrintF("firstbool: %d %x\n", ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ), ( fh->fh_status & ( DOSLIB_FWRITE | DOSLIB_FAPPEND | DOSLIB_ERRORFLAG | DOSLIB_WIDESTREAM | DOSLIB_EOFFLAG ) ));
		//KPrintF("secondbool: %d %x\n", ( DOSLIB_FREAD | DOSLIB_FRW ), ( fh->fh_status & ( DOSLIB_FREAD | DOSLIB_FRW ) ));
		//for(;;);
		Printf("Error prepread");
		fh->fh_status |= DOSLIB_ERRORFLAG;
		SetIoErr(ERROR_OBJECT_WRONG_TYPE);
		return EOF;
	}
    fh->fh_status |= DOSLIB_FREAD | DOSLIB_BYTESTREAM;
    if ( ( fh->fh_bufidx == fh->fh_bufend ) && ( fh->fh_ungetidx == 0 ) ) return _FillBuffer( DOSBase, fh, size );
    else return 0;
}

static DOSIO _FlushBuffer(pDOSBase DOSBase, pFileHandle fh)
{
	INT32 written = 0;
	INT32	rv = 0;
	INT32	length = fh->fh_bufidx;

    while(written != length) 
	{
		INT32 justWrote;
		INT32 toWrite = length - written;
		justWrote = Write(fh, fh->fh_buffer + written, toWrite);
        written += justWrote;
        fh->fh_pos += justWrote;

        if(!justWrote) 
		{
            fh->fh_status |=DOSLIB_ERRORFLAG;
            rv = EOF;
			break;
        }
    }

	fh->fh_bufidx -= written;
	memmove( fh->fh_buffer, fh->fh_buffer + written, fh->fh_bufidx );
    return rv;
}

DOSIO dos_FPutC(pDOSBase DOSBase, pFileHandle fh, INT32 c)
{
	if (_PrepWrite(DOSBase, fh) == EOF) return EOF;
	
	fh->fh_buffer[fh->fh_bufidx++] = (char) c;
	if 	( ( fh->fh_bufidx == fh->fh_bufsize )
	|| 	( ( fh->fh_status & _IOLBF ) && ( (char)c == '\n' ) )
	|| 	( fh->fh_status & _IONBF )
		)
	{
        /* buffer filled, unbuffered stream, or end-of-line. */
		//KPrintF("unbuffered\n");
        return ( _FlushBuffer( DOSBase, fh ) == 0 ) ? c : EOF;
    }
    return c;	
}

DOSIO dos_FPuts(pDOSBase DOSBase, pFileHandle fh, STRPTR s)
{
    if ( _PrepWrite(DOSBase, fh ) == EOF )
    {
        return EOF;
    }
    while ( *s != '\0' )
    {
        fh->fh_buffer[ fh->fh_bufidx++ ] = *s;
        if ( ( fh->fh_bufidx == fh->fh_bufsize ) ||
             ( ( fh->fh_status & _IOLBF ) && *s == '\n' )
           )
        {
            if ( _FlushBuffer( DOSBase, fh ) == EOF )
            {
                return EOF;
            }
        }
        ++s;
    }
    if ( fh->fh_status & _IONBF )
    {
        if ( _FlushBuffer( DOSBase, fh ) == EOF ) return EOF;
    }
    return 0;
}

DOSIO dos_FRead(pDOSBase DOSBase, pFileHandle fh, APTR ptr, UINT32 size, UINT32 nmemb)
{
    if ( _PrepRead( DOSBase, fh, size ) == EOF )
    {
        return 0;
    }
    char * dest = (char *)ptr;
    UINT32 nmemb_i;
    for ( nmemb_i = 0; nmemb_i < nmemb; ++nmemb_i )
    {
        UINT32 numread = _GetChars( DOSBase, &dest[ nmemb_i * size ], size, EOF, fh );
        if( numread != size ) break;
    }
    return nmemb_i;
}

DOSIO dos_FGetC(pDOSBase DOSBase, pFileHandle fh)
{
	if ( _PrepRead( DOSBase, fh, 1 ) == EOF ) return EOF;
	char c;
	UINT32 n = _GetChars( DOSBase, &c, 1, EOF, fh );
	//Printf("n= %d %x\n", n, c);
	return n == 0 ? EOF : (unsigned char) c;
}

STRPTR dos_FGets(pDOSBase DOSBase, pFileHandle fh, STRPTR s, UINT32 size)
{
	if ( size == 0 ) return NULL;
	if ( size == 1 )
	{
		*s = '\0';
		return s;
	}
	if ( _PrepRead( DOSBase, fh, size ) == EOF ) return NULL;
	char * dest = s;
	dest += _GetChars( DOSBase, dest, size - 1, '\n', fh );
	*dest = '\0';
	return ( dest == s ) ? NULL : s;
}

DOSIO dos_UnGetC(pDOSBase DOSBase, pFileHandle fh, INT32 c)
{
    (void)DOSBase;
//			fh->fh_status |= DOSLIB_EOFFLAG;
//			return EOF;

//	if (c == EOF || fh->fh_ungetidx == DOSLIB_UNGETCBUFSIZE ) return DOSIO_TRUE;
	if (fh->fh_ungetidx == DOSLIB_UNGETCBUFSIZE ) return DOSIO_FALSE;
	if (c == EOF)
	{
		if (fh->fh_status & DOSLIB_EOFFLAG) return EOF;
		
		char ch = fh->fh_buffer[ fh->fh_bufidx-1 ];
		//Printf("pushing back %c/%x\n", ch, ch);
		fh->fh_ungetbuf[fh->fh_ungetidx++] = ch;
		return ch;
	}
	fh->fh_ungetbuf[fh->fh_ungetidx++] = (UINT8) c;
	return c;
}

DOSIO dos_WriteChars(pDOSBase DOSBase, UINT8 *buffer, INT32 size)
{
	return FWrite(Output(), buffer, 1, size);
}

DOSIO dos_PutStr(pDOSBase DOSBase, UINT8 *buffer)
{
	return FPuts(Output(), (STRPTR)buffer);
}


DOSIO dos_FWrite(pDOSBase DOSBase, pFileHandle fh, const APTR ptr, UINT32 size, UINT32 nmemb)
{
	if ( _PrepWrite( DOSBase, fh ) == EOF ) return 0;
	UINT32 offset = 0;
	UINT32 nmemb_i;

    for ( nmemb_i = 0; nmemb_i < nmemb; ++nmemb_i )
    {
        for ( UINT32 size_i = 0; size_i < size; ++size_i )
        {
            if ( ( fh->fh_buffer[ fh->fh_bufidx++ ] = ((char*)ptr)[ nmemb_i * size + size_i ] ) == '\n' )
            {
                /* Remember last newline, in case we have to do a partial line-buffered flush */
                offset = fh->fh_bufidx;
                //lineend = true;
            }
            if ( fh->fh_bufidx == fh->fh_bufsize )
            {
                if ( _FlushBuffer( DOSBase, fh ) == EOF )
                {
                    /* Returning number of objects completely buffered */
                    return nmemb_i;
                }
                //lineend = false;
                /*
                 * The entire buffer has been flushed; this means we have to
                 * reset our newline position as we have already written
                 * that part of the stream.
                 */
                offset = 0;
            }
        }
    }
    /* Fully-buffered streams are OK. Non-buffered streams must be flushed,
       line-buffered streams only if there's a newline in the buffer.
    */
    switch ( fh->fh_status & ( _IONBF | _IOLBF ) )
    {
        case _IONBF:
            if ( _FlushBuffer( DOSBase, fh ) == EOF )
            {
                /* We are in a pinch here. We have an error, which requires a
                   return value < nmemb. On the other hand, all objects have
                   been written to buffer, which means all the caller had to
                   do was removing the error cause, and re-flush the stream...
                   Catch 22. We'll return a value one short, to indicate the
                   error, and can't really do anything about the inconsistency.
                */
                return nmemb_i - 1;
            }
            break;
        case _IOLBF:
            {
            UINT32 bufidx = fh->fh_bufidx;
            fh->fh_bufidx = offset;
            if ( _FlushBuffer( DOSBase, fh ) == EOF )
            {
                /* See comment above. */
                fh->fh_bufidx = bufidx;
                return nmemb_i - 1;
            }
            fh->fh_bufidx = bufidx - offset;
            memmove( fh->fh_buffer, fh->fh_buffer + offset, fh->fh_bufidx );
            }
    }
    return nmemb_i;
}

DOSIO dos_Flush(pDOSBase DOSBase, pFileHandle fh)
{
	if (fh != NULL)
	{
		return _FlushBuffer(DOSBase, fh);
	}
	return 0;
}

DOSIO dos_SeekInternal(pDOSBase DOSBase, pFileHandle fh, INT32 pos, INT32 mode);

static inline UINT64 _ftell64(pFileHandle fh)
{
    return ( fh->fh_pos - ( ( (int)fh->fh_bufend - (int)fh->fh_bufidx ) + (int)fh->fh_ungetidx ) );
}

DOSIO dos_Tell(pDOSBase DOSBase, pFileHandle fh)
{
    uint64_t    off64 = _ftell64(fh);
    if (off64 > INT_FAST64_MAX)
    {
        SetIoErr(ERROR_BAD_NUMBER);
        return -1;
    }
    return off64;
}

static INT64 _Seek(pDOSBase DOSBase, pFileHandle fh, INT64 loffset, INT32 whence)
{
    INT64 newPos = 0;
    INT32 off = loffset;
//    KPrintF("got for seek: %lld with: %lld, %d\n", newPos, loffset, whence);
    newPos = dos_SeekInternal(DOSBase, fh, off, whence);
    KPrintF("got from seek: %lld with: %lld, %d\n", newPos, loffset, whence);
    if ( (newPos == -1) && (IoErr() == ERROR_SEEK_ERROR) ) return EOF;
    fh->fh_ungetidx = 0;
    fh->fh_bufidx   = 0;
    fh->fh_bufend   = 0;
    fh->fh_pos      = newPos;
    return newPos;
}

DOSIO dos_Seek(pDOSBase DOSBase, pFileHandle fh, INT32 loffset, INT32 whence)
{
    UINT64 offset = loffset;
//    KPrintF("seek with: %lld, %d\n", offset, whence);
    
    if ( fh->fh_status & DOSLIB_FWRITE )
    {
        if ( _FlushBuffer( DOSBase, fh ) == EOF ) return EOF;
    }
//    KPrintF("seek with: %lld, %d\n", offset, whence);
    fh->fh_status &= ~ DOSLIB_EOFFLAG;
    if ( fh->fh_status & DOSLIB_FRW )
    {
        fh->fh_status &= ~ ( DOSLIB_FREAD | DOSLIB_FWRITE );
    }
//    KPrintF("seek with: %lld, %d\n", offset, whence);
    if ( whence == SEEK_CUR )
    {
        whence  = SEEK_SET;
        offset += _ftell64( fh );
    }
//    KPrintF("seek with: %lld, %d\n", offset, whence);
//    KPrintF("Calling _Seek: %lld, %d\n", offset, whence);
    int64_t ret = _Seek( DOSBase, fh, offset, whence );
//    return (ret != EOF)? (DOSIO)ret : EOF;
    return (ret != EOF ) ? 0 : EOF;
}

DOSIO dos_Close(pDOSBase DOSBase, pFileHandle fh)
{
	pProcess	proc = FindProcess(NULL);
	pMinList	FileList = &proc->pr_OpenFiles;
	pFileHandle	tmp;
	
	ForeachNode(FileList, tmp)
	{
		if (tmp == fh)
		{
            if ( fh->fh_status & DOSLIB_FWRITE )
            {
                if ( _FlushBuffer( DOSBase, fh ) == EOF ) return EOF;
            }

            /* Close handle */
//			KPrintF("fh->fh_Type: %x, Lock: %x", fh->fh_Type, fh->fh_Arg1);
			INT32 ret = DoPkt(fh->fh_Type, ACTION_END,(INT32)fh->fh_Arg1, 0, 0, 0, 0);
//			KPrintF("fh->fh_Type: %x", fh->fh_Type);
			//if (ret || ErrorReport(IoErr(), REPORT_STREAM, (INT32) fh, NULL))
			if (!ret)
			{
				KPrintF("CLOSE: ACTION_END ERROR\n");
				for(;;);
			}
			Remove((pNode)&fh->fh_Node);
#if 0
            /* Delete tmpfile() */
            if ( fh->fh_status & DOSLIB_DELONCLOSE )
            {
                remove( fh->fh_filename );
            }
#endif
            /* Free stream */
			if ( ( fh->fh_status & DOSLIB_FREEBUFFER) ) // changed from (!( to ((.
			{
//				KPrintF("Freebuffer\n");
				FreeVec( fh->fh_buffer );
			}
            if ( ! ( fh->fh_status & DOSLIB_STATIC ) ) 
            {
//				KPrintF("FH\n");
				FreeVec( fh );
			}
            return DOSIO_TRUE;
        }
    }		
	SetIoErr(ERROR_OBJECT_NOT_FOUND);
    return DOSIO_FALSE;
}

#if 0
#define MODE_OLDFILE	     1005	// Open existing file. Error if not found
#define MODE_NEWFILE	     1006	// Open/Create new File. EXCLUSIVE_LOCK
#define MODE_READWRITE	     1004	// Open old File with SHARED_LOCK, creates file if it doesnt exist
#endif

pFileHandle dos_Open(pDOSBase DOSBase, STRPTR string, INT32 mode)
{
	pProcess proc = FindProcess(NULL);

	if (string == NULL || mode == 0 || proc == NULL) return NULL;
	
	pFileHandle fh = AllocVec(sizeof(FileHandle) + DOSLIB_UNGETCBUFSIZE + DOSLIB_BUFSIZ, MEMF_PUBLIC|MEMF_FREE);
	if (fh) 
	{
		switch(mode)
		{
			case MODE_OLDFILE:
				fh->fh_status = DOSLIB_FREAD;
			break;
			case MODE_NEWFILE:
				fh->fh_status = DOSLIB_FWRITE;
			break;
			case MODE_READWRITE:
				fh->fh_status = DOSLIB_FAPPEND | DOSLIB_FWRITE;
			break;
		}
		
		fh->fh_ungetbuf = (INT8 *)fh + sizeof(FileHandle);
		fh->fh_buffer	= fh->fh_ungetbuf + DOSLIB_UNGETCBUFSIZE;
		fh->fh_bufsize	= DOSLIB_BUFSIZ;
		fh->fh_bufidx	= 0;
		fh->fh_bufend	= 0;
		fh->fh_ungetidx	= 0;
		AddTail((pList)&proc->pr_OpenFiles, (pNode)&fh->fh_Node);
		fh->fh_Interactive	= DOSIO_FALSE;
		
		pHandlerProc hp = ObtainHandler(string);
//		KPrintF("hp: %x\n", hp);
		if (hp == NULL && (IoErr() == 0))
		{
			// NIL:
			fh->fh_Type = NULL;
			fh->fh_status |= _IONBF;
			return fh;
		} else if (hp)
		{
//			KPrintF("Handler: %s\n", hp->hp_DevNode->de_Node.ln_Name);
			fh->fh_Type = hp->hp_Port;
//			KPrintF("Handler: %x\n", fh->fh_Type);
			BOOL ok = DoPkt(fh->fh_Type, mode, (INT32)fh, (INT32)hp->hp_Lock, (INT32)string, 0, 0);
//			if (fh->fh_Interactive && mode == MODE_OLDFILE) 
//			{
//				KPrintF("Opened an Interactive File\n");
//				fh->fh_status |= _IONBF;
//			} else
			fh->fh_status	|= _IOLBF;
//			KPrintF("new open: type: %x, mode: %d, fh: %x\n", fh->fh_Type, mode, fh );
//			KPrintF("ok: %x\n", ok);
			ReleaseHandler(hp);
			if (ok) return fh;
		}
		SetIoErr(ERROR_OBJECT_NOT_FOUND);
	} else
		SetIoErr(ERROR_NO_FREE_STORE);
	return NULL;
}



/*
Flush();
dos.library/FWrite
*/

