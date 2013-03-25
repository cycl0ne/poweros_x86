#ifndef irq_h_ints
#define irq_h_ints

#include "types.h"
#include "list.h"

struct Interrupt {
    struct  Node is_Node;
	//iroutine	is_Handler;
	UINT32		is_Cycles;
	UINT32		is_Count;
    APTR    	is_Data;		    /* server data segment  */
    void    	*(*is_Code)();	    /* server code entry    */
};

struct IntVector 
{
    List		iv_List;
    APTR		iv_Data;	// For Quicker Access the highest Prio Int is stored here
    VOID		*(*iv_Code)(); // Highest Prio Int
};

#endif
