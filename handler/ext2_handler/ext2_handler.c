/**
 * @file ext2handler.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */

#include "residents.h"
#include "ext2_handler.h"
#include "bcache_interface.h"

#define HANDLER_VERSION_STRING "\0$VER: ext2.handler 0.1 ("__DATE__")\r\n";
#define HANDLER_VERSION 0

static const char Name[]	= "ext2.handler";
static const char Version[] = HANDLER_VERSION_STRING
static APTR EndResident;
static void EXT2_Handler(APTR data);
extern void ext2_DOSLoop(pGD gd);
extern BOOL ext2_MountVol(pGD gd);

static volatile RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	0, // RTF_AUTOINIT | RTF_COLDSTART,
	HANDLER_VERSION,
	NT_HANDLER,
	-120,
	(APTR)Name,
	(APTR)Version,
	(APTR)EXT2_Handler
};

#define ReturnError 	ReplyPkt(pkt, DOSIO_FALSE, RETURN_FAIL)
#undef SysBase

void ext2_InitTimerFlush(pGD gd, UINT32 secs);

static void EXT2_Handler(APTR SysBase)
{
	pGD			gd = AllocVec(sizeof(struct GlobalData), MEMF_PUBLIC|MEMF_CLEAR);

	if (gd)
	{
		gd->gd_SysBase	= SysBase;
		DOSBase			= OpenLibrary("dos.library", 0);
		BCBase	= OpenLibrary("bcache.library", 0);
		if (!BCBase) KPrintF("Couldnt open bcache.lib\n");
		if (DOSBase)
		{
			pDosPacket	pkt = WaitPkt();

			UtilBase	= OpenLibrary("utility.library", 0);
			if (UtilBase)
			{
				STRPTR name			= (STRPTR) 		pkt->dp_Arg1;
				pStartupMsg fssm	= (pStartupMsg)	pkt->dp_Arg2;
				pDosEntry dosEntry	= (pDosEntry) 	pkt->dp_Arg3;

				gd->gd_mountName	= name;
				gd->gd_fssm			= fssm;
				gd->gd_dosEntry		= dosEntry;

				gd->gd_myproc		= FindProcess(NULL);
				if (gd->gd_myproc == NULL)
				{
					KPrintF("ERR: MyProc\n");
				}
				gd->gd_msgport		= &gd->gd_myproc->pr_MsgPort;
				// Tell dos our handlerport
				dosEntry->de_Handler = gd->gd_msgport;

				gd->gd_port			= CreateMsgPort(NULL);

				if (gd->gd_port)
				{
					gd->gd_request		= CreateIORequest(gd->gd_port, sizeof(IOStdReq));
					if (gd->gd_request)
					{
//						KPrintF("[EXT2]Initialize\n");
//						KPrintF("[EXT2] MountName: %s\n", name);
						gd->gd_tport = CreateMsgPort(NULL);
						if (gd->gd_tport)
						{
							gd->gd_trequest = (TimeRequest*) CreateIORequest(gd->gd_tport, sizeof(TimeRequest));
							if (gd->gd_trequest)
							{
								if (OpenDevice("timer.device", UNIT_VBLANK, (IOStdReq*)gd->gd_trequest, 0)!= IOERR_OPENFAIL)
								{
									ext2_InitTimerFlush(gd,  5);
	#if 0
									KPrintF("FSSM Debug\n");
									KPrintF("FSSM Device: %s\n", fssm->fssm_Device);
									KPrintF("FSSM Unit: %d\n", fssm->fssm_Unit);
									KPrintF("FSSM Flags: %x\n", fssm->fssm_Flags);
									KPrintF("FSSM Environment --------\n");
									#define denvec fssm->fssm_Environ
	
									KPrintF("DosEnvec: TableSize %d", denvec->de_TableSize);
									KPrintF(", de_SizeBlock %d", denvec->de_SizeBlock);
									KPrintF(", de_SecOrg %d", denvec->de_SecOrg);
									KPrintF(", de_Surfaces %d", denvec->de_Surfaces);
									KPrintF(", de_SectorPerBlock %d", denvec->de_SectorPerBlock);
									KPrintF(", de_BlocksPerTrack %d", denvec->de_BlocksPerTrack);
									KPrintF(", de_Reserved %d", denvec->de_Reserved);
									KPrintF(", de_PreAlloc %d", denvec->de_PreAlloc);
									KPrintF(", de_Interleave %d", denvec->de_Interleave);
									KPrintF(", de_LowCyl %d", denvec->de_LowCyl);
									KPrintF(", de_HighCyl %d", denvec->de_HighCyl);
									KPrintF(", de_NumBuffers %d", denvec->de_NumBuffers);
									KPrintF(", de_BufMemType %x", denvec->de_BufMemType);
									KPrintF(", de_MaxTransfer %x", denvec->de_MaxTransfer);
									KPrintF(", de_Mask %d", denvec->de_Mask);
									KPrintF(", de_BootPri %d", denvec->de_BootPri);
									KPrintF(", de_DosType %x\n", denvec->de_DosType);
	#endif
									KPrintF("Open Device: %s on Unit: %d\n",fssm->fssm_Device, fssm->fssm_Unit);
									if (OpenDevice(fssm->fssm_Device, fssm->fssm_Unit, gd->gd_request, 0) != IOERR_OPENFAIL)
									{
										NewList((pList)&gd->gd_LockList);
										NewList((pList)&gd->gd_NodeList);
										// Initialise Disk
										// TODO:
										gd->gd_Debug = 0;
										if (ext2_MountVol(gd))
										{
											gd->gd_CacheSize= 20;
											gd->gd_Cache	= CreateCache(gd->gd_request, gd->block_size, 20);
											// Return OK
											Strcpy(gd->root_node->name, "System");
											gd->gd_VolumeNode = MakeDosEntry("System", DLT_VOLUME);
											gd->gd_VolumeNode->de_Handler = gd->gd_msgport;
											gd->gd_VolumeNode->de_Misc.volumeNode.de_DiskType = ID_DOS_DISK;
											DateStamp(&gd->gd_VolumeNode->de_Misc.volumeNode.de_VolumeDate);
											if (AddDosEntry(gd->gd_VolumeNode) == DOSCMD_SUCCESS)
											{
			//									KPrintF("Send ok");
												ReplyPkt(pkt, DOSIO_TRUE, RETURN_OK);
			//									KPrintF("-> ok\n");
												// Do DOS Processing
												ext2_DOSLoop(gd);
											}
											FreeDosEntry(gd->gd_VolumeNode);
										}
									}
									CloseDevice((IOStdReq*)gd->gd_trequest);
								}
								DeleteIORequest((IOStdReq*)gd->gd_trequest);
							}
							DeleteMsgPort(gd->gd_tport);
						}
						DeleteIORequest(gd->gd_request);
					}

					DeleteMsgPort(gd->gd_port);
				}

				CloseLibrary(UtilBase);
			}

			CloseLibrary(DOSBase);
			ReplyPkt(pkt, DOSIO_FALSE, RETURN_FAIL);
		}
	}
	return;
}





