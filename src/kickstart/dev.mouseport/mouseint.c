#include "types.h"
#include "io.h"
#include "list.h"
#include "device.h"
#include "irq.h"
#include "io.h"
#include "inputevent.h"

#include "mouseport.h"

#include "sysbase.h"
#include "exec_funcs.h"

/// in in INT8 ///
static inline INT8 inb(INT16 _port)
{
	INT8 result;
	__asm__ volatile ("inb %1, %0" : "=a" (result) : "Nd" (_port));
	return result;
}
/// out in INT8 ///
static inline void outb(INT16 _port, INT8 _data)
{
	__asm__ volatile ("outb %0, %1" : : "a" (_data), "Nd" (_port));
}

#define IsMsgPortEmpty(x) \
	( ((x)->mp_MsgList.lh_TailPred) == (struct Node *)(&(x)->mp_MsgList) )

#define	LEFT_CLICK   0x01
#define	RIGHT_CLICK  0x02
#define	MIDDLE_CLICK 0x04

#define IRQ_MOUSE		12
#define UBF_INITIALIZE	1

#   define GetHead(l)       (void *)(((struct List *)l)->lh_Head->ln_Succ \
				? ((struct List *)l)->lh_Head \
				: (struct Node *)0)
#   define GetTail(l)       (void *)(((struct List *)l)->lh_TailPred->ln_Pred \
				? ((struct List *)l)->lh_TailPred \
				: (struct Node *)0)
#   define GetSucc(n)       (void *)(((struct Node *)n)->ln_Succ->ln_Succ \
				? ((struct Node *)n)->ln_Succ \
				: (struct Node *)0)
#   define GetPred(n)       (void *)(((struct Node *)n)->ln_Pred->ln_Pred \
				? ((struct Node *)n)->ln_Pred \
				: (struct Node *)0)

static INT8 mouse_byte[3];
static INT8	mouse_cycle = 0;

__attribute__((no_instrument_function)) BOOL mouse_handler(UINT32 number, MDBase *MDBase, APTR SysBase)
{
	UINT8 status = inb(0x64);
	if (!(status & 0x20)) return 1;

    switch(mouse_cycle)
    {
    case 0:
         mouse_byte[0]=inb(0x60);
         ++mouse_cycle;
         break;
    case 1:
        mouse_byte[1]=inb(0x60);
        ++mouse_cycle;
        break;
    case 2:
        mouse_byte[2]=inb(0x60);
 
		UINT32 tail = MDBase->BufTail;
		tail += 4;
		tail &= (MDBUFSIZE-1);
		
		if (tail == MDBase->BufHead)
		{
			//test_over++;
			//DPrintF("B!");			//Bufferoverflow
			//if (test_over == 10) {
			//	DPrintF("Bufferoverflow [%x][%x]\n", MDBase->BufTail, MDBase->BufHead);
			//	while(1);
			//}
		} else 
		{
			//test_over--;
			UINT16 qualifier = 0;
			UINT16 buttons = IECODE_NOBUTTON;
			if (mouse_byte[0] & 0x01) {buttons = IECODE_LBUTTON; qualifier |= LEFT_CLICK;}
			if (mouse_byte[0] & 0x02) {buttons = IECODE_RBUTTON; qualifier |= RIGHT_CLICK;}
			if (mouse_byte[0] & 0x03) {buttons = IECODE_MBUTTON; qualifier |= MIDDLE_CLICK;}

			MDBase->BufQueue[tail]	= buttons;
			MDBase->BufQueue[tail+1]= qualifier;
			MDBase->BufQueue[tail+2]= mouse_byte[1];
			MDBase->BufQueue[tail+3]= mouse_byte[2];
			MDBase->BufTail = tail;
			//DPrintF("bh %x, bt %x\n",MDBase->BufHead, MDBase->BufTail);
		}
		mouse_cycle = 0;
		if (MDBase->Flags & DUB_IS_SERVICE) {
			DPrintF("[Service!]");
			//while(1);
			return 1; // we are in Service
		}
		MDBase->Flags |= DUB_IS_SERVICE;

		if (MDBase->Unit.unit_Flags & DUB_STOPPED) {DPrintF("mouseport.dev stopped\n");MDBase->Flags &= ~DUB_IS_SERVICE;return 0;}
		if (!IsMsgPortEmpty(&MDBase->Unit.unit_MsgPort)) {
			struct IOStdReq *new = (struct IOStdReq *)GetHead(&MDBase->Unit.unit_MsgPort.mp_MsgList);
			mouseCmdVector[MD_READEVENT](new, MDBase);
		}
		MDBase->Flags &= ~DUB_IS_SERVICE;
		break;
	default:
		DPrintF("Error in PS2Device\n");
		break;
	}
	return 1;
}
