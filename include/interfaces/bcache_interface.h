/**
* File: /bcache_interface．h
* User: cycl0ne
* Date: 2014-10-22
* Time: 05:10 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef BCACHE_INTERFACE_H
#define BCACHE_INTERFACE_H
#include "types.h"
#include "devices.h"

typedef struct BCacheBase *pBCacheBase;
typedef struct bcache {
} bcache_t;
//typedef void * bcache_t;

bcache_t CreateCache(struct IOStdReq* io, INT32 block_size, INT32 block_count);
void DestroyCache(bcache_t *_cache);
int ReadBlock(bcache_t *_cache, void *buf, UINT64 blocknum);
int GetBlock(bcache_t *_cache, void **ptr, UINT64 blocknum);
int PutBlock(bcache_t *_cache, UINT64 blocknum);
int MarkBlockDirty(bcache_t *_cache, UINT64 blocknum);
int ZeroBlock(bcache_t *_cache, UINT64 blocknum);
int FlushCache(bcache_t *_cache);
void DumpCache(bcache_t *_cache);

#if 1
#define CreateCache(a,b,c)		_LIBCALL(BCBase,  5, bcache_t * , (pBCacheBase, struct IOStdReq*, INT32, INT32), (BCBase,a,b,c))
#define DestroyCache(a)		    _LIBCALL(BCBase,  6, void	 	, (pBCacheBase, bcache_t *), (BCBase,a))
#define ReadBlock(a,b,c)		_LIBCALL(BCBase,  7, int		, (pBCacheBase, bcache_t *, void*, UINT64), (BCBase,a,b,c))
#define GetBlock(a,b,c)		    _LIBCALL(BCBase,  8, int		, (pBCacheBase, bcache_t *, void**, UINT64), (BCBase,a,b,c))
#define PutBlock(a,b)	    	_LIBCALL(BCBase,  9, int		, (pBCacheBase, bcache_t *, UINT64), (BCBase,a,b))
#define MarkBlockDirty(a,b)		_LIBCALL(BCBase, 10, int		, (pBCacheBase, bcache_t *, UINT64), (BCBase,a,b))
#define ZeroBlock(a,b)		    _LIBCALL(BCBase, 11, int		, (pBCacheBase, bcache_t *, UINT64), (BCBase,a,b))
#define FlushCache(a)	    	_LIBCALL(BCBase, 12, int		, (pBCacheBase, bcache_t *), (BCBase,a))
#define DumpCache(a)    		_LIBCALL(BCBase, 13, bcache_t	, (pBCacheBase, bcache_t *), (BCBase,a))

#else

#define CreateCache(a,b,c)	(((bcache_t	*(*)(pBCacheBase, struct IOStdReq*, INT32, INT32))	_GETVECADDR(BCBase, 5))(BCBase,a,b,c))
#define DestroyCache(a)		(((void	 	(*)(pBCacheBase, bcache_t *))							_GETVECADDR(BCBase, 6))(BCBase,a))
#define ReadBlock(a,b,c)	(((int		(*)(pBCacheBase, bcache_t *, void*, UINT64))			_GETVECADDR(BCBase, 7))(BCBase,a,b,c))
#define GetBlock(a,b,c)		(((int		(*)(pBCacheBase, bcache_t *, void**, UINT64))			_GETVECADDR(BCBase, 8))(BCBase,a,b,c))
#define PutBlock(a,b)		(((int		(*)(pBCacheBase, bcache_t *, UINT64))					_GETVECADDR(BCBase, 9))(BCBase,a,b))
#define MarkBlockDirty(a,b)	(((int		(*)(pBCacheBase, bcache_t *, UINT64))					_GETVECADDR(BCBase,10))(BCBase,a,b))
#define ZeroBlock(a,b)		(((int		(*)(pBCacheBase, bcache_t *, UINT64))					_GETVECADDR(BCBase,11))(BCBase,a,b))
#define FlushCache(a)		(((int		(*)(pBCacheBase, bcache_t *))							_GETVECADDR(BCBase,12))(BCBase,a))
#define DumpCache(a)		(((bcache_t	(*)(pBCacheBase, bcache_t *))							_GETVECADDR(BCBase,13))(BCBase,a))
#endif

#endif
