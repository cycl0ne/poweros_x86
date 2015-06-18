#ifndef DOS_PACKETS_H
#define DOS_PACKETS_H

#include "types.h"
#include "ports.h"
#include "dos_io.h"

#if 1
typedef struct DosPacket {
	Message		dp_Message;
	INT32		dp_Action;
	
	INT32		dp_Res1;
	INT32		dp_Res2;

	INT32		dp_Arg1;
	INT32		dp_Arg2;
	INT32		dp_Arg3;
	INT32		dp_Arg4;
	INT32		dp_Arg5;
	INT32		dp_Arg6;
	INT32		dp_Arg7;
	INT32		dp_Arg8;
} DosPacket, *pDosPacket;

#else

typedef struct DosPacket {
	Message		dp_Message;
	int64_t		dp_Action;
	
	int64_t		dp_Res1;
	int64_t		dp_Res2;

	int64_t		dp_Arg1;
	int64_t		dp_Arg2;
	int64_t		dp_Arg3;
	int64_t		dp_Arg4;
	int64_t		dp_Arg5;
	int64_t		dp_Arg6;
	int64_t		dp_Arg7;
	int64_t		dp_Arg8;	
} DosPacket, *pDosPacket;
#endif

#define ACTION_NIL		0
#define ACTION_STARTUP		0
#define ACTION_NEWSHELL		1
#define ACTION_EXECUTE		2

#define ACTION_GET_BLOCK	2	// OBSOLETE ! WARNING IS REUSED FOR SHELL!

#define ACTION_SET_MAP		4
#define ACTION_DIE		5
#define ACTION_EVENT		6
#define ACTION_CURRENT_VOLUME	7
#define ACTION_LOCATE_OBJECT	8
#define ACTION_RENAME_DISK	9
#define ACTION_WRITE		'W'
#define ACTION_READ		'R'
#define ACTION_FREE_LOCK	15
#define ACTION_DELETE_OBJECT	16
#define ACTION_RENAME_OBJECT	17
#define ACTION_MORE_CACHE	18
#define ACTION_COPY_DIR		19
#define ACTION_WAIT_CHAR	20
#define ACTION_SET_PROTECT	21
#define ACTION_CREATE_DIR	22
#define ACTION_EXAMINE_OBJECT	23
#define ACTION_EXAMINE_NEXT	24
#define ACTION_DISK_INFO	25
#define ACTION_INFO		26
#define ACTION_FLUSH		27
#define ACTION_SET_COMMENT	28  
#define ACTION_PARENT		29
#define ACTION_TIMER		30
#define ACTION_INHIBIT		31
#define ACTION_DISK_TYPE	32
#define ACTION_DISK_CHANGE	33
#define ACTION_SET_DATE		34
#define ACTION_SCREEN_MODE	994
#define ACTION_READ_RETURN	1001
#define ACTION_WRITE_RETURN	1002
#define ACTION_SEEK		1008
#define ACTION_FINDUPDATE	1004
#define ACTION_FINDINPUT	1005
#define ACTION_FINDOUTPUT	1006
#define ACTION_END		1007
#define ACTION_SET_FILE_SIZE	1022
#define ACTION_WRITE_PROTECT	1023
#define ACTION_SAME_LOCK	40
#define ACTION_CHANGE_SIGNAL	995
#define ACTION_FORMAT		1020
#define ACTION_MAKE_LINK	1021
#define ACTION_READ_LINK	1024
#define ACTION_FH_FROM_LOCK	1026
#define ACTION_IS_FILESYSTEM	1027
#define ACTION_CHANGE_MODE	1028
#define ACTION_COPY_DIR_FH	1030
#define ACTION_PARENT_FH	1031
#define ACTION_EXAMINE_ALL	1033
#define ACTION_EXAMINE_FH	1034
#define ACTION_LOCK_RECORD	2008
#define ACTION_FREE_RECORD	2009
#define ACTION_ADD_NOTIFY	4097
#define ACTION_REMOVE_NOTIFY	4098
#define ACTION_EXAMINE_ALL_END	1035
#define ACTION_SET_OWNER	1036
#define	ACTION_SERIALIZE_DISK	4200

typedef struct HandlerProc {
	pMsgPort 	hp_Port;
	pFileLock	hp_Lock;
	UINT32		hp_Flags;
	pDosEntry	hp_DevNode;
} HandlerProc, *pHandlerProc;

//
//
#define DVPB_UNLOCK	0
#define DVPF_UNLOCK	(1L << DVPB_UNLOCK)
#define DVPB_ASSIGN	1
#define DVPF_ASSIGN	(1L << DVPB_ASSIGN)




#endif
