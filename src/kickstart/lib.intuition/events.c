/*
 * All Input Events are handled here. What are we doing? We have 2 Streams: 
 * a) input.device giving us: Keyboard, 100hz Timer, Mouse
 * b) intuition.library giving us Statechanges and Workload
 */
#include "intuitionbase.h"
#include "input.h"
#include "exec_funcs.h"

#define SysBase IBase->ib_SysBase

struct InputEvent *IntuitionInput(struct InputEvent *ie, IntuitionBase *IBase)
{
//	DPrintF("ie");
	return ie;
}
