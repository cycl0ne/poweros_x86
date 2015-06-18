/**
 * @file dospackets.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

DOSCALL dos_SendPkt(pDOSBase DOSBase, pDosPacket dp, pMsgPort port, pMsgPort replyPort)
{
	if (dp == NULL)
	{
		KPrintF("[dos]SendPkt: dp = NULL\n");
		SetIoErr(ERROR_NO_PACKET);
		return DOSCMD_FAIL;
	}
	dp->dp_Message.mn_ReplyPort = replyPort;	
	SetIoErr(ERROR_SYSTEMCALL);
	return PutMsg(port, &dp->dp_Message);
}

DOSCALL dos_ReplyPkt(pDOSBase DOSBase, pDosPacket dp, INT32 res1, INT32 res2)
{
	if (dp == NULL)
	{
		KPrintF("[dos]ReplyPkt: dp = NULL\n");
		SetIoErr(ERROR_NO_PACKET);
		return DOSCMD_FAIL;
	}

	pProcess me	= FindProcess(NULL);
	if (me == NULL)
	{
		KPrintF("[dos]ReplyPkt: FindProcess = NULL\n");
		return DOSCMD_FAIL;
	}
	
	dp->dp_Res1 = res1;
	dp->dp_Res2 = res2;
	//KPrintF("[ReplyMsg] - res1: %x, res2: %x\n", res1, res2);

	pMsgPort port	= dp->dp_Message.mn_ReplyPort; // The ReplyPort we are sending a Message
	//KPrintF("[ReplyMsg] ReplyPort: %x -[%x]\n", port, &dp->dp_Message);
	dp->dp_Message.mn_ReplyPort = &me->pr_MsgPort;	// Our Port if he "ReplyMsg()"
	
	//SetIoErr(ERROR_SYSTEMCALL);
	//KPrintF("[ReplyMsg] ReplyPort: %x -[%x]\n", port, &dp->dp_Message);
	return PutMsg(port, &dp->dp_Message);
}

DOSCALL dos_AbortPkt(pDOSBase DOSBase, pDosPacket dp, pMsgPort port)
{
	SetIoErr(ERROR_SYSTEMCALL);
	return DOSCMD_FAIL;
}

pDosPacket dos_WaitPkt(pDOSBase DOSBase)
{
	pMessage	msg = NULL;
	pMsgPort	port= NULL;
	pProcess 	me	= FindProcess(NULL);
	if (me == NULL)
	{
		KPrintF("[dos]WaitPkt: FindProcess = NULL\n");
		return DOSCMD_NULL;
	}

	port = &me->pr_MsgPort;
	while (WaitPort(port) == NULL);
	//KPrintF("WaitPkt: GetMsg\n");
	msg = GetMsg(port);
	return (pDosPacket)msg;
}

DOSIO dos_DoPkt(pDOSBase DOSBase, pMsgPort port, INT32 action, INT32 arg1, INT32 arg2, INT32 arg3, INT32 arg4, INT32 arg5)
{
	DosPacket	dp;
	pMsgPort	replyPort;
	pProcess 	me	= FindProcess(NULL);

	if (me == NULL)
	{
		// Create a MsgPort
		replyPort = CreateMsgPort(NULL);
		KPrintF("[dos]DoPkt: FindProcess = NULL Created Port: %x\n", replyPort);
		//return DOSIO_FALSE;
	} else
		replyPort		= &me->pr_MsgPort;

	dp.dp_Action	= action;
	dp.dp_Arg1		= arg1;
	dp.dp_Arg2		= arg2;
	dp.dp_Arg3		= arg3;
	dp.dp_Arg4		= arg4;
	dp.dp_Arg5		= arg5;
	dp.dp_Arg6		= 0;
	dp.dp_Arg7		= 0;
	dp.dp_Arg8		= 0;
	dp.dp_Res1		= 0;
	dp.dp_Res2		= 0;
	
	if (SendPkt(&dp, port, replyPort) == DOSCMD_SUCCESS)
	{
		//KPrintF("DoPkt: Send Pkt: [%x]\n", &dp);
		//KPrintF("[WP]Process [%s] MsgPort: %x\n", me->pr_Task.tcb_Node.ln_Name, replyPort);
		while (WaitPort(replyPort) == NULL);
		pMessage msg = GetMsg(replyPort);
		//KPrintF("DoPkt: Got Message %x\n", msg);
		if (msg != (pMessage)&dp || msg == NULL) 
		{
			KPrintF("Wrong PAcket: PANIC ASYNC!\n");
			SetIoErr(ERROR_ASYNC_PACKET);
			return DOSIO_FALSE;
		}
		
		if (me == NULL) DeleteMsgPort(replyPort);
		else 
			me->pr_Result2 = dp.dp_Res2;
		return (DOSIO)dp.dp_Res1;
	} else
		KPrintF("DoPkt failed\n");
	return DOSIO_FALSE;
}

pDosPacket dos_CreatePkt(pDOSBase DOSBase)
{
	pDosPacket ret = AllocVec(sizeof (DosPacket), MEMF_FAST|MEMF_CLEAR);
	SetIoErr(ERROR_NO_FREE_STORE);

	if (ret != NULL)
	{
		ret->dp_Message.mn_ReplyPort= NULL;	// we did a MEMF_CLEAR, so just to be sure
		ret->dp_Message.mn_Length	= sizeof(DosPacket);
	}
	return ret;
}

DOSCALL dos_DeletePkt(pDOSBase DOSBase, pDosPacket dp)
{
	if (dp == NULL) 
	{
		SetIoErr(ERROR_NO_PACKET);
		return DOSCMD_FAIL;
	}
	FreeVec(dp);
	return DOSCMD_SUCCESS;
}

