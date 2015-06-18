/**
 * @file shell_cmds.h
 *
 * shell_cmds Prototyp
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "types.h"

void cmd_quit(void);
void cmd_cd(void);
void cmd_endcli(void);
void cmd_echo(void);
void cmd_avail(void);
void cmd_why(void);
void cmd_setenv(void);
void cmd_makedir(void);
void cmd_set(void);
void cmd_makeleanfs(void);
void cmd_dir(void);
void cmd_pfs(void);
void cmd_makesfs(void);
void cmd_sanitycheck(void);
void cmd_testconsole(void);
void cmd_nyancat(void);
void cmd_runprg(void);
void cmd_run(void);
void cmd_addbuffers(void);
void cmd_resident(void);
void cmd_debug(void);

typedef void(*shellcmd)(void );

typedef struct InternalCmd
{
	char		*name;
	shellcmd	function;
} ICmd, *pICmd;

