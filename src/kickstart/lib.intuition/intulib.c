#include "intuitionbase.h"
#include "input.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

struct InputEvent *IntuitionInput(struct InputEvent *ie);

static char IDName[] = "input.device";

APTR intu_OpenLib(IntuitionBase *IBase)
{
	if (IBase->ib_Library.lib_OpenCnt) return IBase;
	IBase->ib_Library.lib_OpenCnt++;
	
	if (OpenDevice(IDName, 0, (struct IORequest *)&IBase->ib_IIOR, 0) == 0)
    {
		IBase->ib_IIOR.io_Message.mn_ReplyPort = NULL;
		IBase->ib_IIOR.io_Command = IND_ADDHANDLER;
		IBase->ib_IIOR.io_Data = &IBase->ib_InputInterrupt;

		IBase->ib_InputInterrupt.is_Node.ln_Pri = 50;
		IBase->ib_InputInterrupt.is_Node.ln_Name  = LIBRARY_NAME
		IBase->ib_InputInterrupt.is_Code = (APTR)IntuitionInput;
		IBase->ib_InputInterrupt.is_Data = (APTR) IBase;

		SendIO((struct IORequest *)&IBase->ib_IIOR);
		IBase->ib_InputDeviceTask = (struct Task *) FindTask(IDName);
		DPrintF("InputDevTask: %x\n", IBase->ib_InputDeviceTask);
    }	
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(IBase);
}

APTR intu_CloseLib(IntuitionBase *IBase)
{
	IBase->ib_Library.lib_OpenCnt--;
	return NULL;
}

APTR intu_ExpungeLib(IntuitionBase *IBase)
{
	return NULL;
}

APTR intu_ExtFuncLib(IntuitionBase *IBase)
{
	return NULL;
}
