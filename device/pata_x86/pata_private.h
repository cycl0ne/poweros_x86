#include "types.h"
#include "devices.h"
#include "interrupts.h"
#include "exec_interface.h"

#define PRIMARY_MASTER		0
#define PRIMARY_SLAVE		1
#define SECONDARY_MASTER	2
#define SECONDARY_SLAVE		3
#define	MAX_BUS				4

#define MAX_CMD				CMD_FLUSH
#define DUB_STOPPED (1<<0)
#define ATA_SECTOR_SIZE 512

typedef struct 
{
	UINT16 flags;
	UINT16 unused1[9];
	char     serial[20];
	UINT16 unused2[3];
	char     firmware[8];
	char     model[40];
	UINT16 sectors_per_int;
	UINT16 unused3;
	UINT16 capabilities[2];
	UINT16 unused4[2];
	UINT16 valid_ext_data;
	UINT16 unused5[5];
	UINT16 size_of_rw_mult;
	UINT32 sectors_28;
	UINT16 unused6[38];
	UINT64 sectors_48;
	UINT16 unused7[152];
} __attribute__((packed)) ATA_Identity;

/*
From device.h:
typedef struct Unit {
    MsgPort	unit_MsgPort;	//queue for unprocessed messages
    UINT16	unit_Flags;
    UINT16  unit_OpenCnt;	//number of active opens
} Unit, *pUnit;
*/

typedef struct PataUnit 
{
	Unit		pu_Unit;
	INT32		pu_ioBase;
	INT32		pu_Control;
	INT32		pu_Slave;
	ATA_Identity	pu_Identity;
} PataUnit, *pPataUnit;

typedef struct PataBase
{
	Device		dev_Device;
	pSysBase	dev_SysBase;
	
	pTask		dev_BootTask;
	pTask		dev_Task;
	pMsgPort	dev_Port;
	MinList		dev_InOutQueue;
	INT8		dev_Status[MAX_BUS];
	PataUnit	dev_Unit[MAX_BUS];
} PataBase, *pPataBase;

void pata_QueueCommand(struct IOStdReq *io);
void pata_EndCommand(struct IOStdReq *io, UINT32 error);
