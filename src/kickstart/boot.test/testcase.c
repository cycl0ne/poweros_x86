#include "types.h"
#include "sysbase.h"
#include "resident.h"
#include "timer.h"
#include "exec_funcs.h"
#include "mouseport.h"
#include "keyboard.h"
#include "inputevent.h"

// This is a Testsuite for implementations on the OS.
// Since we boot out of the ROM, we need an Resident Structure

static struct TestBase *test_Init(struct TestBase *TestBase, UINT32 *segList, struct SysBase *SysBase);
static void test_TestTask(APTR data, struct SysBase *SysBase);

struct TestBase
{
	SysBase	*SysBase;
	Task *WorkerTask;
};

static const APTR InitTab[4]=
{
	(APTR)sizeof(struct TestBase),
	(APTR)NULL,  // Array of function (Library/Device)
	(APTR)NULL,
	(APTR)test_Init
};


static const char name[] = "test";
static const char version[] = "test 0.1\n";
static const char EndResident;
#define DEVICE_VERSION 0
#define DEVICE_REVISION 1

static const struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_COLDSTART|RTF_TESTCASE,
	DEVICE_VERSION,
	NT_DEVICE,
	-100,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

static struct TestBase *test_Init(struct TestBase *TestBase, UINT32 *segList, struct SysBase *SysBase)
{
	TestBase->SysBase = SysBase;
	// Only initialise here, dont do long stuff, Multitasking is enabled. But we are running here with prio 100
	// For this we initialise a worker Task with Prio 0 
	TestBase->WorkerTask = TaskCreate("TestSuite", test_TestTask, SysBase, 4096*4, 0); //4kb Stack should be enough
	return TestBase;
}

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

struct InputEvent g_ie;

struct TestStruct {
	struct MsgPort *mp[10];
	struct IOStdReq *io[10];
	struct InputEvent ie[10];
};

#include "alerts.h"

static void test_mouse(SysBase *SysBase)
{
	struct TestStruct *ts = AllocVec(sizeof(struct TestStruct), MEMF_CLEAR);

	if (ts != NULL)
	{
		INT32 ret = 0;
		struct MsgPort *mp= CreateMsgPort();
		for (int i=0 ; i<10; i++)
		{
			ts->mp[i] = mp;
			// No need for 10 ports ts->mp[i] = CreateMsgPort();
			ts->io[i] = CreateIORequest(mp, sizeof(struct IOStdReq));
			if ((ts->mp[i] == NULL)||(ts->io[i] == NULL))
			{
				DPrintF("Error allocating Port/Message at : %d\n", i);
				break;
			}
			ret = OpenDevice("mouseport.device", 0, (struct IORequest *)ts->io[i], 0);
			if (ret != 0) 
			{
				DPrintF("OpenDevice mouseport.device failed!\n");
				break;
			}
			ts->io[i]->io_Command = MD_READEVENT; /* add a new request */
			ts->io[i]->io_Error = 0;
			ts->io[i]->io_Actual = 0;	
			ts->io[i]->io_Data = &ts->ie[i];
			ts->io[i]->io_Flags = 0;
			ts->io[i]->io_Length = sizeof(struct InputEvent);
			ts->io[i]->io_Message.mn_Node.ln_Name = (STRPTR)i;
			SendIO((struct IORequest *) ts->io[i] );
			//DPrintF("io->flags = %b\n", ts->io[i]->io_Flags);
			if (ts->io[i]->io_Error > 0) DPrintF("Io Error [%d]\n", i);
		}
		
		for(;;)
		{
			Wait(1<<mp->mp_SigBit);
			while (!IsMsgPortEmpty(mp))
			{
				//DPrintF("[ID:%x]", SysBase->IDNestCnt);
				struct IOStdReq *rcvd_io = (struct IOStdReq *)GetMsg(mp);
				struct InputEvent *rcvd_ie = (struct InputEvent *)rcvd_io->io_Data;
				DPrintF("Event Class %x (i= %x)\n", rcvd_ie->ie_Class, rcvd_io->io_Message.mn_Node.ln_Name);
			
				rcvd_io->io_Command = MD_READEVENT; /* add a new request */
				rcvd_io->io_Error = 0;
				rcvd_io->io_Actual = 0;	
				//rcvd_io->io_Data = &ts->ie[i];
				rcvd_io->io_Flags = 0;
				rcvd_io->io_Length = sizeof(struct InputEvent);

				SendIO((struct IORequest *) rcvd_io );
			}
		}
		
		//DPrintF("EventClass [%x]\n", ts->io[i].ie_Class);
	}
}

static void test_keyboard(SysBase *SysBase)
{
	struct TestStruct *ts = AllocVec(sizeof(struct TestStruct), MEMF_CLEAR);

	if (ts != NULL)
	{
		INT32 ret = 0;
		struct MsgPort *mp= CreateMsgPort();
		for (int i=0 ; i<10; i++)
		{
			ts->mp[i] = mp;
			ts->io[i] = CreateIORequest(mp, sizeof(struct IOStdReq));
			if ((ts->mp[i] == NULL)||(ts->io[i] == NULL))
			{
				DPrintF("Error allocating Port/Message at : %d\n", i);
				for(;;);
				break;
			}
			ret = OpenDevice("keyboard.device", 0, (struct IORequest *)ts->io[i], 0);
			if (ret != 0) 
			{
				DPrintF("OpenDevice keyboard.device failed!\n");
				for(;;);
				break;
			}
			ts->io[i]->io_Command = KBD_READEVENT; /* add a new request */
			ts->io[i]->io_Error = 0;
			ts->io[i]->io_Actual = 0;	
			ts->io[i]->io_Data = &ts->ie[i];
			ts->io[i]->io_Flags = 0;
			ts->io[i]->io_Length = sizeof(struct InputEvent);
			ts->io[i]->io_Message.mn_Node.ln_Name = (STRPTR)i;
			SendIO((struct IORequest *) ts->io[i] );
			//DPrintF("io->flags = %b\n", ts->io[i]->io_Flags);
			if (ts->io[i]->io_Error > 0) 
			{
				DPrintF("Io Error [%d]\n", i);
			}
		}

		for(;;)
		{
			Wait(1<<mp->mp_SigBit);
			while (!IsMsgPortEmpty(mp))
			{
				//DPrintF("[ID:%x]", SysBase->IDNestCnt);
				struct IOStdReq *rcvd_io = (struct IOStdReq *)GetMsg(mp);
				struct InputEvent *rcvd_ie = (struct InputEvent *)rcvd_io->io_Data;
				DPrintF("Event Class %x (i= %x)\n", rcvd_ie->ie_Class, rcvd_io->io_Message.mn_Node.ln_Name);
			
				rcvd_io->io_Command = KBD_READEVENT; /* add a new request */
				rcvd_io->io_Error = 0;
				rcvd_io->io_Actual = 0;	
				//rcvd_io->io_Data = &ts->ie[i];
				rcvd_io->io_Flags = 0;
				rcvd_io->io_Length = sizeof(struct InputEvent);

				SendIO((struct IORequest *) rcvd_io );
			}
		}
		
		//DPrintF("EventClass [%x]\n", ts->io[i].ie_Class);
	}
}

static void test_AlertTest(SysBase *SysBase)
{
	Alert(AN_MemCorrupt, "Memory defect -> Just kidding\n");
}

static void test_Srini(SysBase *SysBase)
{
// Small Tutorial for Srini on opening a device. 
// Example: timer.device
// We will create a wait(seconds); Function

	// We need a proper MessagePort to get Messages
	struct MsgPort *mp= CreateMsgPort();
	struct TimeRequest *io;
	// Now we need an IO Request Structure
	io = CreateIORequest(mp, sizeof(struct TimeRequest));
	if (io == NULL) 
	{
		DPrintF("Couldnt create IORequest (no Memory?)\n");
		DeleteMsgPort(mp);		
	}
	//UNIT_VBLANK works at 1/100sec and give him the IO
	INT32 ret = OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)io, 0);
	if (ret != 0)
	{
		DPrintF("OpenDevice Timer failed!\n");
		// Please tidy up, close all !
		DeleteIORequest((struct IORequest *)io);
		DeleteMsgPort(mp);
		return;
	}
	
	// Now we have everything setup.. Lets Proceed
	io->tr_node.io_Command = TR_ADDREQUEST; /* add a new timer request */
	io->tr_time.tv_micro = 0;
	io->tr_time.tv_secs = 5;

	// post request to the timer -- will go to sleep till done
	DPrintF("We will go 5 Seconds to sleep\n");
	DoIO((struct IORequest *) io );
	DPrintF("Return after 5 Seconds\n");
}

static void test_printit(INT32 chr, APTR SysBase)
{
	RawPutChar(chr);
}

static void test_RawIO(SysBase *SysBase)
{
	RawIOInit();

	// Simple printf("hello...
	RawDoFmt("HELLO WORLD\n", NULL, test_printit, SysBase);

	UINT32 var[2] = {0x32, 0x10};

	// we emulate a command sequence printf("...", var1, var2);
	RawDoFmt("[%b] [%x]\n", (va_list) var, test_printit, SysBase);

	// single char output
	RawPutChar('H');
	RawPutChar('E');
	RawPutChar('L');
	RawPutChar('O');
	RawPutChar('\n');
	
	// Now test some input ->
	UINT8 chr = RawMayGetChar();
	while(chr != 'q') {
		// we only print chars received a-z and return
		if ((chr > 'a' && chr <'z') || chr=='\r') RawPutChar(chr);
		chr = RawMayGetChar();
	}
	Alert(AN_MemCorrupt, "End of Test RawIO\n");
}

BOOL VmwSetVideoMode(UINT32 Width, UINT32 Height, UINT32 Bpp, SysBase *SysBase);

#include "vgagfx.h"
#include "vmware.h"

static inline void
memset32(void *dest, UINT32 value, UINT32 size)
{
   asm volatile ("cld; rep stosl" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static void test_TestTask(APTR data, struct SysBase *SysBase) 
{
	DPrintF("Binary  Output: %b\n", 0x79);
	DPrintF("Hex     Output: %x\n", 0x79);
	DPrintF("Decimal Output: %d\n", 0x79);

	VgaGfxBase *VgaGfxBase = OpenLibrary("vgagfx.library", 0);
	if (!VgaGfxBase) DPrintF("Failed to open library\n");
	
	SVGA_SetDisplayMode(VgaGfxBase, 640, 480, 32);

	SVGA_DrawFillRect32(VgaGfxBase,   0,   0, 640,  10, 0xffff0000, 0);
	SVGA_DrawFillRect32(VgaGfxBase,   0, 470, 640, 480, 0xffff0000, 0);
	SVGA_DrawFillRect32(VgaGfxBase,   0,   0,  10, 480, 0xffff0000, 0);
	SVGA_DrawFillRect32(VgaGfxBase, 630,   0, 640, 480, 0xffff0000, 0);
	
//	SVGA_CopyRect(VgaGfxBase, 0, 0, 100, 100, 50, 50);
//	SVGA_FillRect(VgaGfxBase, 0xff00ff00, 5, 5, 30, 240);

	SVGA_FifoUpdateFullscreen(VgaGfxBase);

//	SVGA_DrawHorzLine32(VgaGfxBase, 0, 640, 0, 0xffff0000, 0);
//	SVGA_DrawHorzLine32(VgaGfxBase, 0, 640, 479, 0xffff0000, 0);
	
//	for(int i= 0xFF000000;i<0xFFFFFFFF; i++) {	
//		memset32(VgaGfxBase->fbDma, i, 640*20);
		//SVGA_FifoUpdateFullscreen(VgaGfxBase);
//	}
	
//	SVGA_FillRect(VgaGfxBase, i, 5, 5, 630, 240);

//	SVGA_FillRect(VgaGfxBase, 0xFFFF0000, 0, 0, 640, 10);
//	SVGA_FillRect(VgaGfxBase, 0xFFFF0000, 0, 470, 640, 10);
//	SVGA_FillRect(VgaGfxBase, 0xFFFF0000, 0, 0, 10, 480);
//	SVGA_FillRect(VgaGfxBase, 0xFFFF0000, 630, 0, 10, 480);

//	for(int i= 0xFF000000;i<0xFFFFFFFF; i++) SVGA_FillRect(VgaGfxBase, i, 5, 5, 630, 240);

//	test_mouse(SysBase);
//	test_keyboard(SysBase);
//	test_AlertTest(SysBase);
//	test_RawIO(SysBase);
//	VmwSetVideoMode(800, 600, 32, SysBase);

//	test_Srini(SysBase);
	DPrintF("[TESTTASK] Finished, we are leaving... bye bye... till next reboot\n");
}



