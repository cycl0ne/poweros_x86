/**
 * @file patacmds.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "pata_private.h"
#include "ata.h"

#include "hw.h"

#define SysBase ((pPataBase)io->io_Device)->dev_SysBase
#define DEVBase ((pPataBase)io->io_Device)

static void Invalid(struct IOStdReq *io);
static void Start(struct IOStdReq *io);
static void Stop(struct IOStdReq *io);
static void Flush(pIOStdReq io);
static void Reset(pIOStdReq io);
static void SendToTask(pIOStdReq io);
static void ata_device_read_sector(pPataBase devBase, pPataUnit dev, UINT32 lba, UINT8 *buf);
static void ata_device_write_sector(pPataBase devBase, pPataUnit dev, UINT32 lba, UINT8* buf);


void (*pata_commandVector[])(struct IOStdReq *) = {
	Invalid, Reset, SendToTask/*Read*/, SendToTask/*Write*/, Invalid/*update*/, 
	/*Clear*/ Invalid, Stop, Start, Flush
};

INT8 pata_commandQuick[] =
{
	-1 //Invalid
	,-1 //RESET
	, 0 //READ
	, 0 //WRITE
	,-1 //UPDATE
	,-1 //CLEAR
	,-1 //STOP
	,-1 //START
	,-1 //FLUSH
};

//CMD_INVALID -1
static void Invalid(struct IOStdReq *io)
{
	pata_EndCommand(io, IOERR_NOCMD);
}

//CMD_STOP -1
static void Stop(struct IOStdReq *io)
{
	io->io_Unit->unit_Flags |= DUB_STOPPED;
	pata_EndCommand(io, 0);
}

static void Start(struct IOStdReq *io)
{
	io->io_Unit->unit_Flags &= ~DUB_STOPPED;
	struct IOStdReq *new = (struct IOStdReq *)GetHead(&DEVBase->dev_InOutQueue);
	if (new != NULL)
	{
		pata_commandVector[new->io_Command](new);
	}
	pata_EndCommand(io, 0);
}

//CMD_FLUSH -1
static void Flush(pIOStdReq io)
{
	struct Node *node;
	struct Node *nextnode;
	
	ForeachNodeSafe(&DEVBase->dev_InOutQueue, node, nextnode)
	{
		pata_EndCommand((struct IOStdReq *)node, IOERR_ABORTED);
	}	
	pata_EndCommand(io, 0);
}

//CMD_RESET -1
static void Reset(struct IOStdReq *io)
{
	Stop(io);
	Flush(io);
	Start(io);
}

static void SendToTask(pIOStdReq io)
{
	io->io_Flags &= ~IOF_QUICK;
	io->io_Actual = io->io_Error = 0;
	//KPrintF("PutMsg\n");
	PutMsg(DEVBase->dev_Port, (pMessage) io);
	//KPrintF("PutMsg\n");
}

void pata_EndCommand(struct IOStdReq *io, UINT32 error)
{
	io->io_Error = error;
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return; //{ KPrintF("Returning Quick [Error: %d]\n", error);return;}
	ReplyMsg(&io->io_Message);
	//KPrintF("Replied\n");
	return;	
}

static UINT64 ata_max_offset(pPataUnit dev) 
{
	UINT64 sectors = dev->pu_Identity.sectors_48;
	if (!sectors) {
		/* Fall back to sectors_28 */
		sectors = dev->pu_Identity.sectors_28;
	}

	return sectors * ATA_SECTOR_SIZE;
}

static void dev_Read(pIOStdReq io)
{
	UINT32 lba_start = io->io_Offset;
	UINT32 lba_length = io->io_Length;
	UINT8*	buffer = io->io_Data;
	
	if ((lba_start > ata_max_offset((pPataUnit)io->io_Unit)) || (buffer == NULL)) 
	{
		io->io_Actual = 0;
		pata_EndCommand(io, -1);
	}
	
//	KPrintF("Starting to read from Harddisk, %d - %d [buf=%x]\n", lba_start, lba_length, buffer);
	while (lba_length) 
	{
//		KPrintF("while buffer %x\n", buffer);
		ata_device_read_sector(DEVBase, (pPataUnit)io->io_Unit, lba_start, buffer);
		buffer+= ATA_SECTOR_SIZE;
		lba_start++;
		lba_length--;
//		KPrintF("while buffer %x\n", buffer);
	}
	io->io_Actual = lba_start-io->io_Offset;
//	KPrintF("Finished with reading %d, %d \n", lba_start, io->io_Actual);
//	KPrintF("while buffer %x\n", buffer);
	pata_EndCommand(io, 0);	
}

static void dev_Write(pIOStdReq io)
{
	UINT32 lba_start = io->io_Offset;
	UINT32 lba_length = io->io_Length;
	UINT8*	buffer = io->io_Data;
	
	if ((lba_start > ata_max_offset((pPataUnit)io->io_Unit)) || (buffer == NULL)) 
	{
		io->io_Actual = 0;
		pata_EndCommand(io, -1);
	}
	
	while (lba_length) 
	{
		ata_device_write_sector(DEVBase, (pPataUnit)io->io_Unit, lba_start, buffer);
		buffer+= ATA_SECTOR_SIZE;
		lba_start++;
		lba_length--;
	}
	io->io_Actual = lba_start-io->io_Offset;
	//KPrintF("Finished with writing %d, %d\n", lba_start, io->io_Actual);
	pata_EndCommand(io, 0);	
}

#undef SysBase

UINT32 dev_PataTask(APTR SysBase, pPataBase devBase)
{
	pIOStdReq	msg = NULL;
	
	devBase->dev_Port = CreateMsgPort(NULL);
	if (devBase->dev_Port == NULL)
	{
		KPrintF("PANIC: PataTask Error\n");
		for(;;);
	}
	
	SignalTask(devBase->dev_BootTask, SIGF_SINGLE);
	for (;;)
	{
		WaitPort(devBase->dev_Port);
		if ((msg = (pIOStdReq)GetMsg(devBase->dev_Port))!=NULL)
		{
			//KPrintF("Pata: got a msg : CMD: %d\n", msg->io_Command);
			switch (msg->io_Command)
			{
				case CMD_READ:
					dev_Read(msg);
					break;
				case CMD_WRITE:
					dev_Write(msg);				
					break;
				default:
					pata_EndCommand(msg, -1);
				break;
			}
			//ReplyMsg(&msg->io_Message);
		}
	}
	return 0;
}

#define SysBase devBase->dev_SysBase

static void ata_device_read_sector(pPataBase devBase, pPataUnit dev, UINT32 lba, UINT8 *buf) 
{
	UINT16 bus = dev->pu_ioBase;
	UINT8 slave = dev->pu_Slave;

//	spin_lock(&ata_lock);

	int errors = 0;
try_again:
	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait(dev, 0);

	outportb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 1);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if (ata_wait(dev, 1)) {
		KPrintF("Error during ATA read of lba block %d", lba);
		errors++;
		if (errors > 4) {
			KPrintF("-- Too many errors trying to read this block. Bailing.");
			//spin_unlock(&ata_lock);
			return;
		}
		goto try_again;
	}

	int size = 256;
	inportsm(bus,buf,size);
	ata_wait(dev, 0);
//	spin_unlock(&ata_lock);
}

static void ata_device_write_sector(pPataBase devBase, pPataUnit dev, UINT32 lba, UINT8* buf) 
{
	UINT16 bus = dev->pu_ioBase;
	UINT8 slave = dev->pu_Slave;

	//spin_lock(&ata_lock);

	outportb(bus + ATA_REG_CONTROL, 0x02);

	ata_wait(dev, 0);
	outportb(bus + ATA_REG_HDDEVSEL, 0xe0 | slave << 4 | (lba & 0x0f000000) >> 24);
	ata_wait(dev, 0);

	outportb(bus + ATA_REG_FEATURES, 0x00);
	outportb(bus + ATA_REG_SECCOUNT0, 0x01);
	outportb(bus + ATA_REG_LBA0, (lba & 0x000000ff) >>  0);
	outportb(bus + ATA_REG_LBA1, (lba & 0x0000ff00) >>  8);
	outportb(bus + ATA_REG_LBA2, (lba & 0x00ff0000) >> 16);
	outportb(bus + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
	ata_wait(dev, 0);
	int size = ATA_SECTOR_SIZE / 2;
	outportsm(bus,buf,size);
	outportb(bus + 0x07, ATA_CMD_CACHE_FLUSH);
	ata_wait(dev, 0);
//	spin_unlock(&ata_lock);
}

#if 0
static UINT64 ata_max_offset(pPataUnit dev) 
{
	UINT64 sectors = dev->pu_Identity.sectors_48;
	if (!sectors) {
		/* Fall back to sectors_28 */
		sectors = dev->pu_Identity.sectors_28;
	}

	return sectors * ATA_SECTOR_SIZE;
}

static UINT32 read_ata(pPataBase devBase, pPataUnit dev, UINT32 offset, UINT32 size, UINT8 *buffer) 
{
	struct ata_device * dev = (struct ata_device *)node->device;

	unsigned int start_block = offset / ATA_SECTOR_SIZE;
	unsigned int end_block = (offset + size - 1) / ATA_SECTOR_SIZE;

	unsigned int x_offset = 0;

	if (offset > ata_max_offset(dev)) {
		return 0;
	}

	if (offset + size > ata_max_offset(dev)) {
		unsigned int i = ata_max_offset(dev) - offset;
		size = i;
	}

	if (offset % ATA_SECTOR_SIZE) {
		unsigned int prefix_size = (ATA_SECTOR_SIZE - (offset % ATA_SECTOR_SIZE));
		char * tmp = malloc(ATA_SECTOR_SIZE);
		ata_device_read_sector(dev, start_block, (UINT8 *)tmp);

		memcpy(buffer, (void *)((uintptr_t)tmp + (offset % ATA_SECTOR_SIZE)), prefix_size);

		free(tmp);

		x_offset += prefix_size;
		start_block++;
	}

	if ((offset + size)  % ATA_SECTOR_SIZE && start_block < end_block) {
		unsigned int postfix_size = (offset + size) % ATA_SECTOR_SIZE;
		char * tmp = malloc(ATA_SECTOR_SIZE);
		ata_device_read_sector(dev, end_block, (UINT8 *)tmp);

		memcpy((void *)((uintptr_t)buffer + size - postfix_size), tmp, postfix_size);

		free(tmp);

		end_block--;
	}

	while (start_block <= end_block) {
		ata_device_read_sector(dev, start_block, (UINT8 *)((uintptr_t)buffer + x_offset));
		x_offset += ATA_SECTOR_SIZE;
		start_block++;
	}

	return size;
}

static UINT32 write_ata(pPataBase devBase, pPataUnit dev, UINT32 offset, UINT32 size, UINT8 *buffer) 
{
	struct ata_device * dev = (struct ata_device *)node->device;

	unsigned int start_block = offset / ATA_SECTOR_SIZE;
	unsigned int end_block = (offset + size - 1) / ATA_SECTOR_SIZE;

	unsigned int x_offset = 0;

	if (offset > ata_max_offset(dev)) {
		return 0;
	}

	if (offset + size > ata_max_offset(dev)) {
		unsigned int i = ata_max_offset(dev) - offset;
		size = i;
	}

	if (offset % ATA_SECTOR_SIZE) {
		unsigned int prefix_size = (ATA_SECTOR_SIZE - (offset % ATA_SECTOR_SIZE));

		char * tmp = malloc(ATA_SECTOR_SIZE);
		ata_device_read_sector(dev, start_block, (UINT8 *)tmp);

		debug_print(NOTICE, "Writing first block");

		memcpy((void *)((uintptr_t)tmp + (offset % ATA_SECTOR_SIZE)), buffer, prefix_size);
		ata_device_write_sector_retry(dev, start_block, (UINT8 *)tmp);

		free(tmp);
		x_offset += prefix_size;
		start_block++;
	}

	if ((offset + size)  % ATA_SECTOR_SIZE && start_block < end_block) {
		unsigned int postfix_size = (offset + size) % ATA_SECTOR_SIZE;

		char * tmp = malloc(ATA_SECTOR_SIZE);
		ata_device_read_sector(dev, end_block, (UINT8 *)tmp);

		debug_print(NOTICE, "Writing last block");

		memcpy(tmp, (void *)((uintptr_t)buffer + size - postfix_size), postfix_size);

		ata_device_write_sector_retry(dev, end_block, (UINT8 *)tmp);

		free(tmp);
		end_block--;
	}

	while (start_block <= end_block) {
		ata_device_write_sector_retry(dev, start_block, (UINT8 *)((uintptr_t)buffer + x_offset));
		x_offset += ATA_SECTOR_SIZE;
		start_block++;
	}

	return size;
}
#endif
