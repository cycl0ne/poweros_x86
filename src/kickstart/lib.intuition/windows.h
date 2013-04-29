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

typedef struct IDCMPMessage
{
	struct Message	ExecMessage;
	struct nWindow	*IDCMPWindow;
	UINT32			Class;
	UINT16			Code;
	UINT16			Qualifier;
	INT16			MouseX;
	INT16			MouseY;
	UINT32			Seconds;
	UINT32			Micros;
} IntuiMessage, *pIDCMPMessage;

struct nWindow {
	struct nScreen		*screen;

	INT32				x, y, width, height;
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
    struct MsgPort 		*UserPort, *WindowPort;
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
    struct MsgPort 		*UserPort, *WindowPort;
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
