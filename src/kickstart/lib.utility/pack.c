#include "utility.h"
#include "pack.h"
#include "utility_funcs.h"

#define SysBase UtilBase->SysBase

union memaccess
{
    UINT8	ub;
    UINT16	uw;
    UINT32	ul;
    INT8	sb;
    INT16	sw;
    INT32	sl;
};


UINT32 util_PackStructureTags(pUtility UtilBase, APTR pack, UINT32 *packTable, struct TagItem *tagList)
{
    Tag				tagBase;
    UINT16			memOff;
    UINT16			tagOff;
    UINT8			bitOff;
    struct TagItem 	*ti;
    INT32			count = 0;
    union memaccess * memptr;


    tagBase = *packTable++;
    for( ; *packTable != 0; packTable++)
    {
		if(*packTable == -1)
		{
			tagBase = *++packTable;
			continue;
		}

		if((*packTable & PSTF_PACK))    continue;
		tagOff = (*packTable >> 16) & 0x3FF;
		ti = FindTagItem(tagBase + tagOff, tagList);
		if(ti == NULL) continue;

		memOff = *packTable & 0x1FFF;
		bitOff = (*packTable & 0xE000) >> 13;
		memptr = (union memaccess *)((UINT8 *)pack + memOff);

		if((*packTable & (PKCTRL_BIT|PSTF_EXISTS)) == (PKCTRL_BIT|PSTF_EXISTS))
		{
			if(*packTable & PSTF_SIGNED)
				memptr->ub &= ~(1 << bitOff);
			else
				memptr->ub |= (1 << bitOff);
			count++;
			continue;
		}

		switch((*packTable >> 24) & 0x98)
		{
			case (PKCTRL_ULONG >> 24):
			memptr->ul = ti->ti_Data;
			break;

			case (PKCTRL_UWORD >> 24):
			memptr->uw = ti->ti_Data;
			break;

			case (PKCTRL_UBYTE >> 24):
			memptr->ub = ti->ti_Data;
			break;

			case (PKCTRL_LONG >> 24):
			memptr->sl = ti->ti_Data;
			break;

			case (PKCTRL_WORD >> 24):
			memptr->sw = ti->ti_Data;
			break;

			case (PKCTRL_BYTE >> 24):
			memptr->sb = ti->ti_Data;
			break;

			case (PKCTRL_BIT >> 24):
			if(ti->ti_Data)
				memptr->ub |= (1L << bitOff);
			else
				memptr->ub &= ~(1L << bitOff);
			break;

			case (PKCTRL_FLIPBIT >> 24):
			if(ti->ti_Data)
				memptr->ub &= ~(1L << bitOff);
			else
				memptr->ub |= (1L << bitOff);
			break;
			default:
			count--;
		}
		count++;
    }
    return count;
}

UINT32 util_UnpackStructureTags(pUtility UtilBase, APTR pack, UINT32 *packTable, struct TagItem *tagList)
{
    Tag				tagBase;
    UINT16			memOff;
    UINT16			tagOff;
    UINT8			bitOff;
    struct TagItem *ti;
    INT16			count = 0;
    union memaccess *memptr;

    tagBase = *packTable++;
    for( ; *packTable != 0; packTable++)
    {
		if(*packTable == -1)
		{
			tagBase = *++packTable;
			continue;
		}

		if((*packTable & PSTF_UNPACK))    continue;
		tagOff = (*packTable >> 16) & 0x3FF;
		ti = FindTagItem(tagBase + tagOff, tagList);
		if(ti == NULL) continue;

		memOff = *packTable & 0x1FFF;
		bitOff = (*packTable & 0xE000) >> 13;
		memptr = (union memaccess *)((UINT8 *)pack + memOff);

		switch(*packTable & 0x98000000)
		{
			case PKCTRL_ULONG:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->ul;
			break;

			case PKCTRL_UWORD:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->uw;
			break;

			case PKCTRL_UBYTE:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->ub;
			break;

			case PKCTRL_LONG:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->sl;
			break;

			case PKCTRL_WORD:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->sw;
			break;

			case PKCTRL_BYTE:
			*(IPTR *)ti->ti_Data = (IPTR)memptr->sb;
			break;

			case PKCTRL_BIT:
			if( memptr->ub & (1 << bitOff) )
				*(IPTR *)ti->ti_Data = TRUE;
			else
				*(IPTR *)ti->ti_Data = FALSE;
			break;

			case PKCTRL_FLIPBIT:
			if( memptr->ub & (1 << bitOff) )
				*(IPTR *)ti->ti_Data = FALSE;
			else
				*(IPTR *)ti->ti_Data = TRUE;
			break;
			default:
			count--;
		}
		count++;
	} 
    return count;
}
