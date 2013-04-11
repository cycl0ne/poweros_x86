#ifndef hooks_h
#define hooks_h
#include "types.h"
#include "list.h"

struct Hook
{
    struct MinNode	h_MinNode;
    APTR			(*h_Entry)();     /* Main entry point */
    APTR			(*h_SubEntry)();  /* Secondary entry point */
    APTR			h_Data;	    /* Whatever you want */
};

typedef APTR (*HOOKFUNC)();



#endif
