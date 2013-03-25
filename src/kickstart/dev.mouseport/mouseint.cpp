#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"
#include "kbd_io.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"

#define	LEFT_CLICK   0x01
#define	RIGHT_CLICK  0x02
#define	MIDDLE_CLICK 0x04

#define IRQ_MOUSE		12
#define UBF_INITIALIZE	1

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

static INT8  mouse_byte[3];
#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

void arch_irq_mask(UINT32);
void arch_irq_unmask(UINT32);
#define DisableMouse() arch_irq_mask(1<<12);
#define EnableMouse() arch_irq_unmask(1<<12);

__attribute__((no_instrument_function)) BOOL mouse_handler(UINT32 number, MDBase *MDBase, APTR SysBase)
{
	UINT8 status = inb(0x64);
	if (!(status & 0x20)) return 0;
	UINT8 cycle = MDBase->MouseCycle;

    switch(MDBase->MouseCycle)
    {
    case 0:
         mouse_byte[0]=inb(0x60);
         MDBase->MouseCycle++;
         break;
    case 1:
        mouse_byte[1]=inb(0x60);
        MDBase->MouseCycle++;
        break;
    case 2:
        mouse_byte[2]=inb(0x60);
        MDBase->MouseCycle++;
		if (mouse_byte[0] & 0x80 || mouse_byte[0] & 0x40) 
		{
			MDBase->MouseCycle = 0;
			break; //Overflow? BadPacket
		}
//		DisableMouse();
		UINT32 tail = MDBase->BufTail;
		tail += 4;
		tail &= (MDBUFSIZE-1);
//		DPrintF("H:%x T:%x t:%x\n",MDBase->BufTail, MDBase->BufHead, tail);
		//DPrintF("[%x][%x][%x]-", mouse_byte[0],mouse_byte[1],mouse_byte[2]);
		if (tail == MDBase->BufHead)
		{
			//Bufferoverflow
			DPrintF("B!");
		} else 
		{
			UINT16 qualifier = 0;
			UINT16 buttons = IECODE_NOBUTTON;
			if (mouse_byte[0] & 0x01) {buttons = IECODE_LBUTTON; qualifier |= LEFT_CLICK;}
			if (mouse_byte[0] & 0x02) {buttons = IECODE_RBUTTON; qualifier |= RIGHT_CLICK;}
			if (mouse_byte[0] & 0x03) {buttons = IECODE_MBUTTON; qualifier |= MIDDLE_CLICK;}

			MDBase->BufQueue[tail] = buttons;
			MDBase->BufQueue[tail+1] = qualifier;
			MDBase->BufQueue[tail+2] = mouse_byte[1];
			MDBase->BufQueue[tail+3] = mouse_byte[2];

			MDBase->BufTail = tail; // Write Value back
	//DPrintF("MouseBuffer: %x  %x\n", MDBase->BufQueue[MDBase->BufTail], MDBase->BufQueue[MDBase->BufTail+1]);
		}
		MDBase->MouseCycle = 0;
		if (MDBase->Unit.unit_Flags & DUB_STOPPED) return 0;
		
		if (!IsMsgPortEmpty(&MDBase->Unit.unit_MsgPort))
		{
			struct IOStdReq *new = (struct IOStdReq *)MDBase->Unit.unit_MsgPort.mp_MsgList.lh_Head;
			if (new != NULL) 
			{
				//DPrintF("(INT)CmdVector call\n");
				mouseCmdVector[MD_READEVENT](new, MDBase);
				//DPrintF("(INT)CmdVector call back\n");
			}
		}
		break;
	default:
		DPrintF("Servicing still 2\n");
		break;
	}
	return 0;
}
