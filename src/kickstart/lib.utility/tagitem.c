#include "utility.h"
#include "utility_funcs.h"

#define SysBase UtilBase->SysBase

struct TagItem *util_NextTagItem(pUtility UtilBase, const struct TagItem **tagListPtr)
{
	if (!(*tagListPtr)) return NULL;
	while(1)
	{
		switch((*tagListPtr)->ti_Tag)
		{
			case TAG_MORE:
				if (!((*tagListPtr) = (struct TagItem *)(*tagListPtr)->ti_Data)) return NULL;
				continue;
			case TAG_IGNORE:
				break;
			case TAG_END:
				(*tagListPtr) = 0;
				return NULL;
			case TAG_SKIP:
				(*tagListPtr) += (*tagListPtr)->ti_Data + 1;
				continue;
			default:
				return (struct TagItem *)(*tagListPtr)++;
		}
		(*tagListPtr)++;
	}
}

struct TagItem *util_FindTagItem(pUtility UtilBase, UINT32 tagValue, const struct TagItem *tagList)
{
    struct TagItem *tag;
    const struct TagItem *tagptr = tagList;

    while( (tag = (struct TagItem *)NextTagItem(&tagptr)) )
    {
    	if(tag->ti_Tag == (UINT32)tagValue) return tag;
    }
    return NULL;
}

BOOL util_TagInArray(pUtility UtilBase, UINT32 tagValue, UINT32 *tagArray)
{
    while(*tagArray != TAG_DONE)
    {
    	if(*tagArray == tagValue) return TRUE;
	    tagArray++;
    }
    return FALSE;
}

UINT32 util_GetTagData(pUtility UtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList)
{
	struct TagItem *ti = NULL;
    if (tagList && (ti = FindTagItem(tagValue, tagList))) return ti->ti_Data;
    return defaultVal;
}

struct TagItem *util_AllocateTagItems(pUtility UtilBase, UINT32 numTags)
{
    struct TagItem *tags = NULL;
    if (numTags) tags = AllocVec( numTags * sizeof(struct TagItem) , MEMF_CLEAR | MEMF_PUBLIC );
    return tags;
}

void util_FreeTagItems(pUtility UtilBase, struct TagItem *tagList)
{
	//FreeVec accepts NULL;
	FreeVec(tagList);
}

void util_RefreshTagItemClones(pUtility UtilBase, struct TagItem *clone, const struct TagItem *original)
{
    struct TagItem *current;
    if (!clone) return;
    if (!original)
    {
    	clone->ti_Tag = TAG_DONE;
	    return;
    }
    while ((current = NextTagItem (&original)))
    {
    	*clone = *current;
	    clone++;
    }
}

struct TagItem *util_CloneTagItems(pUtility UtilBase, const struct TagItem *tagList)
{
    struct TagItem *newList;
    INT32 numTags = 1;
    if (tagList)
    {
    	const struct TagItem *tmp;
	    tmp = tagList;
	    while (NextTagItem (&tmp) != NULL) numTags++;
    }
    if (newList = AllocateTagItems(numTags)) RefreshTagItemClones(newList, tagList);
	return newList;
}

UINT32 util_PackBoolTags(pUtility UtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap)
{
    struct TagItem *current, *found, *tstate = tagList;
    while ((current = (struct TagItem *) NextTagItem ((const struct TagItem **)&tstate)))
    {
    	if ((found = (struct TagItem *)FindTagItem (current->ti_Tag, boolMap)))
	    {
	        if (current->ti_Data == 0) initialFlags &= ~(found->ti_Data);
	        else
		        initialFlags |= found->ti_Data;
	    }
    }
    return initialFlags;
}

UINT32 util_FilterTagItems(pUtility UtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic)
{
    UINT32 valid = 0;
    if(tagList && filterArray)
    {
    	struct TagItem *ti;

	    while((ti = NextTagItem((const struct TagItem **)&tagList)))
	    {
	        if(logic == TAGFILTER_AND)
	        {
		        if(TagInArray(ti->ti_Tag, filterArray)) valid++;
		    else
		        ti->ti_Tag = TAG_IGNORE;
	        }
	        else if(logic == TAGFILTER_NOT)
	        {
    		    if(TagInArray(ti->ti_Tag, filterArray)) ti->ti_Tag = TAG_IGNORE;
	    	else
		        valid++;
	        }
	    }
    }
    return valid;
}

void util_FilterTagChanges(pUtility UtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply)
{
    if (originalList && changeList)
    {
    	struct TagItem *change, *orig;

    	while ((change = NextTagItem((const struct TagItem **)&changeList)))
	    {
	        if ((orig = FindTagItem(change->ti_Tag, originalList)))
	        {
        		if (change->ti_Data == orig->ti_Data) change->ti_Tag = TAG_IGNORE;
		        else
		        {
        		    if (apply) orig->ti_Data = change->ti_Data;
		        }
	        }
	    } 
    } 
}

void util_MapTags(pUtility UtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType)
{
    struct TagItem *tag;
    struct TagItem *map;

    while ((tag = NextTagItem ((const struct TagItem **)&tagList)))
    {
    	if (mapList && (map = FindTagItem (tag->ti_Tag, mapList)))
	    {
	        if (map->ti_Data == TAG_DONE) tag->ti_Tag = TAG_IGNORE;
    	    else
		        tag->ti_Tag = map->ti_Data;
	    }
	    else if (mapType == MAP_REMOVE_NOT_FOUND) tag->ti_Tag = TAG_IGNORE;
    }
}

void util_ApplyTagChanges(pUtility UtilBase, struct TagItem *list, struct TagItem *changelist)
{
    for(;;)
    {
    	switch(list->ti_Tag)
	    {
	        case TAG_END:
				return;
	        case TAG_IGNORE:
				break;
    	    case TAG_MORE:
				list=(struct TagItem *)list->ti_Data;
			continue;
    	    case TAG_SKIP:
				list+=(UINT32)list->ti_Data;
    		break;
    	    default:
    	    {
    		    struct TagItem *tagitem;
    		    tagitem = FindTagItem(list->ti_Tag, changelist);
        		if (tagitem!=NULL) list->ti_Data = tagitem->ti_Data;
    		    break;
	        }
	    }
	list++;
    }
}
