/**
 * @file devices.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"

SysCall lib_AddDevice(SysBase *SysBase, struct Device *device)
{
	device->dev_Node.ln_Type = NT_DEVICE;
	device->dev_Flags|=LIBF_CHANGED;

	//SumLibrary(&device);
  	Forbid();
  	Enqueue(&SysBase->DevList,&device->dev_Node);
  	Permit();
	return OK;
}

SysCall lib_RemDevice(SysBase *SysBase, struct Device *device)
{
	SysCall ret;

	if (device == NULL) return SYSERR;
	Forbid();
  	Remove(&device->dev_Node);
//	ret = (((UINT32(*)(struct Device *)) _GETVECADDR(device, DEV_EXPUNGE))(device));
	ret = _LIBCALL(device, DEV_EXPUNGE, uint32_t, (pDevice), (device));
	Permit();
	return ret;
}

SysCall lib_CloseDevice(SysBase *SysBase, pIOStdReq iORequest)
{
	if(NULL != iORequest->io_Device)
	{
		Forbid();
		_LIBCALL(iORequest->io_Device, DEV_CLOSE, void, (struct Device *, pIOStdReq), (iORequest->io_Device, iORequest));
		//(((void(*)(struct Device *, pIOStdReq))_GETVECADDR(iORequest->io_Device, DEV_CLOSE))(iORequest->io_Device, iORequest));
		iORequest->io_Device = NULL;
		Permit();
		return OK;
	}
	return SYSERR;
}

SysCall lib_OpenDevice(struct SysBase *SysBase, STRPTR devName, UINT32 unitNum, pIOStdReq iORequest, UINT32 flags)
{
	pDevice device;
	SysCall ret = IOERR_OPENFAIL; //Same as SYSERR

	Forbid();
	device = (pDevice)FindName(&SysBase->DevList, devName);
	Permit();

	if (NULL != device)
	{
    	iORequest->io_Error  = 0;
    	iORequest->io_Device = device;
    	iORequest->io_Unit   = NULL;

    	Forbid();
		if (NULL == _LIBCALL(device, DEV_OPEN, APTR, (struct Device *, pIOStdReq, UINT32, UINT32), (device, iORequest, unitNum, flags)))
//    	if (NULL == (((APTR(*)(struct Device *, pIOStdReq, UINT32, UINT32)) _GETVECADDR(device, DEV_OPEN))(device, iORequest, unitNum, flags)))
		{
			Permit();
			return ret;
		}
		Permit();
    	ret = iORequest->io_Error;
    	if (ret < 0) iORequest->io_Device = NULL;
	}
	return ret;
}

pIOStdReq lib_CreateIORequest(SysBase *SysBase, pMsgPort ioReplyPort, UINT32 size)
{
	pIOStdReq ret = NULL;

	if (NULL == ioReplyPort) return NULL;

	if (size == 0)
	{
		size = sizeof(IOStdReq);
	}
	ret = (pIOStdReq) AllocVec(size, MEMF_FAST);	
	
  	if (NULL != ret)
	{
    	ret->io_Message.mn_Node.ln_Type = NT_MESSAGE;
    	ret->io_Message.mn_ReplyPort	= ioReplyPort;
    	ret->io_Message.mn_Length		= size;
	}
	return ret;
}

SysCall lib_DeleteIORequest(SysBase *SysBase, pIOStdReq iorequest)
{
	if(iorequest != NULL)
	{
		iorequest->io_Message.mn_Node.ln_Succ = NULL;
		iorequest->io_Device                  = NULL;
		FreeVec(iorequest);
		return OK;
	}
	return SYSERR;
}

SysCall lib_AbortIO(SysBase *SysBase, pIOStdReq iORequest)
{
	_LIBCALL(iORequest->io_Device, DEV_ABORTIO, void, (struct Device *, pIOStdReq), (iORequest->io_Device, iORequest));
	//(((void(*)(struct Device *, pIOStdReq)) _GETVECADDR(iORequest->io_Device, DEV_ABORTIO))(iORequest->io_Device, iORequest));
	return (SysCall)iORequest->io_Error;
}

pIOStdReq lib_CheckIO(SysBase *SysBase, pIOStdReq iORequest)
{
	if(iORequest->io_Message.mn_Node.ln_Type == NT_MESSAGE && iORequest->io_Flags&IOF_QUICK) 
	{
		return NULL;
	}
	else return iORequest;
}

SysCall lib_DoIO(SysBase *SysBase, pIOStdReq iORequest)
{
	if (iORequest == NULL) return SYSERR;
	if (iORequest->io_Device == NULL) 
	{
		iORequest->io_Error = IOERR_OPENFAIL;
		return SYSERR;
	}
	iORequest->io_Flags = IOF_QUICK;
	iORequest->io_Message.mn_Node.ln_Type = NT_MESSAGE;

	_LIBCALL(iORequest->io_Device, DEV_BEGINIO, void, (struct Device *, pIOStdReq), (iORequest->io_Device, iORequest));
//	(((void(*)(pDevice, pIOStdReq)) _GETVECADDR(iORequest->io_Device, DEV_BEGINIO))(iORequest->io_Device, iORequest));
	 if(!(iORequest->io_Flags & IOF_QUICK)) WaitIO(iORequest);
	//if (iORequest->io_Error != 0) KPrintF("return doio (error: %d)\n", iORequest->io_Error);
	return (SysCall)iORequest->io_Error;
}

SysCall lib_SendIO(SysBase *SysBase, pIOStdReq io)
{
	if (io == NULL) 
	{
		KPrintF("SENDIO: Error io = NULL\n");		
		return SYSERR;
	}
	if (io->io_Device == NULL) 
	{
		KPrintF("SENDIO: Error Devce = NULL\n");
		io->io_Error = IOERR_OPENFAIL;
		return SYSERR;
	}
	IRQMask im = Disable();
	io->io_Message.mn_Node.ln_Type = NT_MESSAGE;
	//KPrintF("Sendio: ReplyPort: %x\n",io->io_Message.mn_ReplyPort);
	Restore(im);
	_LIBCALL(io->io_Device, DEV_BEGINIO, void, (struct Device *, pIOStdReq), (io->io_Device, io));
	//(((void(*)(pDevice, pIOStdReq)) _GETVECADDR(io->io_Device, DEV_BEGINIO))(io->io_Device, io));
	return OK;
}

SysCall lib_WaitIO(SysBase *SysBase, pIOStdReq iORequest)
{
    while(!(iORequest->io_Flags&IOF_QUICK) && iORequest->io_Message.mn_Node.ln_Type == NT_MESSAGE)
	{
		Signal sig = 1<<iORequest->io_Message.mn_ReplyPort->mp_SigBit;
//		KPrintF("WIO -> Waitsignal\n");
		WaitSignal(sig);
//		KPrintF("WIO <- Waitsignal\n");
	}

	if(iORequest->io_Message.mn_Node.ln_Type == NT_REPLYMSG)
	{
		IRQMask ipl = Disable();
		Remove(&iORequest->io_Message.mn_Node);
		Restore(ipl);
	}
//	KPrintF("return\n");
	return (SysCall)iORequest->io_Error;
}



