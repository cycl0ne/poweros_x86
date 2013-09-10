#ifndef virtiodev_h
#define virtiodev_h

#include "types.h"
#include "device.h"
#include "io.h"
#include "exec_funcs.h"
#include "lib_virtio.h"
#include "virtioblk.h"

#define MAX_VBLK	5

#define CheckForMsg(a) (((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ != NULL)
#define GetMessage(a) (struct IOStdRequest *)(((struct MsgPort)a).mp_MsgList.lh_Head->ln_Succ)

#define	HD_MOTOR	(CMD_NONSTD+0)	/* control the disk's motor */
#define	HD_SEEK		(CMD_NONSTD+1)	/* explicit seek (for testing) */
#define	HD_FORMAT	(CMD_NONSTD+2)	/* format disk */
#define	HD_REMOVE	(CMD_NONSTD+3)	/* notify when disk changes */
#define	HD_CHANGENUM	(CMD_NONSTD+4)	/* number of disk changes */
#define	HD_CHANGESTATE	(CMD_NONSTD+5)	/* is there a disk in the drive? */
#define	HD_PROTSTATUS	(CMD_NONSTD+6)	/* is the disk write protected? */
#define	HD_RAWREAD	(CMD_NONSTD+7)	/* read raw bits from the disk */
#define	HD_RAWWRITE	(CMD_NONSTD+8)	/* write raw bits to the disk */
#define	HD_GETDRIVETYPE	(CMD_NONSTD+9)	/* get the type of the disk drive */
#define	HD_GETNUMTRACKS	(CMD_NONSTD+10)	/* # of tracks for this type drive */
#define	HD_ADDCHANGEINT	(CMD_NONSTD+11)	/* TD_REMOVE done right */
#define	HD_REMCHANGEINT	(CMD_NONSTD+12)	/* remove softint set by ADDCHANGEINT */
#define HD_GETGEOMETRY	(CMD_NONSTD+13) /* gets the disk geometry table */
#define HD_EJECT	(CMD_NONSTD+14) /* for those drives that support it */
#define	HD_LASTCOMM	(CMD_NONSTD+15)
#define	HD_SCSICMD	(CMD_NONSTD+16)

#define DUB_STOPPED (1<<0)
#define DUB_IS_SERVICE (1<<7)

#define IOERR_OK	0

typedef struct VirtioBlkUnit {
    struct  MsgPort *vbu_MsgPort;	/* queue for unprocessed messages */
    UINT16  		 vbu_OpenCnt;		/* number of active opens */
    UINT8			 vbu_Flags;
    UINT8   		 vbu_ChangeState;
	struct VirtioBlk vbu_vb;
}VirtioBlkUnit;

typedef struct VDBase {
	struct Device		 vd_Device;
	APTR				*vd_SysBase;
	APTR				*vd_VirtIOBase;
	APTR				*vd_ExpansionBase;
	Task				*vd_BootTask;
	Task				*vd_Task;
	struct MsgPort		 vd_MsgPort;
	struct VirtioBlkUnit vd_Unit[MAX_VBLK];
	UINT32				 vd_MaxUnit;
	UINT32				 vd_Flags;
}VDBase;

extern void (*virtioBlkCmdVector[])(struct IOStdReq *);
void vd_EndCommand(struct IOStdReq *io, UINT32 error);
BOOL vd_VirtioBlk_setup(struct VDBase *VDBase, VirtioBlk *vb, INT32 unit_num);
BOOL vd_VirtioBlk_alloc_phys_requests(VDBase *VDBase, VirtioBlk *vb);
BOOL vd_VirtioBlk_configuration(VDBase *VDBase, VirtioBlk *vb);

#endif
