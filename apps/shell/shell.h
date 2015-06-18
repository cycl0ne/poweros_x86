#ifndef SHELL_H
#define SHELL_H
/**
 * @file lexan.c
 *
 * Lexical analyser for our shell
 * Idea from xinus lexan, adopted and extended.
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

/* Moved to dos_io.h
#define SHELL_RUNEX			1
#define SHELL_SYSTEM_SYNC 	2
#define SHELL_SYSTEM_ASYNC	3
*/
/*
 * Shell Startupmsg
 * 
 * ARG1 = Name
 * ARG2 = StartupMsg / TagList? ;-)
 * ARG3 = ComLinInt
 * ARG4 = 0
 * ARG5 = 0
 * -------------------------
 * from handler.c (DISK):
 * DOSIO ret = DoPkt(pID, ACTION_STARTUP, (INT32)name, StartupMsg, (INT32)dosEntry, 0, 0);
**/
/*
typedef struct StartupMsg
{
	UINT32		sm_SType;		// SM Type: Shell, Disk,...
	
	// - Shell Message -------------
	UINT32		sm_ShellType;	// SHELL_RUNEX, SHELL_SYSTEM, SHELL_SYSTEM_ASYNC
	pFileHandle	sm_Command;
	pFileHandle	sm_Input;
	pFileHandle	sm_Output;
	pFileLock	sm_CurDir;
	BOOL		sm_Run;
	pMsgPort	sm_FileSysTask;
} ShellSM, *pShellSM;
*/
#define ERROR_SHELL_INIT	300

typedef struct GlobalData
{
	APTR		gd_SysBase;
	pDOSBase	gd_DOSBase;
	pUtilBase	gd_UtilBase;
	pProcess	gd_ShellProc;
	pComLinInt	gd_Cli;
	pShellSM	gd_SStartup;
	
	STRPTR		gd_Prompt;
	STRPTR		gd_CommandName;
	STRPTR		gd_CommandFile;
	STRPTR		gd_SetName;

	BOOL		gd_Echo;
	UINT32		gd_CliNum;
	pFileHandle	gd_StdOut;
	UINT8		buffer[512];
	INT8		gd_Alias[256];
	UINT8		cbuffer[512];
	INT32		cpos;
	INT32		gd_Res1;
	INT32		gd_Res2;
}GD, *pGD;


#define SHELL_BUFLEN  256       /**< length of general buffer           */
#define SHELL_MAXTOK  32        /**< maximum tokens per line            */
#define SHELL_SYNTAXERR  "Syntax error.\n" 

#define	SH_NEWLINE	'\n'		/* New line character		*/
#define	SH_EOF		'\04'		/* Control-D is EOF		*/
#define	SH_AMPER	'&'		/* ampersand character		*/
#define	SH_BLANK	' '		/* blank character		*/
#define	SH_TAB		'\t'		/* tab character		*/
#define	SH_SQUOTE	'\''		/* single quote character	*/
#define	SH_DQUOTE	'"'		/* double quote character	*/
#define	SH_LESS		'<'		/* less-than character	*/
#define	SH_GREATER	'>'		/* greater-than character	*/

/* Token types */

#define	SH_TOK_AMPER	0		/* ampersand token		*/
#define	SH_TOK_LESS	1		/* less-than token		*/
#define	SH_TOK_GREATER	2		/* greater-than token		*/
#define	SH_TOK_OTHER	3		/* token other than those	*/
#define NULLCH	'\0'

DOSCALL	shell_lexan(INT8* line, INT32 len, INT8* tokbuf, INT32* tlen, INT32 tok[], INT32 toktyp[]);



#endif
