#ifndef DOS_H
#define DOS_H

#include "types.h"
#include "lists.h"
#include "libraries.h"

#include "dos_io.h"

#define	 DOSNAME  "dos.library"

// error report types for ErrorReport()
//
#define REPORT_STREAM		0	/* a stream */
#define REPORT_TASK			1	/* a process - unused */
#define REPORT_LOCK			2	/* a lock */
#define REPORT_VOLUME		3	/* a volume node */
#define REPORT_INSERT		4	/* please insert volume */

// DOS Shell
//
#define RUN_EXECUTE		-1
#define RUN_SYSTEM		-2
#define RUN_SYSTEM_ASYNCH	-3

// DOS Call Returnvalues, beware the difference to the DOSIO!!!!!!
//
typedef INT32	DOSCALL;
#define	DOSCMD_FAIL		-1
#define DOSCMD_NULL		 0
#define	DOSCMD_SUCCESS	 1

// DOS IO Returnvalues
//
typedef INT32	DOSIO;
#define DOSIO_FALSE		 0
#define DOSIO_TRUE		-1
#define DOSIO_ZERO		 0   // The ZERO LOCK = Root dir

#define ZERO_LOCK		0
#define LOCK_DIFFERENT		-1
#define LOCK_SAME		0
#define LOCK_SAME_VOLUME	1
#define LOCK_SAME_HANDLER	LOCK_SAME_VOLUME

#define SIGBREAKB_CTRL_C   12
#define SIGBREAKB_CTRL_D   13
#define SIGBREAKB_CTRL_E   14
#define SIGBREAKB_CTRL_F   15

#define SIGBREAKF_CTRL_C   (1<<SIGBREAKB_CTRL_C)
#define SIGBREAKF_CTRL_D   (1<<SIGBREAKB_CTRL_D)
#define SIGBREAKF_CTRL_E   (1<<SIGBREAKB_CTRL_E)
#define SIGBREAKF_CTRL_F   ((long)1<<SIGBREAKB_CTRL_F)

#define ENDSTREAMCH	-1

// Return Codes used by PowerDOS
//
#define CLI_DEFAULT_STACK	8192
#define CLI_DEFAULT_FAIL_LEVEL	10

#define RETURN_OK		 0
#define RETURN_WARN		 5
#define RETURN_ERROR	10
#define RETURN_FAIL		20

#define LINK_HARD	0
#define LINK_SOFT	1	/* softlinks are not fully supported yet */

#define CHANGE_LOCK	0
#define CHANGE_FH	1

typedef struct PathList {
	MinNode		pl_Node;
	pFileLock	pl_Lock;
}PathList, *pPathList;

typedef struct PathNode {
	MinNode		pl_Node;
	pFileLock	pl_Lock;
}PathNode, *pPathNode;

typedef struct CommandLineInterface {
	INT32		cli_Result2;
	STRPTR		cli_SetName;
	//pFileLock	cli_CommandDir;
	MinList		cli_CommandDir;
	INT32		cli_ReturnCode;
	STRPTR		cli_CommandName;
	INT32		cli_FailLevel;
	STRPTR		cli_Prompt;
    pFileHandle	cli_StandardInput;
    pFileHandle	cli_CurrentInput;
    STRPTR		cli_CommandFile;
    BOOL		cli_Interactive;
    BOOL		cli_Background; 
    pFileHandle	cli_CurrentOutput;
    INT32		cli_DefaultStack; 
    pFileHandle	cli_StandardOutput;
    pSegment	cli_Module;
}ComLinInt, *pComLinInt;

typedef INT32 (*Process_Function)(void *, void *, UINT32);
typedef INT32 (*Exit_Function)(void *, DOSCALL, APTR);

// DOS Process structure
//
typedef struct Process {
	Task				pr_Task;
	MsgPort				pr_MsgPort;
	Process_Function	pr_ProcessCode;
	STRPTR				pr_Arguments;
	INT32				pr_Result2;
	Exit_Function		pr_ExitCode;
	APTR				pr_ExitData;
	MinList 			pr_LocalVars; 
	pFileLock			pr_CurrentDir;
	pFileLock			pr_HomeDir;
	pFileHandle			pr_CIS;
	pFileHandle			pr_COS;
	pFileHandle			pr_CES;
	pMsgPort			pr_ConsoleTask;
	pMsgPort			pr_FileSystemTask;
	pComLinInt			pr_CLI;
	INT32				pr_CLINum;
	MinList				pr_OpenFiles;
	APTR				pr_WindowPtr; // Fix this sometime!
}Process, *pProcess;

typedef struct LocalVar {
	struct Node lv_Node;
	UINT32	lv_Flags;
	UINT8	*lv_Value;
	UINT32	lv_Len;
}LocalVar, *pLocalVar;


/* bit definitions for lv_Node.ln_Type: */
#define LV_VAR			0	/* an variable */
#define LV_ALIAS		1	/* an alias */
/* to be or'ed into type: */
#define LVB_IGNORE		7	/* ignore this entry on GetVar, etc */
#define LVF_IGNORE		0x80

/* definitions of flags passed to GetVar()/SetVar()/DeleteVar() */
/* bit defs to be OR'ed with the type: */
/* item will be treated as a single line of text unless BINARY_VAR is used */
#define GVB_GLOBAL_ONLY		8
#define GVF_GLOBAL_ONLY		0x100
#define GVB_LOCAL_ONLY		9
#define GVF_LOCAL_ONLY		0x200
#define GVB_BINARY_VAR		10		/* treat variable as binary */
#define GVF_BINARY_VAR		0x400
#define GVB_DONT_NULL_TERM	11	/* only with GVF_BINARY_VAR */
#define GVF_DONT_NULL_TERM	0x800

/* this is only supported in >= V39 dos.  V37 dos ignores this. */
/* this causes SetVar to affect ENVARC: as well as ENV:.	*/
#define GVB_SAVE_VAR		12	/* only with GVF_GLOBAL_VAR */
#define GVF_SAVE_VAR		0x1000

/* Returned by ReadItem() */
#define ITEM_EQUAL    -2
#define ITEM_ERROR    -1
#define ITEM_NOTHING   0
#define ITEM_UNQUOTED  1
#define ITEM_QUOTED    2

#endif
