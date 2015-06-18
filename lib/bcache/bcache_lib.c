#include "bcache.h"
#include "residents.h"

#define LIBRARY_VERSION_STRING "\0$VER: bcache.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char Name[] = "bcache.library";
static const char Version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static APTR bc_OpenLib(pBCacheBase BCacheBase)
{
	BCacheBase->Library.lib_OpenCnt++;
	//SysBase->LibNode.exp_Flags &= ~LIBF_DELEXP;
	return(BCacheBase);
}

static APTR bc_CloseLib(pBCacheBase BCacheBase)
{
	BCacheBase->Library.lib_OpenCnt--;
	return NULL;
}

static APTR bc_ExpungeLib(pBCacheBase BCacheBase)
{
	return NULL;
}

static APTR bc_ExtFuncLib(pBCacheBase BCacheBase)
{
	return NULL;
}

static pBCacheBase bc_InitLibrary(pBCacheBase BCacheBase, UINT32 *segList, pSysBase execBase)
{
	//pSysBase SysBase = execBase;
	BCacheBase->bc_SysBase = execBase;
	return BCacheBase;
}

/*******************

Function Table

********************/

bcache_t bc_CreateCache(pBCacheBase BCacheBase, struct IOStdReq* io, INT32 block_size, INT32 block_count);
void bc_DestroyCache(pBCacheBase BCacheBase, bcache_t _cache);
int bc_ReadBlock(pBCacheBase BCacheBase, bcache_t _cache, void *buf, UINT64 blocknum);
int bc_GetBlock(pBCacheBase BCacheBase, bcache_t _cache, void **ptr, UINT64 blocknum);
int bc_PutBlock(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum);
int bc_MarkBlockDirty(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum);
int bc_ZeroBlock(pBCacheBase BCacheBase, bcache_t _cache, UINT64 blocknum);
int bc_FlushCache(pBCacheBase BCacheBase, bcache_t _cache);
void bc_DumpCache(pBCacheBase BCacheBase, bcache_t _cache);


static volatile APTR FuncTab[] =
{
	(void(*)) bc_OpenLib,
	(void(*)) bc_CloseLib,
	(void(*)) bc_ExpungeLib,
	(void(*)) bc_ExtFuncLib,

	(void(*)) bc_CreateCache,
	(void(*)) bc_DestroyCache,
	(void(*)) bc_ReadBlock,
	(void(*)) bc_GetBlock,
	(void(*)) bc_PutBlock,
	(void(*)) bc_MarkBlockDirty,
	(void(*)) bc_ZeroBlock,
	(void(*)) bc_FlushCache,
	(void(*)) bc_DumpCache,

	(APTR) ((UINT32)-1)
};

/*******************

RESIDENT PART

********************/
static const struct BCacheBase BCacheLibData =
{
	.Library.lib_Node.ln_Name = (APTR)&Name[0],
	.Library.lib_Node.ln_Type = NT_LIBRARY,
	.Library.lib_Node.ln_Pri = 0,
	.Library.lib_OpenCnt = 0,
	.Library.lib_Flags = 0, //LIBF_SUMUSED|LIBF_CHANGED,
	.Library.lib_NegSize = 0,
	.Library.lib_PosSize = 0,
	.Library.lib_Version = LIBRARY_VERSION,
	.Library.lib_Revision = LIBRARY_REVISION,
	.Library.lib_Sum = 0,
	.Library.lib_IDString = (APTR)&Version[7],
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(struct BCacheBase),
	(APTR)FuncTab,
	(APTR)&BCacheLibData,
	(APTR)bc_InitLibrary
};

static const char EndResident = 0;

static const volatile RESIDENT_TAG ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_AUTOINIT|RTF_COLDSTART,
	LIBRARY_VERSION,
	NT_LIBRARY,
	103,
	(STRPTR)Name,
	(STRPTR)&Version[7],
	&InitTab
};



