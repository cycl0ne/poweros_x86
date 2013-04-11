#include "regionbase.h"

#define LIBRARY_VERSION_STRING "\0$VER: region.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "region.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

static RegionBase *region_Init(RegionBase *RegionBase, UINT32 *segList, APTR SysBase);
APTR region_OpenLib(RegionBase *RegionBase);
APTR region_CloseLib(RegionBase *RegionBase);
APTR region_ExpungeLib(RegionBase *RegionBase);
APTR region_ExtFuncLib(RegionBase *RegionBase);
ClipRegion *reg_AllocRegion(RegionBase *RegionBase);
ClipRegion *reg_AllocRectRegion(RegionBase *RegionBase, INT32 left, INT32 top, INT32 right, INT32 bottom);
ClipRegion *reg_AllocRectRegionIndirect(Rect *prc);
void reg_CopyRegion(RegionBase *RegionBase, ClipRegion *dst, ClipRegion *src);
void reg_DestroyRegion(RegionBase *RegionBase, ClipRegion *rgn);
BOOL reg_EmptyRegion(RegionBase *RegionBase, ClipRegion *rgn);
BOOL reg_EqualRegion(RegionBase *RegionBase, ClipRegion *r1, ClipRegion *r2);
INT32 reg_GetRegionBox(RegionBase *RegionBase, ClipRegion *rgn, Rect *prc);
void reg_IntersectRegion(RegionBase *RegionBase, ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2);
void reg_OffsetRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 x, INT32 y);
BOOL reg_PtInRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 x, INT32 y);
INT32 reg_RectInRegion(RegionBase *RegionBase, ClipRegion *rgn, const Rect *rect);
void reg_SetRectRegion(RegionBase *RegionBase, ClipRegion *rgn, INT32 left, INT32 top, INT32 right, INT32 bottom);
void reg_SetRectRegionIndirect(RegionBase *RegionBase, ClipRegion *rgn, Rect *prc);
void reg_SubtractRegion(RegionBase *RegionBase, ClipRegion *regD, ClipRegion *regM, ClipRegion *regS );
void reg_SubtractRectFromRegion(RegionBase *RegionBase, const Rect *rect, ClipRegion *rgn);
void reg_UnionRegion(RegionBase *RegionBase, ClipRegion *newReg, ClipRegion *reg1, ClipRegion *reg2);
void reg_UnionRectWithRegion(RegionBase *RegionBase, const Rect *rect, ClipRegion *rgn);
void reg_XorRegion(RegionBase *RegionBase, ClipRegion *dr, ClipRegion *sra, ClipRegion *srb);

void SVGA_Init(RegionBase *RegionBase);

static volatile APTR FuncTab[] =
{
	(void(*)) region_OpenLib,
	(void(*)) region_CloseLib,
	(void(*)) region_ExpungeLib,
	(void(*)) region_ExtFuncLib,
	(void(*)) reg_AllocRegion,
	(void(*)) reg_AllocRectRegion,
	(void(*)) reg_AllocRectRegionIndirect,
	(void(*)) reg_CopyRegion,
	(void(*)) reg_DestroyRegion,
	(void(*)) reg_EmptyRegion,
	(void(*)) reg_EqualRegion,
	(void(*)) reg_GetRegionBox,
	(void(*)) reg_IntersectRegion,
	(void(*)) reg_OffsetRegion,
	(void(*)) reg_PtInRegion,
	(void(*)) reg_RectInRegion,
	(void(*)) reg_SetRectRegion,
	(void(*)) reg_SetRectRegionIndirect,
	(void(*)) reg_SubtractRegion,
	(void(*)) reg_SubtractRectFromRegion,
	(void(*)) reg_UnionRegion,
	(void(*)) reg_UnionRectWithRegion,
	(void(*)) reg_XorRegion,
	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(RegionBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)region_Init
};

static const volatile struct Resident ROMTag =
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK | RTF_AUTOINIT,
	LIBRARY_VERSION,
	NT_LIBRARY,
	95,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

static RegionBase *region_Init(RegionBase *RegionBase, UINT32 *segList, APTR SysBase)
{
	RegionBase->Library.lib_OpenCnt = 0;
	RegionBase->Library.lib_Node.ln_Pri = 0;
	RegionBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	RegionBase->Library.lib_Node.ln_Name = (STRPTR)name;
	RegionBase->Library.lib_Version = LIBRARY_VERSION;
	RegionBase->Library.lib_Revision = LIBRARY_REVISION;
	RegionBase->Library.lib_IDString = (STRPTR)&version[7];
	RegionBase->SysBase	= SysBase;
	return RegionBase;
}

static const char EndResident = 0;
