#ifndef WINDOWS_H
#define WINDOWS_H

#include "types.h"
#include "ports.h"
#include "tasks.h"
#include "font.h"
#include "regions.h"
#include "rastport.h"
#include "pixmap.h"

#define BARFILL			RGB(225,225,225)
#define BARLINEDARK		RGB( 96, 96, 96)
#define BARLINELIGHT	RGB(155,155,155)
#define BLACK			RGB(  0,  0,  0)

#define BACKGROUND		RGB(170,170,170)
#define WHITE			RGB(255,255,255)

/* --- Flags requested at OpenWindow() time by the application --------- */
#define WFLG_SIZEGADGET	    0x00000001L	/* include sizing system-gadget? */
#define WFLG_DRAGBAR	    0x00000002L	/* include dragging system-gadget? */
#define WFLG_DEPTHGADGET    0x00000004L	/* include depth arrangement gadget? */
#define WFLG_CLOSEGADGET    0x00000008L	/* include close-box system-gadget? */

#define WFLG_SIZEBRIGHT	    0x00000010L	/* size gadget uses right border */
#define WFLG_SIZEBBOTTOM    0x00000020L	/* size gadget uses bottom border */

/* --- refresh modes ------------------------------------------------------ */
/* combinations of the WFLG_REFRESHBITS select the refresh type */
#define WFLG_REFRESHBITS    0x000000C0L
#define WFLG_SMART_REFRESH  0x00000000L
#define WFLG_SIMPLE_REFRESH 0x00000040L
#define WFLG_SUPER_BITMAP   0x00000080L
#define WFLG_OTHER_REFRESH  0x000000C0L

#define WFLG_BACKDROP	    0x00000100L	/* this is a backdrop window */

#define WFLG_REPORTMOUSE    0x00000200L	/* to hear about every mouse move */

#define WFLG_GIMMEZEROZERO  0x00000400L	/* a GimmeZeroZero window	*/

#define WFLG_BORDERLESS	    0x00000800L	/* to get a Window sans border */

#define WFLG_ACTIVATE	    0x00001000L	/* when Window opens, it's Active */

/* --- Other User Flags --------------------------------------------------- */
#define WFLG_RMBTRAP	    0x00010000L	/* Catch RMB events for your own */
#define WFLG_NOCAREREFRESH  0x00020000L	/* not to be bothered with REFRESH */

/* - V36 new Flags which the programmer may specify in NewWindow.Flags	*/
#define WFLG_NW_EXTENDED    0x00040000L	/* extension data provided	*/
					/* see struct ExtNewWindow	*/

/* - V39 new Flags which the programmer may specify in NewWindow.Flags	*/
#define WFLG_NEWLOOKMENUS   0x00200000L	/* window has NewLook menus	*/


/* These flags are set only by Intuition.  YOU MAY NOT SET THEM YOURSELF! */
#define WFLG_WINDOWACTIVE   0x00002000L	/* this window is the active one */
#define WFLG_INREQUEST	    0x00004000L	/* this window is in request mode */
#define WFLG_MENUSTATE	    0x00008000L	/* Window is active with Menus on */
#define WFLG_WINDOWREFRESH  0x01000000L	/* Window is currently refreshing */
#define WFLG_WBENCHWINDOW   0x02000000L	/* WorkBench tool ONLY Window */
#define WFLG_WINDOWTICKED   0x04000000L	/* only one timer tick at a time */

/* V36 and higher flags to be set only by Intuition: */
#define WFLG_VISITOR	    0x08000000L	/* visitor window		*/
#define WFLG_ZOOMED	    0x10000000L	/* identifies "zoom state"	*/
#define WFLG_HASZOOM	    0x20000000L	/* window has a zoom gadget	*/

struct nWindow {
	struct nScreen		*screen;

	INT32				x, y, width, height;
	UINT32				Flags;
	struct CRastPort	*frp;
	struct Task			*owner;

	UINT32				id;
	struct nWindow		*next;
	struct nWindow		*parent;
	struct nWindow		*children;
	struct nWindow		*siblings;

	BOOL				mapped;
	BOOL				realized;
	ClipRegion			*clipregion;
	PixMap				*buffer;

	INT32				bordersize;
	UINT32				bordercolor;
	UINT32				background;

	INT32				xoff, yoff;
	ClipRegion			*userclipregion;

	CSTRPTR				title;
    UINT32				IDCMPFlags;

	UINT32				MousePending;
	UINT32				RptPending;
	
	struct MsgPort		WindowPort;
    struct MsgPort 		*UserPort, *WPort;
    struct IntuiMessage *MessageKey;
};

struct nScreen {
	struct nWindow		root;
	struct nWindow		*list;
	CGfxFont			*Font;
    struct ViewPort 	*ViewPort;
	INT32				MouseX, MouseY;
	UINT32				Flags;
	CSTRPTR				Title;
	CSTRPTR				DefaultTitle;
    SignalSemaphore		LockScreen;
};

#define WA_Dummy	     (TAG_USER + 99)
#define WA_Left 	     (WA_Dummy + 1)
#define WA_Top		     (WA_Dummy + 2)
#define WA_Width	     (WA_Dummy + 3)
#define WA_Height	     (WA_Dummy + 4)

typedef struct Window 
{
	struct Window		*next;
	struct Task			*owner;
	struct Window		*parent;
	struct Window		*children;
	struct Window		*siblings;
	struct Screen		*screen;
	INT32				bordersize;
	UINT32				bordercolor;
	UINT32				background;
	
	INT32				x, y, width, height;	// Windows coordinates in Screen
	CRastPort			*rp;						// Windows Rastport for Drawing/Clipping
	UINT32				id;						// Window ID
	STRPTR				title;
	STRPTR				screenTitle;

	ClipRegion			*clipregion;
	INT32				xoff, yoff;
	ClipRegion			*userclipregion;
	
	PixMap				*buffer;
	
	BOOL				realized;
	BOOL				mapped;
	// IDCMP 
    UINT32				IDCMPFlags;
    struct MsgPort 		*UserPort;
    struct MsgPort		WindowPort; // Embedded MsgPort for this Window!
    struct IntuiMessage *MessageKey;	

#if 0
	// Not implemented at the moment
    struct Menu 		*MenuStrip;
    struct Gadget 		*FirstGadget;
    struct Requester 	*FirstRequest;
    INT32				ReqCount;
#endif
} Window, *pWindow;

#endif
