#ifndef residents_h
#define residents_h

#include "types.h"
#include "lists.h"

#define RTC_MATCHWORD 0x0DEC0DE0

#define RTF_COLDSTART  (1<<0)
#define RTF_SINGLETASK (1<<1)
#define RTF_AFTERDOS   (1<<2)
#define RTF_TESTCASE   (1<<5)
#define RTF_AUTOINIT   (1<<7)

#define RTW_NEVER     0
#define RTW_COLDSTART 1

typedef struct Resident
{
    UINT32	rt_MatchWord; /* equal to RTC_MATCHWORD (see RTC_MATCHWORD) */
    struct Resident * volatile rt_MatchTag;  /* Pointer to this struct */
    APTR	rt_EndSkip;
    UINT8	rt_Flags;     /* see Flags */
    UINT8	rt_Version;
    UINT8	rt_Type;
    INT8	rt_Pri;
    STRPTR	rt_Name;
    STRPTR	rt_IdString;
    APTR	rt_Init;
}Resident, *pResident;

typedef struct ResidentNode
{
	Node		rn_Node;
	pResident	rn_Resident;
}ResidentNode, *pResidentNode;

#define RESIDENT_TAG __attribute__((section(".resident"))) Resident

#endif
