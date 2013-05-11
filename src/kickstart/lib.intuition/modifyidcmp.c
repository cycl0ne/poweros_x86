#include "types.h"
#include "intuitionbase.h"
#include "inputevent.h"
#include "idcmp.h"
#include "intupools.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

#if 0
static void _ClearPending(struct nWindow *win, UINT32 class, UINT32 qual)
{
	if (class == IDCMP_MASK_TIMER)
	{
		CHANGE_BITS(win->Flags, WFLG_WINDOWTICKED);
	} else if (class == IDCMP_MASK_MOUSE_MOTION)
	{
		--(win->MousePending);
	} else if ((class == IDCMP_MASK_RAWKEY || class == IDCMP_MASK_VANILLAKEY || class == IDCMP_MASK_IDCMPUPDATE) && TEST_BITS(qual, IEQUALIFIER_REPEAT))
	{
		--(win->RptPending);
	}
}

void _RecycleMsg(IntuitionBase *IBase, struct nWindow *win)
{
	struct IDCMPMessage *msg;
	//if () return; // Should check if this window is in our list!
	if (NULL == win->WPort) return;
	
	while ((msg = (struct IDCMPMessage *) GetMsg(win->WPort)))
	{
		if (msg->ExecMessage.mn_Node.ln_Type == NT_REPLYMSG)
		{
			_ClearPending(win, msg->Class, msg->Qualifier);
			msg->Class = IDCMP_MASK_EMPTYMSG;
		}
	}
}

BOOL intu_ModifyIDCMP(IntuitionBase *IBase, struct nWindow *win, UINT32 flags)
{
	BOOL port = FALSE;
	
	if (flags)
	{
		if (win->WPort == NULL)
		{
			win->WindowPort.mp_Flags = PA_IGNORE;
			NewList(&win->WindowPort.mp_MsgList);
			win->WPort = &win->WindowPort;
			port = TRUE;
		}
		if (win->UserPort == NULL)
		{
			win->UserPort = CreateMsgPort();
			if (!win->UserPort) 
			{
				if (port) win->WPort = NULL;
				return FALSE;
			}
		}
    }
//call IntuHandler(ModifyIDCMP)
    if (flags == 0)
    {
		struct MsgPort	*tmp = win->UserPort;
		if (tmp)
		{
			win->UserPort = NULL;
			DeleteMsgPort(tmp);
		}
		win->WPort = NULL;
    }
    return TRUE;
}

void srv_ModifyIDCMP(IntuitionBase *IBase, struct nWindow *win, UINT32 flags)
{
	win->IDCMPFlags = flags;
	if (!flags)
	{
		if (win->UserPort)
		{
			struct IDCMPMessage *msg;
			while ( (msg = (struct IDCMPMessage *) GetMsg(win->UserPort)) )
			{
				ReplyMsg((struct Message *)msg);
			}
		}
		_RecycleMsg(IBase, win);
	}
}

static UINT32 _trans4IEtoIDCMP(UINT32 ieclass, UINT32 code)
{
	return 1;
}

BOOL srv_SendIDCMP(IntuitionBase *IBase, UINT32 ieclass, UINT32 code, struct InputToken *it, struct nWindow *win, APTR iaddress)
{
	struct IDCMPMessage	*imsg;
	UINT32				idcmpclass = _trans4IEtoIDCMP(ieclass, code);
	struct InputEvent	*ie = it->it_IE;
	
	if (!idcmpclass) return FALSE;
	if ( (idcmpclass == IDCMP_MASK_RAWKEY) && TEST_BITS( win->IDCMPFlags, IDCMP_MASK_VANILLAKEY) )
	{
		if (TEST_BITS( code, IECODE_UP_PREFIX)) 
		{
			return TRUE;
		} else if (it->it_Code != 0)
		{
			idcmpclass = IDCMP_MASK_VANILLAKEY;
			code = it->it_Code;
		} else if ( !TEST_BITS(win->IDCMPFlags, IDCMP_MASK_RAWKEY) )
		{
			return TRUE;
		}
	}
	
	if ( TEST_BITS(win->IDCMPFlags, idcmpclass) )
	{		
		if (idcmpclass == IDCMP_MASK_MOUSE_MOTION)
		{
			if (win->MousePending < win->MouseQueueLimit)
			{
				win->MousePending++;
			} else
			{
				return TRUE;
			}
		}
		
		if ( (idcmpclass == IDCMP_MASK_RAWKEY) || (idcmpclass == IDCMP_MASK_VANILLAKEY) && TEST_BITS( ie->ie_Qualifier, IEQUALIFIER_REPEAT ) )
		{
			win->RptPending++;
		} else
		{
			return TRUE;
		}
		
		imsg = srv_InitIDCMPMsg(IBase, idcmpclass, code, ie, win);

		if (imsg)
		{
			imsg->iAddress = iaddress;
			if ( idcmpclass == IDCMP_MASK_RAWKEY)
			{
				
			} else if (ieclass = IECLASS_RAWMOUSE)
			{
				imsg->IAddress = win;
				if (TESTBITS(win->IDCMPFlags, IDCMP_MASK_MOUSE_MOTION))
				{
					imsg->MouseX = ie->ie_X;
					imsg->MouseY = ie->ie_Y;
				}
			}
			if (window->UserPort != NULL)
			{
				PutMsg(window->UserPort, (struct Message *)imsg);
			} else
			{
				ReplyMsg(window->WPort, (struct Message *)imsg);
			}
		}
		return TRUE;
	}
	return FALSE;
}
#endif
