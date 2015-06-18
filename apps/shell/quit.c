/**
 * @file cli.c
 *
 * CLI Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "gfx.h"
#include "fonts.h"

#if 1

#include "timer.h"

struct InputEvent
{
    struct InputEvent * ie_NextEvent;

    UINT8 ie_Class;     /* see below for definitions */
    UINT8 ie_SubClass;  /* see below for definitions */
    UINT16 ie_Code;      /* see below for definitions */
    UINT16 ie_Qualifier; /* see below for definitions */

    union
    {
        struct
        {
            INT16 ie_x;
            INT16 ie_y;
        } ie_xy;

        APTR ie_addr;

        struct
        {
            UINT8 ie_prev1DownCode;
            UINT8 ie_prev1DownQual;
            UINT8 ie_prev2DownCode;
            UINT8 ie_prev2DownQual;
        } ie_dead;
    } ie_position;

    struct TimeVal      ie_TimeStamp;
};
#define ie_X             ie_position.ie_xy.ie_x
#define ie_Y             ie_position.ie_xy.ie_y
#define ie_EventAddress  ie_position.ie_addr
#define ie_Prev1DownCode ie_position.ie_dead.ie_prev1DownCode
#define ie_Prev1DownQual ie_position.ie_dead.ie_prev1DownQual
#define ie_Prev2DownCode ie_position.ie_dead.ie_prev2DownCode
#define ie_Prev2DownQual ie_position.ie_dead.ie_prev2DownQual

/* InputEvent Classes */
#define IECLASS_NULL           0
#define IECLASS_RAWKEY         1
#define IECLASS_RAWMOUSE       2
#define IECLASS_EVENT          3
#define IECLASS_POINTERPOS     4
#define IECLASS_TIMER          6
#define IECLASS_GADGETDOWN     7
#define IECLASS_GADGETUP       8
#define IECLASS_REQUESTER      9
#define IECLASS_MENULIST       10
#define IECLASS_CLOSEWINDOW    11
#define IECLASS_SIZEWINDOW     12
#define IECLASS_REFRESHWINDOW  13
#define IECLASS_NEWPREFS       14
#define IECLASS_DISKREMOVED    15
#define IECLASS_DISKINSERTED   16
#define IECLASS_ACTIVEWINDOW   17
#define IECLASS_INACTIVEWINDOW 18
#define IECLASS_NEWPOINTERPOS  19 /* (IEPointerPixel *) */
#define IECLASS_MENUHELP       20
#define IECLASS_CHANGEWINDOW   21
#define	 MD_READEVENT	   (CMD_NONSTD+0)

#endif

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"
#include "gfx_interface.h"

#define TEMPLATE    "NAME,STRING/M"
#define OPT_NAME    0
#define OPT_STRING  1
#define OPT_COUNT   2

APTR gfx_OpenView(APTR GfxBase, INT32, INT32, INT32);
#define WINDOWID uint32_t

WINDOWID strat_NewWindow(APTR StratBase, WINDOWID parent, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t bordersize, uint32_t background, uint32_t bordercolor);
void strat_MapWindow(APTR StratBase, WINDOWID wid);

static void draw3dbox(APTR GfxBase, APTR pb,  int x, int y, int w, int h, UINT32 crTop, UINT32 crBottom)
{
	UINT32 old = SetForegroundColor(pb, crTop);
	Line(pb, x, y+h-2, x, y+1,TRUE);		/* left*/
	Line(pb, x, y, x+w-2, y,TRUE);			/* top*/

	SetForegroundColor(pb, crBottom);
	Line(pb, x+w-1, y, x+w-1, y+h-2,TRUE);		/* right*/
	Line(pb, x+w-1, y+h-1, x, y+h-1,TRUE);		/* bottom*/
	SetForegroundColor(pb, old);
}

static void test_Arc(APTR GfxBase, APTR rp)
{
	int x = 40;
	int y = 40;
	int rx = 30;
	int ry = 30;
	int xoff = (rx + 10) * 2;

#if 1
	/* filled arc*/
	SetForegroundColor(rp, RGB(0,250,0));
	ArcAngle(rp, x, y, 3, 3, 0, 0, PIE);

	SetForegroundColor(rp, RGB(0,0,0));
	ArcAngle(rp, x, y, 3, 3, 0, 0, ARC);
	Point(rp, x, y);

#else
	GrSetGCForeground(gc, GREEN);
	GrArc(wid, gc, x, y, rx, ry, 0, -30, -30, 0, GR_PIE);
	GrArc(wid, gc, x+5, y, rx, ry, 30, 0, 0, -30, GR_PIE);
	GrArc(wid, gc, x, y+5, rx, ry, -30, 0, 0, 30, GR_PIE);
	GrArc(wid, gc, x+5, y+5, rx, ry, 0, 30, 30, 0, GR_PIE);
#endif
	/* outlined arc*/
//	DPrintF("outlined arc\n");
	SetForegroundColor(rp, RGB(0,250,0));
	x += xoff;
	Arc(rp, x, y, rx, ry, 0, -30, -30, 0, ARCOUTLINE);
	Arc(rp, x+5, y, rx, ry, 30, 0, 0, -30, ARCOUTLINE);
	Arc(rp, x, y+5, rx, ry, -30, 0, 0, 30, ARCOUTLINE);
	Arc(rp, x+5, y+5, rx, ry, 0, 30, 30, 0, ARCOUTLINE);

	/* arc only*/
//	DPrintF("arc only\n");
	x += xoff;
	Arc(rp, x, y, rx, ry, 0, -30, -30, 0, ARC);
	Arc(rp, x+5, y, rx, ry, 30, 0, 0, -30, ARC);
	Arc(rp, x, y+5, rx, ry, -30, 0, 0, 30, ARC);
	Arc(rp, x+5, y+5, rx, ry, 0, 30, 30, 0, ARC);
}

DOSCALL cmd_quit(APTR SysBase)
{
#if 0
	UINT32			opts[OPT_COUNT];
	struct RDargs	*rdargs;
	UINT8*			template = TEMPLATE;
//	UINT8			buffer[256];
#endif
	
	pDOSBase DOSBase = OpenLibrary("dos.library",0);
	uint32_t ts = TimeStamp();
	Printf("TimeStamp: %d\n",ts);
	
	struct MsgPort *mp	= CreateMsgPort(NULL);
	pIOStdReq		io	= CreateIORequest(mp, sizeof(struct IOStdReq));;

	uint8_t			*buffer = AllocVec(0x1000000, MEMF_FAST);
	if (!buffer) Printf("Error couldnt alloc Mem\n");

	int32_t ret = OpenDevice("virtio_blk.device", 0, io, 0);
	if (ret != 0)
	{
		Printf("OpenDevice virtio_blk.device failed!\n");
		return 0;
	}
	
//	uint8_t buffer[1024];
	
	io->io_Command = CMD_READ;
	io->io_Error = 0;
	io->io_Actual = 0;
	io->io_Data = buffer;
	io->io_Flags = 0;
	io->io_Length = 0x1000000 / 512;
	io->io_Offset = 0;
	Printf("Doio\n");
	DoIO( io );
	Printf("[%d]Doio end io_actual = %d(800) (Err:%d) \n", TimeStamp(), io->io_Actual, io->io_Error);

	FreeVec(buffer);
	CloseDevice(io);
	DeleteIORequest(io);
	DeleteMsgPort(mp);
	
#if 0
	INT32 xres = 1024;
	INT32 yres = 768;
	INT32 x=0, y=0;
	
	struct MsgPort *mp	= CreateMsgPort(NULL);
	pIOStdReq		io	= CreateIORequest(mp, sizeof(struct IOStdReq));;

	int32_t ret = OpenDevice("mouse.device", 0, io, 0);
	if (ret != 0)
	{
		Printf("OpenDevice mouseport.device failed!\n");
		return;
	}
	struct InputEvent ie;

	io->io_Command = MD_READEVENT; /* add a new request */
	io->io_Error = 0;
	io->io_Actual = 0;
	io->io_Data = &ie;
	io->io_Flags = 0;
	io->io_Length = sizeof(struct InputEvent);
	io->io_Message.mn_Node.ln_Name = (STRPTR)0;
	Printf("Doio\n");
	DoIO( io );
	if (io->io_Error > 0) Printf("Io Error [%d]\n", 0);
//	DPrintF("GotIO\n");

	int32_t	num = 0;

	for(;;)
	{
		pIOStdReq rcvd_io = io;
//		struct InputEvent *rcvd_ie = (struct InputEvent *)rcvd_io->io_Data;
		//DPrintF("Event Class %x (i= %x)[%d, %d](%x/%x)  --- ", rcvd_ie->ie_Class, rcvd_io->io_Message.mn_Node.ln_Name, rcvd_ie->ie_X, rcvd_ie->ie_Y, rcvd_ie, &ie);

		rcvd_io->io_Command = MD_READEVENT; /* add a new request */
		rcvd_io->io_Error = 0;
		rcvd_io->io_Actual = 0;
		rcvd_io->io_Data = &ie;
		rcvd_io->io_Flags = 0;
		rcvd_io->io_Length = sizeof(struct InputEvent);

		x+=ie.ie_X;
		y-=ie.ie_Y;
		if (x>xres) x=xres;
		if (x<0) x= 0;
		if (y>yres) y=yres;
		if (y<0) y= 0;
//		MoveCursor(x,y);
//KPrintF("x:%d, y:%d -",x, y);
//for(;;);
		num++;
		DoIO( io );
		if (num > 20) break;
	}
#endif
#if 0
	pFileHandle fh = Open("ram:Claus", MODE_NEWFILE);
	SetFileSize(fh, 4096, SEEK_CUR);
	Close(fh);
	fh = Open("ram:Claus", MODE_OLDFILE);
	int32_t pos = Seek(fh, 0, SEEK_END);
//	pos = Seek(fh, 0, SEEK_END);
	int32_t tel = Tell(fh);
	Printf("rel: %d, seek: %d\n", tel, pos);
	Close(fh);
#endif
#if 0
	APTR StratBase = OpenLibrary("superstratum.library", 0);
	if (StratBase)
	{
		uint32_t wid = strat_NewWindow(StratBase, 1, 0, 0, 100, 100, 3, RGB(255,255,255), RGB(255,0,0));
		uint32_t wid2 = strat_NewWindow(StratBase, 1, 0, 0, 100, 100, 3, RGB(0,255,0), RGB(0,0,255));
		strat_MapWindow(StratBase, wid);
		strat_MapWindow(StratBase, wid2);
		Printf("Received wid: %d\n", wid);
		ts = TimeStamp();
		while (ts+2>TimeStamp());
		strat_UnmapWindow(StratBase, wid);
		strat_MoveWindow(StratBase, wid, 150, 150);
		strat_MapWindow(StratBase, wid);
		while (ts+4>TimeStamp());
		strat_RaiseWindow(StratBase, wid);
		while (ts+6>TimeStamp());
		strat_ResizeWindow(StratBase, wid, 300, 300);
		uint32_t gc = strat_NewGC(StratBase);
		
		strat_SetGCForeground(StratBase, gc, RGB(0, 0, 255));
		strat_FillEllipse(StratBase, wid, gc, 80, 80, 50, 50);

		strat_SetGCForeground(StratBase, gc, RGB(0, 255, 0));
		strat_FillRect(StratBase, wid, gc, 10, 10, 100, 100);

//		strat_FillRect(StratBase, wid, gc, 20, 10, 100, 100);
//		strat_FillRect(StratBase, wid, gc, 30, 10, 100, 100);
//		strat_FillRect(StratBase, wid, gc, 40, 10, 100, 100);
//		strat_FillRect(StratBase, wid, gc, 50, 10, 100, 100);


		INT32 xres = 640;
		INT32 yres = 480;
		INT32 x=0, y=0;
		
		struct MsgPort *mp	= CreateMsgPort(NULL);
		pIOStdReq		io	= CreateIORequest(mp, sizeof(struct IOStdReq));;
	
		int32_t ret = OpenDevice("mouse.device", 0, io, 0);
		if (ret != 0)
		{
			Printf("OpenDevice mouseport.device failed!\n");
			return;
		}
		struct InputEvent ie;
	
		io->io_Command = MD_READEVENT; /* add a new request */
		io->io_Error = 0;
		io->io_Actual = 0;
		io->io_Data = &ie;
		io->io_Flags = 0;
		io->io_Length = sizeof(struct InputEvent);
		io->io_Message.mn_Node.ln_Name = (STRPTR)0;
		Printf("Doio\n");
		DoIO( io );
		if (io->io_Error > 0) Printf("Io Error [%d]\n", 0);
	//	DPrintF("GotIO\n");


		for(;;)
		{
			pIOStdReq rcvd_io = io;
	//		struct InputEvent *rcvd_ie = (struct InputEvent *)rcvd_io->io_Data;
			//DPrintF("Event Class %x (i= %x)[%d, %d](%x/%x)  --- ", rcvd_ie->ie_Class, rcvd_io->io_Message.mn_Node.ln_Name, rcvd_ie->ie_X, rcvd_ie->ie_Y, rcvd_ie, &ie);
	
			rcvd_io->io_Command = MD_READEVENT; /* add a new request */
			rcvd_io->io_Error = 0;
			rcvd_io->io_Actual = 0;
			rcvd_io->io_Data = &ie;
			rcvd_io->io_Flags = 0;
			rcvd_io->io_Length = sizeof(struct InputEvent);
	
			x+=ie.ie_X;
			y-=ie.ie_Y;
			if (x>xres) x=xres;
			if (x<0) x= 0;
			if (y>yres) y=yres;
			if (y<0) y= 0;
			//		MoveCursor(x,y);
			Printf("x:%d, y:%d -",(int16_t)ie.ie_X, (int16_t)ie.ie_Y);
			//for(;;);
			strat_MoveCursor(StratBase, x, y);
			DoIO( io );
		}


	} else
		Printf("Error opening strat\n");
#endif
#if 0
Cursor_T arrow = {	/* default arrow cursor*/
	16, 16,
	0,  0,
	RGB(255, 255, 255), RGB(0, 0, 0),
	{ 0xe000, 0x9800, 0x8600, 0x4180,
	  0x4060, 0x2018, 0x2004, 0x107c,
	  0x1020, 0x0910, 0x0988, 0x0544,
	  0x0522, 0x0211, 0x000a, 0x0004 },
	{ 0xe000, 0xf800, 0xfe00, 0x7f80,
	  0x7fe0, 0x3ff8, 0x3ffc, 0x1ffc,
	  0x1fe0, 0x0ff0, 0x0ff8, 0x077c,
	  0x073e, 0x021f, 0x000e, 0x0004 }
};
	Printf("Starting GfxTest\n");
	APTR GfxBase = OpenLibrary("gfx.library",0);
	if (GfxBase)
	{
		APTR pb = gfx_OpenView(GfxBase,640,480,32);

		SetForegroundColor(pb, RGB(150,150,150));
		RectFill(pb, 0, 0, 1024, 768);

		MoveCursor(pb, 0, 0);
		SetCursor(pb, &arrow);
		ShowCursor(pb);
#if 0		
		draw3dbox(GfxBase, pb, 50, 50, 200, 200, RGB(162, 141, 104), RGB(234, 230, 221));
		draw3dbox(GfxBase, pb, 51, 51, 198, 198, RGB(  0,   0,   0), RGB(213, 204, 187));
		test_Arc(GfxBase, pb);
		Printf("Arc Done\n");
		pFont font = CreateFont(pb, FONT_SYSTEM_VAR, 0, 0, NULL);
		Printf("CF Done\n");
		BOOL old = SetUseBackground(pb, FALSE);
		Printf("SUB Done\n");
		Text(pb, font, 50, 200, "Hello World", -1, TF_BASELINE);
		Printf("Text 1 Done\n");
		Text(pb, font, 50, 300, "Hello World", -1, TF_TOP);
		Printf("Text 1 Done\n");
		Text(pb, font, 50, 400, "Hello World", -1, TF_BOTTOM);
		Printf("Text 1 Done\n");
		SetUseBackground(pb, old);
#endif
		Printf("Done\n");
		CloseLibrary(GfxBase);
	} else
		Printf("Failed to open Lib.!\n");
#endif
	
#if 0
	pFileHandle fh = Open("ram:testdoc", MODE_NEWFILE);
	if (fh)
	{
		UINT8 buffer[4096];
		for(int i=0; i<4096; i++)
		{
			buffer[i] = 't';
		}
		Printf("Writing now\n");
		FWrite(fh, buffer, 4096, 1);
		Printf("Finished\n");
		Close(fh);
	}
#endif
#if 0
	pFileHandle fh = Open("testdoc.txt", MODE_OLDFILE);
	Printf("Opening file: testdoc.txt\n");
	if (fh)
	{
		UINT8 buffer[128];
		INT32 cnt = FRead(fh, buffer, 1, 128);
		Printf("Read: %d chars\n", cnt);
		buffer[cnt+1] = '\0'; 
		Printf("Read : %s\n", buffer);
		Close(fh);
	} else
		Printf("Couldnt open file: testdoc.txt\n");
#endif	
#if 0
	MemSet((char*)opts, 0, sizeof(opts));

	rdargs = ReadArgs(template, opts);
	if (rdargs == NULL)
	{
		KPrintF("Error SET: ReadArgs\n");
		SetIoErr(ERROR_BAD_ARGUMENTS);
		PrintFault(IoErr(), NULL);
		return RETURN_ERROR;
	}
	Flush(Output());
	if (opts[OPT_NAME])
		VPrintf("OPT_NAME : %s\n", opts[OPT_NAME]);
//	if (opts[OPT_STRING])
//		VPrintf("OPT_STRING : %s\n", opts[OPT_STRING]);
	KPrintF("Success\n");
//	Fault(STR_CANT_SET, NULL, buffer, 36);
//	VPrintf(buffer, opts[OPT_NAME]);
	//Flush(Output());
	
	FreeArgs(rdargs);
#endif

	CloseLibrary(DOSBase);
	return 0;
}

