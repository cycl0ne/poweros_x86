#include "types.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"
#include "execbase_private.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define TEMPLATE    "DEBUG"
#define OPT_DIR		0
#define OPT_COUNT   1

DOSCALL cmd_debug(pSysBase SysBase)
{
	pDOSBase	DOSBase = OpenLibrary("dos.library", 0);
	UINT32 		opts[OPT_COUNT];
	struct RDargs *rdargs;

	INT32 			rc = RETURN_FAIL, rc2 = 0;

	MemSet((char *)opts, 0, sizeof(opts));
	rdargs = ReadArgs(TEMPLATE, opts);
	if (rdargs == NULL) PrintFault(IoErr(), "ShowDebug");
	else
	{
		rc = RETURN_OK;
		Printf("Showing DebugLog: (%d)\n", SysBase->DBG_Cnt);
		Printf("%s\n", SysBase->DBG_Log);
		FreeArgs(rdargs);
	}
	CloseLibrary(DOSBase);
	return rc;
}
