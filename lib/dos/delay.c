/**
 * @file delay.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"
#include "timer.h"

#define TICKS_PER_SECOND 128

static void _dotimer(pDOSBase DOSBase, INT32 secs, INT32 micro)
{
	struct TimeRequest	timerio;
	/* copy the request */
	CopyMem(&DOSBase->dos_TimerIO, &timerio, sizeof(struct TimeRequest));

	/* set replyport to my process reply port */
	timerio.tr_node.io_Message.mn_ReplyPort = &FindProcess(NULL)->pr_MsgPort;
	timerio.tr_node.io_Command = TR_ADDREQUEST;
	timerio.tr_time.tv_micro   = micro;
	timerio.tr_time.tv_secs    = secs;
//KPrintF("doio\n");
	DoIO((pIOStdReq) &timerio);
//KPrintF("doio\n");
}

/* is callable from a task!!! */

void dos_Delay (pDOSBase DOSBase, UINT32 ticks)
{
	if (ticks)
	{
		struct TimeRequest	timerio;
		UINT32 secs = ticks/ TICKS_PER_SECOND;
		UINT32 micro = 1000000UL / TICKS_PER_SECOND * (ticks % TICKS_PER_SECOND);	
		/* copy the request */
		CopyMem(&DOSBase->dos_TimerIO, &timerio, sizeof(struct TimeRequest));

		/* set replyport to my process reply port */
		timerio.tr_node.io_Message.mn_ReplyPort = &FindProcess(NULL)->pr_MsgPort;
		timerio.tr_node.io_Command = TR_ADDREQUEST;
		timerio.tr_time.tv_micro   = micro;
		timerio.tr_time.tv_secs    = secs;
		DoIO((pIOStdReq) &timerio);
	}
}
