#ifndef DOS_NOTIFY_H
#define DOS_NOTIFY_H

#include "types.h"
#include "ports.h"
#include "tasks.h"

// Taken from Amiga, but not used
//
#define NOTIFY_CLASS	0x40000000
#define NOTIFY_CODE	0x1234

struct NotifyMessage {
	struct Message nm_ExecMessage;
	UINT32  nm_Class;
	UINT16  nm_Code;
	struct NotifyRequest *nm_NReq;	/* don't modify the request! */
	UINT32  nm_DoNotTouch;		/* like it says!  For use by handlers */
	UINT32  nm_DoNotTouch2;		/* ditto */
};

struct NotifyRequest {
	UINT8 *nr_Name;
	UINT8 *nr_FullName;		/* set by dos - don't touch */
	UINT32 nr_UserData;		/* for applications use */
	UINT32 nr_Flags;

	union {

	    struct {
		struct MsgPort *nr_Port;	/* for SEND_MESSAGE */
	    } nr_Msg;

	    struct {
		struct Task *nr_Task;		/* for SEND_SIGNAL */
		UINT8 nr_SignalNum;		/* for SEND_SIGNAL */
		UINT8 nr_pad[3];
	    } nr_Signal;
	} nr_stuff;

	UINT32 nr_Reserved[4];		/* leave 0 for now */

	/* internal use by handlers */
	UINT32 nr_MsgCount;		/* # of outstanding msgs */
	struct MsgPort *nr_Handler;	/* handler sent to (for EndNotify) */
};


// Flags for NotifyRequest (nr_Flags)
//
#define NRF_SEND_MESSAGE	1
#define NRF_SEND_SIGNAL		2
#define NRF_WAIT_REPLY		8
#define NRF_NOTIFY_INITIAL	16

// MAGIC Code for deferred notify
//
#define NRF_MAGIC	0x80000000

// Bitnumbers of the Flags
//
#define NRB_SEND_MESSAGE	0
#define NRB_SEND_SIGNAL		1
#define NRB_WAIT_REPLY		3
#define NRB_NOTIFY_INITIAL	4
#define NRB_MAGIC			31

// Flags reserved for the internal Handlers
//
#define NR_HANDLER_FLAGS	0xffff0000


#endif
