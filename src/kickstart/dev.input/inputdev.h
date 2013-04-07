#ifndef inputdev_h
#define inputdev_h

#include "types.h"
#include "sysbase.h"
#include "io.h"
#include "mouseport.h"

void QueueCommand(struct IOStdReq *io, SysBase *SysBase);
void EndCommand(UINT32 error, struct IOStdReq *io, SysBase *SysBase);

#define	 IND_ADDHANDLER	   (CMD_NONSTD+0)
#define	 IND_REMHANDLER	   (CMD_NONSTD+1)
#define	 IND_WRITEEVENT	   (CMD_NONSTD+2)
#define	 IND_SETTHRESH	   (CMD_NONSTD+3)
#define	 IND_SETPERIOD	   (CMD_NONSTD+4)
#define	 IND_SETMPORT	   (CMD_NONSTD+5)
#define	 IND_SETMTYPE	   (CMD_NONSTD+6)
#define	 IND_SETMTRIG	   (CMD_NONSTD+7)

#define IOF_QUEUED (1<<4)
#define IOF_CURRENT (1<<5)
#define IOF_SERVICING (1<<6)
#define IOF_DONE (1<<7)

#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)

#define MOUSEAHEAD	7

typedef struct IDBase {
	struct Device		Device;
	struct Unit			Unit;
	SysBase				*SysBase;
	Task				*id_BootTask;
	Task				*id_Task;
	INT16				id_RepeatCode;
	struct List			id_HandlerList;
	struct MsgPort		id_IEPort;
	struct InputEvent	id_TData;
	struct TimeRequest	id_TIOR;
	struct InputEvent	id_MData[MOUSEAHEAD];
	struct IOStdReq		id_MIOR;
	struct InputEvent	id_K1Data;
	struct IOStdReq		id_K1IOR;
	struct InputEvent	id_K2Data;
	struct IOStdReq		id_K2IOR;
	struct TimeRequest	id_RIOR;
	struct TimeVal		id_Thresh;
	struct TimeVal		id_Period;
	struct InputEvent	id_RepeatKey;
	UINT16				id_RepeatNumeric;
	UINT16				id_Qualifier;
	UINT8				id_Prev1DownCode;
	UINT8				id_Prev1DownQual;
	UINT16				id_Prev2Down;
	UINT8				id_MPort;
	UINT8				id_MType;
	struct MouseTrigger	id_MTrig;
	UINT8				id_KRActiveMask;
} IDBase;

extern void (*inputCmdVector[])(struct IOStdReq *, IDBase *);
extern INT8 inputCmdQuick[];
#endif
