#ifndef TAGITEM_H
#define TAGITEM_H

#include "types.h"

typedef UINT32 Tag;

typedef struct TagItem
{
	Tag		ti_Tag;
	UINT32	ti_Data;
} TagItem, *pTagItem;

#define TAG_DONE   (0L)
#define TAG_END    (0L)
#define TAG_IGNORE (1L)
#define TAG_MORE   (2L)
#define TAG_SKIP   (3L)

#define TAG_USER    ((UINT32)(1L<<31))
#define TAG_OS	    (16L) 

#define TAGFILTER_AND 0
#define TAGFILTER_NOT 1

#define MAP_REMOVE_NOT_FOUND 0
#define MAP_KEEP_NOT_FOUND   1


#endif
