#ifndef ports_h
#define ports_h
#include "types.h"
#include "lists.h"
#include "tasks.h"


typedef struct MsgPort
{
	Node	mp_Node;
	pTask	mp_SigTask;
	List	mp_MsgList;
	UINT8	mp_Flags;
	UINT8	mp_SigBit;
	UINT16	mp_Padding;
}MsgPort, *pMsgPort, MsgPort_t;

typedef struct Message
{
	Node		mn_Node;
	pMsgPort	mn_ReplyPort;
	UINT32		mn_Length;
}Message, *pMessage;

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

#define PA_MASK     3
#define PA_UNKNOWN  3
#define PA_SIGNAL   0
#define PA_SOFTINT  1
#define PA_IGNORE   2


#endif
