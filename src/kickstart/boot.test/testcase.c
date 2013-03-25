#include "types.h"
#include "sysbase.h"
#include "resident.h"
#include "timer.h"
#include "exec_funcs.h"
#include "mouseport.h"
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

static void test_TestTask(APTR data, struct SysBase *SysBase) 
{
	DPrintF("Binary  Output: %b\n", 0x79);
	DPrintF("Hex     Output: %x\n", 0x79);
	DPrintF("Decimal Output: %d\n", 0x79);

	DPrintF("Load Test Mouse\n");
	test_mouse(SysBase);

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

	DPrintF("[TESTTASK] Finished, we are leaving... bye bye... till next reboot\n");
}



