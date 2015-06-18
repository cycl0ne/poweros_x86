#include "dosbase_private.h"
#include "dos_fault.h"

#define LIBRARY_VERSION_STRING "\0$VER: dos.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char Name[] = "dos.library";
static const char Version[] = LIBRARY_VERSION_STRING

static pDOSBase dos_InitLib(pDOSBase DOSBase, UINT32 *segList, pSysBase execBase);

void dos_OpenLib();
void dos_CloseLib();
void dos_ExpungeLib();
void dos_ExtFuncLib();
void dos_SplitName();
void dos_FilePart();
void dos_PathPart();
void dos_AddPart();
void dos_SendPkt();
void dos_ReplyPkt();
void dos_AbortPkt();
void dos_WaitPkt();
void dos_DoPkt();
void dos_CreatePkt();
void dos_DeletePkt();
void dos_IoErr();
void dos_SetIoErr();
void dos_CheckSignal();
void dos_FindProcess();
void dos_CreateProcess();
void dos_CreateProcessTags();
INT32 dos_NOTUSED(){return 0;}
void dos_AddDosEntry();
void dos_RemDosEntry();
void dos_MakeDosEntry();
void dos_FreeDosEntry();
void dos_FindDosEntry();
void dos_NextDosEntry();
void dos_LockDosList();
void dos_AttemptLockDosList();
void dos_UnLockDosList();
void dos_ObtainHandler();
void dos_ReleaseHandler();
void dos_Lock();
void dos_UnLock();
void dos_SetComment();
void dos_CreateDir();
void dos_DeleteFile();
void dos_SetFileDate();
void dos_SetOwner();
void dos_SetProtection();
void dos_ReadLink();
void dos_MakeLink();
void dos_SetConsoleTask();
void dos_GetConsoleTask();
void dos_CurrentDir();
void dos_GetCurrentDirName();
void dos_SetFileSystemTask();
void dos_GetFileSystemTask();
void dos_SetProgramDir();
void dos_GetProgramDir();
void dos_Cli();
void dos_Output();
void dos_SelectOutput();
void dos_Input();
void dos_SelectInput();
void dos_SetArgStr();
void dos_GetArgStr();
void dos_Open();
void dos_Close();
void dos_Write();
void dos_Read();
void dos_Seek();
void dos_SetFileSize();
void dos_DupLockFromFH();
void dos_ParentOfFH();
void dos_UnLockRecord();
void dos_LockRecord();
void dos_WaitForChar(); //70

void dos_VFPrintf();
void dos_VPrintf();
void dos_VFWritef();
void dos_GetPrompt();
void dos_DateStamp();
void dos_CLI_Init();
void dos_Examine();
void dos_ExNext();
void dos_ExamineFH();
void dos_Info();
void dos_ParentDir();
void dos_DupLock();
void dos_OpenFromLock();
void dos_SetVar();
void dos_GetVar();
void dos_FindVar();
void dos_DeleteVar();
void dos_FPutC();
void dos_FPuts();
void dos_FRead();
void dos_FGetC();
void dos_FGets();
void dos_FWrite();
void dos_Flush();
void dos_ReadArgs();
void dos_FreeArgs();
void dos_Fault();
void dos_PrintFault();
void dos_AddSegment();
void dos_RemSegment();
void dos_FindSegment();
void dos_RunCommand();
void dos_LockSegList();
void dos_UnLockSegList();
void dos_LockSegment();
void dos_UnLockSegment();
void dos_SetPrompt();
void dos_SetProgramName(); //109

void dos_UnGetC();
void dos_WriteChars();
void dos_PutStr();

void dos_SetCurrentDirName();
void dos_NameFromLock();

void dos_MatchPattern();
void dos_MatchPatternNoCase();
void dos_ParsePattern();
void dos_ParsePatternNoCase();
void dos_MatchFirst();
void dos_MatchEnd();
void dos_MatchNext();

void dos_SameLock();
void dos_Printf();
void dos_Delay ();
void dos_LoadSegment();
void dos_AddBuffers();
void dos_Inhibit();
void dos_UnloadSegment();

void dos_AssignLock();
void dos_AssignPath();
void dos_AssignLate();
void dos_AssignAdd();
void dos_AssignRem();

void dos_Execute();
void dos_System();
void dos_ReadItem();
void dos_GetString();
void dos_CliInit();

void dos_Rename();
void dos_IsFileSystem();
void dos_IsInteractive();
void dos_SameDevice();
void dos_TimeStamp();
void dos_Tell();

/*******************
*
*  Function Table
*
********************/
__attribute__((used)) static APTR FuncTab[] =
{
	(void(*)) dos_OpenLib,
	(void(*)) dos_CloseLib,
	(void(*)) dos_ExpungeLib,
	(void(*)) dos_ExtFuncLib,
	(void(*)) dos_SplitName,
	(void(*)) dos_FilePart,
	(void(*)) dos_PathPart,
	(void(*)) dos_AddPart,
	(void(*)) dos_SendPkt, //9
	(void(*)) dos_ReplyPkt,
	(void(*)) dos_AbortPkt,
	(void(*)) dos_WaitPkt,
	(void(*)) dos_DoPkt,
	(void(*)) dos_CreatePkt,
	(void(*)) dos_DeletePkt,
	(void(*)) dos_IoErr, //16
	(void(*)) dos_SetIoErr,
	(void(*)) dos_CheckSignal,
	(void(*)) dos_FindProcess,
	(void(*)) dos_CreateProcess,
	(void(*)) dos_CreateProcessTags,
	(INT32(*)) dos_NOTUSED,
	(INT32(*)) dos_NOTUSED,
	(INT32(*)) dos_NOTUSED,
	(void(*)) dos_AddDosEntry, //25
	(void(*)) dos_RemDosEntry,
	(void(*)) dos_MakeDosEntry,
	(void(*)) dos_FreeDosEntry,
	(void(*)) dos_FindDosEntry,
	(void(*)) dos_NextDosEntry,
	(void(*)) dos_LockDosList,
	(void(*)) dos_AttemptLockDosList,
	(void(*)) dos_UnLockDosList,
	(void(*)) dos_ObtainHandler,
	(void(*)) dos_ReleaseHandler,
	(void(*)) dos_Lock, //36
	(void(*)) dos_UnLock,
	(void(*)) dos_SetComment,
	(void(*)) dos_CreateDir,
	(void(*)) dos_DeleteFile,
	(void(*)) dos_SetFileDate,
	(void(*)) dos_SetOwner,
	(void(*)) dos_SetProtection,
	(void(*)) dos_ReadLink,
	(void(*)) dos_MakeLink,
	(void(*)) dos_SetConsoleTask,
	(void(*)) dos_GetConsoleTask,
	(void(*)) dos_CurrentDir,
	(void(*)) dos_GetCurrentDirName,
	(void(*)) dos_SetFileSystemTask, //50
	(void(*)) dos_GetFileSystemTask,
	(void(*)) dos_SetProgramDir,
	(void(*)) dos_GetProgramDir,
	(void(*)) dos_Cli,
	(void(*)) dos_Output,
	(void(*)) dos_SelectOutput,
	(void(*)) dos_Input,
	(void(*)) dos_SelectInput,
	(void(*)) dos_SetArgStr,
	(void(*)) dos_GetArgStr,
	(void(*)) dos_Open, //61
	(void(*)) dos_Close,
	(void(*)) dos_Write,
	(void(*)) dos_Read,
	(void(*)) dos_Seek,
	(void(*)) dos_SetFileSize,
	(void(*)) dos_DupLockFromFH,
	(void(*)) dos_ParentOfFH,
	(void(*)) dos_UnLockRecord,
	(void(*)) dos_LockRecord,
	(void(*)) dos_WaitForChar, //71
	(void(*)) dos_VFPrintf,
	(void(*)) dos_VPrintf,
	(void(*)) dos_VFWritef,
	(void(*)) dos_GetPrompt,
	(void(*)) dos_DateStamp,
	(void(*)) dos_CLI_Init,
	(void(*)) dos_CliInit,
	(void(*)) dos_CLI_Init,
	(void(*)) dos_Examine,
	(void(*)) dos_ExNext,
	(void(*)) dos_ExamineFH,
	(void(*)) dos_Info,
	(void(*)) dos_ParentDir,
	(void(*)) dos_DupLock,
	(void(*)) dos_OpenFromLock,
	(void(*)) dos_SetVar,
	(void(*)) dos_GetVar,
	(void(*)) dos_FindVar,
	(void(*)) dos_DeleteVar,
	(void(*)) dos_FPutC,
	(void(*)) dos_FPuts,
	(void(*)) dos_FRead,
	(void(*)) dos_FGetC,
	(void(*)) dos_FGets,
	(void(*)) dos_FWrite,
	(void(*)) dos_Flush,
	(void(*)) dos_ReadArgs,
	(void(*)) dos_FreeArgs,
	(void(*)) dos_Fault,	//100
	(void(*)) dos_PrintFault,
	(void(*)) dos_AddSegment,
	(void(*)) dos_RemSegment,
	(void(*)) dos_FindSegment,
	(void(*)) dos_RunCommand,
	(void(*)) dos_LockSegList,
	(void(*)) dos_UnLockSegList,
	(void(*)) dos_LockSegment,
	(void(*)) dos_UnLockSegment,
	(void(*)) dos_SetPrompt,
	(void(*)) dos_SetProgramName,
	(void(*)) dos_UnGetC,
	(void(*)) dos_WriteChars,
	(void(*)) dos_PutStr,
	(void(*)) dos_SetCurrentDirName,
	(void(*)) dos_NameFromLock,
	(void(*)) dos_MatchPattern,
	(void(*)) dos_MatchPatternNoCase,
	(void(*)) dos_ParsePattern,
	(void(*)) dos_ParsePatternNoCase,
	(void(*)) dos_MatchFirst,
	(void(*)) dos_MatchEnd,
	(void(*)) dos_MatchNext,
	(void(*)) dos_SameLock,
	(void(*)) dos_Printf,
	(void(*)) dos_Delay,
	(void(*)) dos_LoadSegment,
	(void(*)) dos_AddBuffers,
	(void(*)) dos_Inhibit,
	(void(*)) dos_UnloadSegment,
	(void(*)) dos_AssignLock,
	(void(*)) dos_AssignPath,
	(void(*)) dos_AssignLate,
	(void(*)) dos_AssignAdd,
	(void(*)) dos_AssignRem,
	(void(*)) dos_Execute,
	(void(*)) dos_System,
	(void(*)) dos_ReadItem,
	(void(*)) dos_GetString,
	(void(*)) dos_Rename,
	(void(*)) dos_IsFileSystem,
	(void(*)) dos_IsInteractive,
	(void(*)) dos_SameDevice,
	(void(*)) dos_TimeStamp,
	(void(*)) dos_Tell,
	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/

static const struct DOSBase DosLibData =
{
	.dos_LibNode.lib_Node.ln_Name = (APTR)&Name[0],
	.dos_LibNode.lib_Node.ln_Type = NT_LIBRARY,
	.dos_LibNode.lib_Node.ln_Pri = 0,
	.dos_LibNode.lib_OpenCnt = 0,
	.dos_LibNode.lib_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.dos_LibNode.lib_NegSize = 0,
	.dos_LibNode.lib_PosSize = 0,
	.dos_LibNode.lib_Version = LIBRARY_VERSION,
	.dos_LibNode.lib_Revision = LIBRARY_REVISION,
	.dos_LibNode.lib_Sum = 0,
	.dos_LibNode.lib_IDString = (APTR)&Version[7],
};

// ROMTAG Resident
static struct InitTable
{
	UINT32	LibBaseSize;
	APTR	FunctionTable;
	APTR	DataTable;
	UINT32 (*InitFunction)();
} InitTab =
{
	sizeof(struct DOSBase),
	FuncTab,
	(APTR)&DosLibData,
	(UINT32(*)())dos_InitLib
};

static APTR EndResident;

__attribute__((used)) static RESIDENT_TAG RomTag =
{
	RTC_MATCHWORD,
	&RomTag,
	&EndResident,
	0, // RTF_AUTOINIT | RTF_COLDSTART,
	LIBRARY_VERSION,
	NT_LIBRARY,
	-120,
	(APTR)Name,
	(APTR)Version,
	&InitTab
};

static void _InitSegment(pDOSBase DOSBase, STRPTR name, pSegment seg)
{
	pResidentNode res 		= FindResident(name);
	if (res)
	{
		seg->seg_Flags			= CMD_SYSTEM;
		seg->seg_Node.ln_Name	= res->rn_Resident->rt_Name;
		seg->seg_Entry			= res->rn_Resident->rt_Init;
//		KPrintF("Found Handler: %s (%x)\n", seg->seg_Node.ln_Name, seg->seg_Entry);
	} else
	{
		seg->seg_Flags			= CMD_DISABLED;
		seg->seg_Node.ln_Name	= NULL;
		seg->seg_Entry			= NULL;
	}
	seg->seg_Node.ln_Pri	= 0;
	seg->seg_Node.ln_Type	= NT_SEGMENT;
	seg->seg_Memory			= NULL;
	InitSemaphore(&seg->seg_Lock);
	AddHead((pList)&DOSBase->dos_SegList, &seg->seg_Node);
}

#undef SysBase

pDOSBase dos_InitLib(pDOSBase DOSBase, UINT32 *segList, pSysBase execBase)
{
	pSysBase SysBase = execBase;
	//KPrintF("[dos] DOSBase = %x\n", DOSBase);
	// we have no AUTOINIT, so we need to create our own startup.
	if (DOSBase == NULL)
	{
		DOSBase = (pDOSBase)MakeLibrary(InitTab.FunctionTable, InitTab.DataTable, NULL, InitTab.LibBaseSize, (UINT32)segList);
		//KPrintF("Dosbase = %x\n", DOSBase);
		if (DOSBase == NULL)
		{
			KPrintF("[dos] Error in creating DOSBase\n");
			return NULL;
		}
		DOSBase->dos_SysBase = execBase;
		DOSBase->dos_UtilBase = OpenLibrary("utility.library", 0);

		if (DOSBase->dos_UtilBase)
		{
			//KPrintF("[dos] loaded Util\n");
			NewListType((pList)&DOSBase->dos_DosList, NT_DOSLIST);
			NewListType((pList)&DOSBase->dos_SegList, NT_SEGMENT);
			InitSemaphore(&DOSBase->dos_DosListLock);
			InitSemaphore(&DOSBase->dos_EntryLock);
			InitSemaphore(&DOSBase->dos_DeleteLock);

			InitSemaphore(&DOSBase->dos_SegLock);
			InitSemaphore(&DOSBase->dos_CliLock);
			DOSBase->dos_Errors = &dos_errors;

			pIOStdReq io = (pIOStdReq)&DOSBase->dos_TimerIO;
			io->io_Message.mn_Node.ln_Type	= NT_MESSAGE;
			io->io_Message.mn_ReplyPort		= NULL;
			io->io_Message.mn_Length		= sizeof(struct TimeRequest);
			if (OpenDevice("timer.device", UNIT_VBLANK, io, 0) < 0)
			{
				KPrintF("[dos] timer.device failed\n");
				return NULL;
			}
//			KPrintF("[dos] Init Segments\n");
			_InitSegment(DOSBase, "shell", 			&DOSBase->dos_Shell);
			_InitSegment(DOSBase, "aux.handler",	&DOSBase->dos_Console);
			_InitSegment(DOSBase, "serial.handler",	&DOSBase->dos_ConTTY);
			_InitSegment(DOSBase, "ext2.handler", 	&DOSBase->dos_FileSystem);
			_InitSegment(DOSBase, "ram.handler", 	&DOSBase->dos_RAMHandler);
//			KPrintF("[dos] Add Library BASE = %x\n", &DOSBase->dos_LibNode);
			AddLibrary(&DOSBase->dos_LibNode);

			//pProcess shell = dos_CreateProc(....)
			// SetProcess Ready
			KPrintF("Create Shell process\n");
			pProcess shell = CreateProcessTags(
				NP_Name, (Tag) "PowerOS CLI",
				NP_Entry, (Tag) DOSBase->dos_Shell.seg_Entry,
				NP_Priority, 0,
				NP_StackSize, 4096*2,
				TAG_END
			);
			KPrintF("Created Shell process\n");
			if (shell)
			{
				KPrintF("Shell Entry: %x\n",  DOSBase->dos_Shell.seg_Entry);
			// Get Bootnodes from Expansion
			// DoPkt(ShellStartupMsg);
				pMsgPort pID = &shell->pr_MsgPort;
				ShellSM ssm;
				ssm.sm_ShellType= SHELL_BOOT;
				ssm.sm_Command	= NULL;
				//INT32 ShellStartupMsg = 0;
				// CheckPkt();
				KPrintF("DoPkt pid =%x\n", pID);
				DOSIO ret = DoPkt(pID, ACTION_STARTUP, NULL, (INT32)&ssm, NULL, 0, 0);

				KPrintF("[dos]DoPkt-Return\n");

				if (ret == DOSIO_TRUE)
				{
					KPrintF("Got ERROR from Shell!! PANIC!!!!!!! \n");
					for(;;);
				}
			//return...
			} else
			{
				KPrintF("Error create INIT Shell\n");
				return NULL;
			}
		} else
			return NULL;
	} else{
		KPrintF("[DOS] huh? Config change? got DOSBase [%x]", DOSBase);
	// It seems we started with AutoInit, Config Change?
	}
	return DOSBase;
}







