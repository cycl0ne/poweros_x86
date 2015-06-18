#ifndef DOS_EXALL_H
#define DOS_EXALL_H

#include "types.h"

struct ExAllData {
	struct ExAllData *ed_Next;
	STRPTR	ed_Name;
	INT32	ed_Type;
	UINT32	ed_Size;
	UINT32	ed_Prot;
	UINT32	ed_Days;
	UINT32	ed_Mins;
	UINT32	ed_Ticks;
	STRPTR	ed_Comment;	/* strings will be after last used field */
	UINT16	ed_OwnerUID;	/* new for V39 */
	UINT16	ed_OwnerGID;
};

struct ExAllControl {
	UINT32	eac_Entries;	 /* number of entries returned in buffer      */
	UINT32	eac_LastKey;	 /* Don't touch inbetween linked ExAll calls! */
	STRPTR	eac_MatchString; /* wildcard string for pattern match or NULL */
	APTR	eac_MatchFunc;
	//struct Hook *eac_MatchFunc; /* optional private wildcard function     */
};

//
//
#define	ED_NAME		1
#define	ED_TYPE		2
#define ED_SIZE		3
#define ED_PROTECTION	4
#define ED_DATE		5
#define ED_COMMENT	6
#define ED_OWNER	7



#endif
