/*
 * All Input Events are handled here. What are we doing? We have 2 Streams: 
 * a) input.device giving us: Keyboard, 100hz Timer, Mouse
 * b) intuition.library giving us Statechanges and Workload
 */
#include "intuitionbase.h"
#include "input.h"
#include "idcmp.h"
#include "intupools.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

struct IToken {
	struct PoolNode		it_Node;
	struct InputEvent	*it_IE;
};

struct InputEvent *IntuitionInput(struct InputEvent *ie, IntuitionBase *IBase)
{
//	DPrintF("ie");
	return ie;
}

#if 0
struct IToken *_NewIEToken(IntuitionBase *IBase, struct InputEvent *ie, enum ITCommand command)
{
	struct IToken *ret = (struct IToken *)GetPool(IBase, IBase->ITFreeList);
	if (ret)
	{
		ret->it_Command = command;
		ret->it_IE		= ie;
		ret->it_Flags	= 0;
		AddTail(&IBase->TokenQueue, (struct Node *)ret);
	}
	return ret;
}

void _CreateIToken(IntuitionBase *IBase, struct InputEvent *ie)
{
    for ( ; ie; ie = ie->ie_NextEvent )
    {
		switch ( ie->ie_Class )
		{
		case IECLASS_RAWKEY:
			convertRawKey( IBase, ie );
			break;
		case IECLASS_POINTERPOS:
		case IECLASS_NEWPOINTERPOS:
			//convertPointerPos( ie, &currpoint );
			break;
		case IECLASS_RAWMOUSE:
			convertMouse( IBase, ie );
			break;
		case IECLASS_TIMER:
			_NewIEToken( IBase, ie, itTIMER );
			break;
		default:
			_NewIEToken( IBase, ie, itUNKNOWNIE );
		}
    }
}
#endif

#if 0
struct IntuiMessage *_AllocEvent()
{
	
}

/*
 * Handle all mouse events.  These are mouse enter, mouse exit, mouse
 * motion, mouse position, button down, and button up.  This also moves
 * the cursor to the new mouse position and changes it shape if needed.
 */
void GsHandleMouseStatus(GR_COORD newx, GR_COORD newy, int newbuttons)
{
	int	 changebuttons;	/* buttons that have changed */
	MWKEYMOD modifiers;	/* latest modifiers */
	
	GdGetModifierInfo(NULL, &modifiers); /* Read kbd modifiers */

	/* If we are currently in raw mode, then just deliver the raw event */
	if (mousedev.flags & MOUSE_RAW) { 
		GsDeliverRawMouseEvent(newx, newy, newbuttons, modifiers);
		return;
	}

	/*
	 * First, if the mouse has moved, then position the cursor to the
	 * new location, which will send mouse enter, mouse exit, focus in,
	 * and focus out events if needed.  Check here for mouse motion and
	 * mouse position events.  Flush the device queue to make sure the
	 * new cursor location is quickly seen by the user.
	 */
	if (newx != cursorx || newy != cursory) {
		GsResetScreenSaver();
		GrMoveCursor(newx, newy);

		GsDeliverMotionEvent(GR_EVENT_TYPE_MOUSE_MOTION, newbuttons, modifiers);
		GsDeliverMotionEvent(GR_EVENT_TYPE_MOUSE_POSITION, newbuttons, modifiers);
	}

	/*
	 * Next, generate a button up event if any buttons have been released.
	 */
	changebuttons = (curbuttons & ~newbuttons);
	if (changebuttons) {

	  GsResetScreenSaver();
	  GsDeliverButtonEvent(GR_EVENT_TYPE_BUTTON_UP, newbuttons, changebuttons, modifiers);
	}

	/*
	 * Finally, generate a button down event if any buttons have been
	 * pressed.
	 */
	changebuttons = (~curbuttons & newbuttons);
	if (changebuttons) {
/*** removed - double mouse button exits server
		if ((newbuttons&(GR_BUTTON_L|GR_BUTTON_R)) == (GR_BUTTON_L|GR_BUTTON_R))
			GsTerminate();
***/
		GsResetScreenSaver();
		GsDeliverButtonEvent(GR_EVENT_TYPE_BUTTON_DOWN,
			newbuttons, changebuttons, modifiers);
	}

	curbuttons = newbuttons;
}


void
GsSelect(GR_TIMEOUT timeout)
{
	/* If mouse data present, service it*/
	if(mousedev.Poll())
		while(GsCheckMouseEvent())
			continue;

	/* If keyboard data present, service it*/
	if(kbddev.Poll())
		while(GsCheckKeyboardEvent())
			continue;

}

GR_BOOL GsCheckMouseEvent(void)
{
	GR_COORD	rootx;		/* latest mouse x position */
	GR_COORD	rooty;		/* latest mouse y position */
	int		newbuttons;	/* latest buttons */
	int		mousestatus;	/* latest mouse status */

	/* Read the latest mouse status: */
	mousestatus = GdReadMouse(&rootx, &rooty, &newbuttons);
	if(mousestatus < 0) {
		GsError(GR_ERROR_MOUSE_ERROR, 0);
		return FALSE;
	} else if(mousestatus) {	/* Deliver events as appropriate: */	
		GsHandleMouseStatus(rootx, rooty, newbuttons);

		/* possibly reset portrait mode based on mouse position*/
		if (autoportrait)
			GsSetPortraitModeFromXY(rootx, rooty);
		return TRUE;
	}
	return FALSE;
}




GR_EVENT *GsAllocEvent(GR_CLIENT *client)
{
	GR_EVENT_LIST	*elp;		/* current element list */
	GR_CLIENT	*oldcurclient;	/* old current client */

	/*
	 * Get a new event structure from the free list, or else
	 * allocate it using malloc.
	 */
	elp = eventfree;
	if (elp)
		eventfree = elp->next;
	else {
		elp = (GR_EVENT_LIST *) malloc(sizeof(GR_EVENT_LIST));
		if (elp == NULL) {
			oldcurclient = curclient;
			curclient = client;
			GsError(GR_ERROR_MALLOC_FAILED, 0);
			curclient = oldcurclient;
			return NULL;
		}
	}

	/*
	 * Add the event to the end of the event list.
	 */
	if (client->eventhead)
	  if (!client->eventtail)
	    client->eventtail = elp;
	  else
	    client->eventtail->next = elp;
	else
	  client->eventhead = elp;
	
	client->eventtail = elp;
	elp->next = NULL;
	elp->event.type = GR_EVENT_TYPE_NONE;

	return &elp->event;
}
#endif
