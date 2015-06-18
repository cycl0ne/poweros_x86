#ifndef devices_h
#define devices_h

#include "types.h"
#include "lists.h"
#include "ports.h"
#include "libraries.h"

typedef struct Device
{
	Node	dev_Node;
	UINT16	dev_OpenCnt;
	UINT16	dev_Flags;
	UINT16	dev_NegSize;
	UINT16	dev_PosSize;
	UINT16	dev_Version;
	UINT16	dev_Revision;
	UINT32	dev_Sum;
	STRPTR	dev_IDString;
} Device, *pDevice;

#define DEV_OPEN    1
#define DEV_CLOSE   2
#define DEV_EXPUNGE 3
#define DEV_EXTFUNC 4
/* library vector offsets for device reserved vectors */
#define DEV_BEGINIO	5
#define DEV_ABORTIO	6

/* io_Flags defined bits */
#define IOB_QUICK	0
#define IOF_QUICK	(1<<0)

#define CMD_INVALID	0
#define CMD_RESET	1
#define CMD_READ	2
#define CMD_WRITE	3
#define CMD_UPDATE	4
#define CMD_CLEAR	5
#define CMD_STOP	6
#define CMD_START	7
#define CMD_FLUSH	8
#define CMD_NONSTD	9

#define IOERR_OPENFAIL	 (-1) /* device/unit failed to open */
#define IOERR_ABORTED	 (-2) /* request terminated early [after AbortIO()] */
#define IOERR_NOCMD		 (-3) /* command not supported by device */
#define IOERR_BADLENGTH	 (-4) /* not a valid length (usually IO_LENGTH) */
#define IOERR_BADADDRESS (-5) /* invalid address (misaligned or bad range) */
#define IOERR_UNITBUSY	 (-6) /* device opens ok, but requested unit is busy */
#define IOERR_SELFTEST	 (-7) /* hardware failed self-test */

/* io_Flags defined bits */
#define IOB_QUICK	0
#define IOF_QUICK	(1<<0)

#define IOF_QUEUED (1<<4)
#define IOF_CURRENT (1<<5)
#define IOF_SERVICING (1<<6)
#define IOF_DONE (1<<7)

typedef struct Unit {
    MsgPort	unit_MsgPort;	/* queue for unprocessed messages */
    UINT16	unit_Flags;
    UINT16  unit_OpenCnt;		/* number of active opens */
} Unit, Unit_t, *pUnit;

#define TEST
#ifdef TEST
typedef struct IOStdReq {
	Message	io_Message;
	pDevice	io_Device;     /* device node pointer  */
	pUnit	io_Unit;	    /* unit (driver private)*/
	UINT16  io_Command;	    /* device command */
	UINT8   io_Flags;
	INT8    io_Error;		    /* error or warning num */
	UINT64  io_Actual;		    /* actual number of bytes transferred */
	UINT64  io_Length;		    /* requested number bytes transferred*/
	APTR    io_Data;		    /* points to data area */
	UINT64  io_Offset;		    /* offset for block structured devices */
} IOStdReq, *pIOStdReq;
#else
typedef struct IOStdReq {
	Message	io_Message;
	pDevice	io_Device;     /* device node pointer  */
	pUnit	io_Unit;	    /* unit (driver private)*/
	UINT16  io_Command;	    /* device command */
	UINT8   io_Flags;
	INT8    io_Error;		    /* error or warning num */
	UINT32  io_Actual;		    /* actual number of bytes transferred */
	UINT32  io_Length;		    /* requested number bytes transferred*/
	APTR    io_Data;		    /* points to data area */
	UINT32  io_Offset;		    /* offset for block structured devices */
} IOStdReq, *pIOStdReq;
#endif

// Trackdisk Device support:

struct DriveGeometry {
	UINT32	dg_SectorSize;		/* in bytes */
	UINT32	dg_TotalSectors;	/* total # of sectors on drive */
	UINT32	dg_Cylinders;		/* number of cylinders */
	UINT32	dg_CylSectors;		/* number of sectors/cylinder */
	UINT32	dg_Heads;		/* number of surfaces */
	UINT32	dg_TrackSectors;	/* number of sectors/track */
	UINT32	dg_BufMemType;		/* preferred buffer memory type */
					/* (usually MEMF_PUBLIC) */
	UINT8	dg_DeviceType;		/* codes as defined in the SCSI-2 spec*/
	UINT8	dg_Flags;		/* flags, including removable */
	UINT16	dg_Reserved;
};

/* device types */
#define DG_DIRECT_ACCESS	0
#define DG_SEQUENTIAL_ACCESS	1
#define DG_PRINTER		2
#define DG_PROCESSOR		3
#define DG_WORM			4
#define DG_CDROM		5
#define DG_SCANNER		6
#define DG_OPTICAL_DISK		7
#define DG_MEDIUM_CHANGER	8
#define DG_COMMUNICATION	9
#define DG_UNKNOWN		31

/* flags */
#define DGB_REMOVABLE		0
#define DGF_REMOVABLE		1

#define	TDERR_NotSpecified	20	/* general catchall */
#define	TDERR_NoSecHdr		21	/* couldn't even find a sector */
#define	TDERR_BadSecPreamble	22	/* sector looked wrong */
#define	TDERR_BadSecID		23	/* ditto */
#define	TDERR_BadHdrSum		24	/* header had incorrect checksum */
#define	TDERR_BadSecSum		25	/* data had incorrect checksum */
#define	TDERR_TooFewSecs	26	/* couldn't find enough sectors */
#define	TDERR_BadSecHdr		27	/* another "sector looked wrong" */
#define	TDERR_WriteProt		28	/* can't write to a protected disk */
#define	TDERR_DiskChanged	29	/* no disk in the drive */
#define	TDERR_SeekError		30	/* couldn't find track 0 */
#define	TDERR_NoMem		31	/* ran out of memory */
#define	TDERR_BadUnitNum	32	/* asked for a unit > NUMUNITS */
#define	TDERR_BadDriveType	33	/* not a drive that trackdisk groks */
#define	TDERR_DriveInUse	34	/* someone else allocated the drive */
#define	TDERR_PostReset		35	/* user hit reset; awaiting doom */

#define	HFERR_SelfUnit		40	/* cannot issue SCSI command to self */
#define	HFERR_DMA		41	/* DMA error */
#define	HFERR_Phase		42	/* illegal or unexpected SCSI phase */
#define	HFERR_Parity		43	/* SCSI parity error */
#define	HFERR_SelTimeout	44	/* Select timed out */
#define	HFERR_BadStatus		45	/* status and/or sense error */

/*----- OpenDevice io_Error values -----*/
#define	HFERR_NoBoard		50	/* Open failed for non-existant board */

#define	TD_MOTOR	(CMD_NONSTD+0)	/* control the disk's motor */
#define	TD_SEEK		(CMD_NONSTD+1)	/* explicit seek (for testing) */
#define	TD_FORMAT	(CMD_NONSTD+2)	/* format disk */
#define	TD_REMOVE	(CMD_NONSTD+3)	/* notify when disk changes */
#define	TD_CHANGENUM	(CMD_NONSTD+4)	/* number of disk changes */
#define	TD_CHANGESTATE	(CMD_NONSTD+5)	/* is there a disk in the drive? */
#define	TD_PROTSTATUS	(CMD_NONSTD+6)	/* is the disk write protected? */
#define	TD_RAWREAD	(CMD_NONSTD+7)	/* read raw bits from the disk */
#define	TD_RAWWRITE	(CMD_NONSTD+8)	/* write raw bits to the disk */
#define	TD_GETDRIVETYPE	(CMD_NONSTD+9)	/* get the type of the disk drive */
#define	TD_GETNUMTRACKS	(CMD_NONSTD+10)	/* # of tracks for this type drive */
#define	TD_ADDCHANGEINT	(CMD_NONSTD+11)	/* TD_REMOVE done right */
#define	TD_REMCHANGEINT	(CMD_NONSTD+12)	/* remove softint set by ADDCHANGEINT */
#define TD_GETGEOMETRY	(CMD_NONSTD+13) /* gets the disk geometry table */
#define TD_EJECT	(CMD_NONSTD+14) /* for those drives that support it */
#define	TD_LASTCOMM	(CMD_NONSTD+15)


#endif