/*
 * We define a Server-> Client Architecture here.
 * If we shouldnt do something on the calling Task, then defer the command to the InputHandler from us.
 * Let him do the job.
 */

#include "intuitionbase.h"
#include "intupools.h"
#include "input.h"

// The Command Interface we support

enum ICommand {
	iOpenScreen,
	iCloseScreen,
	iOpenWindow,
	iCloseWindow,
	iGetWindowInfo,
	iMoveWindow,
	iSizeWindow,
	iWindowToFront,
	iWindowToBack,
	iModifyIDCMP
};

struct IToken {
	struct PoolNode	it_Node;
	enum ICommand	it_Command;
	UINT32			it_Flags;
	
};

#if 0

struct InputToken	{
    struct Node	it_Node;	/* in list				*/
    UWORD	it_Flags;	/* below				*/
    enum ITCommand  it_Command;	/* from enum above			*/
    ULONG	it_SubCommand;	/* such as front/back, ...		*/
    CPTR	it_Object1;	/* window, screen, gadget, parameter	*/
    CPTR	it_Object2;	/* secondary pointer parameter		*/
    union
    {
	struct IBox it_box;	/* pos/dim parameters for itCHANGEWIN	*/
	struct LongPoint it_longmouse;	/* For itMOUSEMOVE		*/
    } it_Position;

    struct InputEvent	*it_IE;	/* copy of input event			*/
    struct Task	*it_Task;	/* who to signal for ITF_SIGNAL 	*/
    struct TabletData *it_Tablet;	/* Tablet data if any		*/
    UBYTE	it_Code;	/* result of RawKeyConvert for RAWKEY	*/
    ULONG	it_Error;	/* return code				*/
};


#endif
