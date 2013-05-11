#include "types.h"
#include "list.h"
#include "ports.h"
#include "sysbase.h"
#include "device.h"
#include "io.h"

#include "exec_funcs.h"

void lib_AbortIO(SysBase *SysBase, struct IORequest *iORequest)
{
	(((void(*)(struct Device *, struct IORequest *)) _GETVECADDR(iORequest->io_Device,6))(iORequest->io_Device, iORequest));
}

struct IORequest *lib_CreateIORequest(SysBase *SysBase, struct MsgPort *ioReplyPort, UINT32 size)
{
	struct IORequest *ret = NULL;

	if(NULL == ioReplyPort) return NULL;
	ret = (struct IORequest *)AllocVec(size,MEMF_FAST);
  	if (NULL != ret)
	{
    	ret->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    	ret->io_Message.mn_ReplyPort=ioReplyPort;
    	ret->io_Message.mn_Length=size;
	}
	return ret;
}

void lib_DeleteIORequest(SysBase *SysBase, struct IORequest *iorequest)
{
  if(iorequest!=NULL)
  {
    iorequest->io_Message.mn_Node.ln_Succ = (struct Node *)0;
    iorequest->io_Device                  = (struct Device *)0;
    FreeVec(iorequest);
  }
}

struct IORequest *lib_CheckIO(SysBase *SysBase,struct IORequest *io)
{
	if ( !(io->io_Flags & IOF_QUICK) ) 
	{
		if (io->io_Message.mn_Node.ln_Type != NT_REPLYMSG) return NULL;
	}
	return io;
}

INT32 lib_DoIO(SysBase *SysBase, struct IORequest *iORequest)
{
	if (iORequest == NULL) return -1;
	if (iORequest->io_Device == NULL) return -1;
	iORequest->io_Flags = IOF_QUICK;
	(((void(*)(struct Device *, struct IORequest *)) _GETVECADDR(iORequest->io_Device,5))(iORequest->io_Device, iORequest));
	WaitIO(iORequest);
	return (INT32)iORequest->io_Error;
}

void lib_SendIO(SysBase *SysBase, struct IORequest *iORequest)
{
	iORequest->io_Flags=0;
	(((void(*)(struct Device *, struct IORequest *)) _GETVECADDR(iORequest->io_Device,5))(iORequest->io_Device, iORequest));
}

INT32 lib_WaitIO(SysBase *SysBase, struct IORequest *io)
{
	if (!(io->io_Flags&IOF_QUICK))
	{
		UINT32 ipl = Disable();
		while(io->io_Message.mn_Node.ln_Type != NT_REPLYMSG)
		{
			Wait(1<<io->io_Message.mn_ReplyPort->mp_SigBit);
		}
		Remove(&io->io_Message.mn_Node);
		Enable(ipl);
	}
	return (INT32)io->io_Error;
}
