/**
 * @file ports.c
 * In this file all port and message handling is done. Lower than this is signal
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "execbase_private.h"
#include "exec_interface.h"


/**
 * @ingroup ports
 *
 * Add port to the public port list
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param pMsgPort
 * 		pointer to a fully initialised message port (see CreateMsgPort)
 * @return
 *		OK or SYSERR if port couldnt be added
*/
SysCall lib_AddPort(SysBase *SysBase, pMsgPort msgPort)
{
	if (msgPort == NULL) return SYSERR;
	if (msgPort->mp_Node.ln_Type != NT_MSGPORT) return SYSERR;

	Forbid();
	Enqueue(&SysBase->PortList, &msgPort->mp_Node);
	Permit();
	return OK;
}

/**
 * @ingroup ports
 *
 * Find port in the public port list
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		pointer to a cstring
 * @return
 *		pMsgPort to the found Port or NULL if none found
*/
pMsgPort lib_FindPort(SysBase *SysBase, STRPTR name)
{
   return (struct MsgPort *)FindName(&SysBase->PortList, name);
}

/**
 * @ingroup ports
 *
 * Delete port in the public port list
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a MsgPort 
 * @return
 *		SYSERR or OK
*/
SysCall lib_RemPort(SysBase *SysBase, pMsgPort msgPort)
{
	if (msgPort == NULL) return SYSERR;
	if (msgPort->mp_Node.ln_Type != NT_MSGPORT) return SYSERR;

	Forbid();
	Remove(&msgPort->mp_Node);
	Permit();
	return OK;
}

/**
 * @ingroup ports
 *
 * Waits for a Message on a given Port
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a MsgPort 
 * @return
 *		pMessage which arrived
*/
pMessage lib_WaitPort(SysBase *SysBase, pMsgPort msgPort)
{
	if (msgPort == NULL) return (pMessage)NULL;

	while (IsListEmpty(&msgPort->mp_MsgList))
  	{
		//KPrintF("WaitPort: Port empty\n");
  		WaitSignal(1 << msgPort->mp_SigBit);
  	}
  	return (struct Message *) msgPort->mp_MsgList.lh_Head;
}

/**
 * @ingroup ports
 *
 * Create a message port
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param port
 *		pointer to a port or NULL
 * @return
 *		pointer of a MsgPort or NULL on fail
*/
pMsgPort lib_CreateMsgPort(SysBase *SysBase, pMsgPort port)
{
	pMsgPort ret = port;

	if (port == NULL)
	{
		ret = (pMsgPort) AllocVec(sizeof(struct MsgPort), MEMF_PUBLIC|MEMF_CLEAR);
	}

   	if (NULL != ret)
   	{
     	INT8 sb;
     	sb = AllocSignal(-1);
     	if (sb != -1)
     	{
       		ret->mp_SigBit = sb;
       		ret->mp_Flags  = PA_SIGNAL;
       		ret->mp_Node.ln_Type = NT_MSGPORT;
       		ret->mp_SigTask = (struct Task *)FindTask(NULL);
       		//DPrintF("MsgPort Task: %x [%s]\n",ret->mp_SigTask, ret->mp_SigTask->Node.ln_Name);
       		NewListType(&ret->mp_MsgList, NT_MSGPORT);
     	} else
     	{
       		if (port == NULL) FreeVec(ret);
       		ret = NULL;
     	}
   	}
	return ret;
}

/**
 * @ingroup ports
 *
 * Delete a message port
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a MsgPort 
 * @return
 *		SYSERR or OK
 * @bug
 *		1) At the moment no pending messages will be answered, the caller has to do this!
 *		2) If someone used this port, which is now "gone", System will get instable.
*/
SysCall lib_DeleteMsgPort(SysBase *SysBase, pMsgPort msgPort)
{
	if (NULL != msgPort)
  	{
    	FreeSignal(msgPort->mp_SigBit);
    	FreeVec(msgPort);
		return OK;
  	}
	return SYSERR;
}


//****************** Message Handling

/**
 * @ingroup ports
 *
 * Gets a Message on a given Port, NONBLOCKING
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a MsgPort 
 * @return
 *		pMessage which arrived or NULL, if no Message
*/
pMessage lib_GetMsg(SysBase *SysBase, pMsgPort msgPort)
{
	if (msgPort == NULL) return NULL;
	if (msgPort->mp_Node.ln_Type != NT_MSGPORT) return NULL;

	UINT32 ipl = Disable();
	pMessage ret = (pMessage)RemHead(&msgPort->mp_MsgList);
	//KPrintF("GetMsg: %x\n", ret);
	Restore(ipl);
	return ret;
}

/*
 * Internal Function for signaling someone about a Message, used in PutMsg and ReplyMsg
 *
*/
static SysCall _PostMsg(SysBase *SysBase, pMessage msg, pMsgPort msgPort, UINT8 type)
{
	if (msg 	== NULL) //return SYSERR;
	{
		KPrintF("PostMsg:SysErr\n");
		return SYSERR;
	}
	if (msgPort == NULL) //return SYSERR;
	{
		KPrintF("PostMsg:SysErr\n");
		return SYSERR;
	}
	if (msgPort->mp_Node.ln_Type != NT_MSGPORT) //return SYSERR;
	{
		KPrintF("PostMsg: msgPort wrong: SysErr\n");
		return SYSERR;
	}
	//KPrintF("[PostMsg] ReplyPort: %x -[%x]\n", msgPort, msg);

	UINT32 ipl = Disable();
	msg->mn_Node.ln_Type = type;
	AddTail(&msgPort->mp_MsgList,&msg->mn_Node);
	//KPrintF("[PostMsg] AddTail List: %x Node: [%x]\n", &msgPort->mp_MsgList, &msg->mn_Node);
	
	Restore(ipl);
	if (msgPort->mp_SigTask)
	{
		switch(msgPort->mp_Flags&PA_MASK)
		{
			case PA_SIGNAL:
				//KPrintF("Signal %s (%x) , Port: %x, Bit: %x\n",msgPort->mp_SigTask->tcb_Node.ln_Name, msgPort->mp_SigTask, msgPort, 1 << msgPort->mp_SigBit);
				//KPrintF("Msg: %x\n", GetHead(&msgPort->mp_MsgList));
				SignalTask(msgPort->mp_SigTask, 1 << msgPort->mp_SigBit);
				break;
			case PA_IGNORE:
			case PA_UNKNOWN:
			default:
				break;
		}
	}
	return OK;
}

/**
 * @ingroup ports
 *
 * Puts a Message into a given Port
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a MsgPort 
 * @param name
 * 		a pointer to a Message
 * @return
 *		SYSERR or OK
*/
SysCall lib_PutMsg(SysBase *SysBase, pMsgPort msgPort, pMessage msg)
{
	return _PostMsg(SysBase, msg, msgPort, NT_MESSAGE); // Change Order!!!
}

/**
 * @ingroup ports
 *
 * Reply to a Message, if the Port has been set to NULL, the message 
 * will be flagged as free
 * 
 * @param SysBase
 *		pointer to the SysBase
 * @param name
 * 		a pointer to a Message
 * @return
 *		SYSERR or OK
*/
SysCall lib_ReplyMsg(SysBase *SysBase, pMessage msg)
{
	pMsgPort msgPort = msg->mn_ReplyPort;
/*
	if (!IsListEmpty(&msgPort->mp_MsgList)) 
	{
		KPrintF("ReplyMsg: Dest Port is not empty\n");
		pNode tmp;
		ForeachNode(&msgPort->mp_MsgList, tmp) 
		{
			KPrintF("Found: %d %x Msg[%x]", tmp->ln_Type, tmp, msg);
		}
	}
*/
	return _PostMsg(SysBase, msg, msgPort, NT_REPLYMSG);
}


