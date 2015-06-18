/**
* File: /clib．h
* User: cycl0ne
* Date: 2014-11-16
* Time: 07:40 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef clib_H
#define clib_H
#include "types.h"
#include "libraries.h"
#include "exec_interface.h"

typedef struct CLIBBase {
	struct Library	Library;
	APTR			lib_SysBase;
} CLibBase_t, *pCLibBase;





#endif
