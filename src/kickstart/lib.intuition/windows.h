#ifndef WINDOWS_H
#define WINDOWS_H

#include "types.h"
#include "ports.h"
#include "tasks.h"
#include "region.h"
#include "rastport.h"
#include "pixmap.h"
#include "screen.h"

typedef struct IDCMPMessage
{
	struct Message	ExecMessage;
	struct Window	*IDCMPWindow;
	UINT32			Class;
	UINT16			Code;
	UINT16			Qualifier;
	INT16			MouseX;
	INT16			MouseY;
	UINT32			Seconds;
	UINT32			Micros;
} IntuiMessage, *IDCMPMessage;

typedef struct Window 
{
	struct Window		*next;
	struct Task			*owner;
	struct Window		*parent;
	struct Window		*children;
	struct Screen		*screen;
	INT32				bordersize;
	UINT32				bordercolor;
	UINT32				background;
	
	INT32				x, y, width, height;	// Windows coordinates in Screen
	CRastPort			rp;						// Windows Rastport for Drawing/Clipping
	UINT32				id;						// Window ID
	STRPTR				title;
	STRPTR				screenTitle;

	ClipRegion			*clipregion;
	PixMap				*buffer;
	
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
