#include "types.h"
#include "utility.h"

#define LIBRARY_VERSION_STRING "\0$VER: utility.library 0.1 ("__DATE__")\r\n";
#define LIBRARY_VERSION 0
#define LIBRARY_REVISION 1

static const char name[] = "utility.library";
static const char version[] = LIBRARY_VERSION_STRING
static const char EndResident;

APTR util_OpenLib(pUtility *UtilBase);
APTR util_CloseLib(pUtility *UtilBase);
APTR util_ExpungeLib(pUtility *UtilBase);
APTR util_ExtFuncLib(pUtility *UtilBase);
static pUtility util_Init(pUtility UtilBase, UINT32 *segList, APTR SysBase);

struct TagItem *util_NextTagItem(pUtility UtilBase, const struct TagItem **tagListPtr);
struct TagItem *util_FindTagItem(pUtility UtilBase, UINT32 tagValue, const struct TagItem *tagList);
BOOL util_TagInArray(pUtility UtilBase, UINT32 tagValue, UINT32 *tagArray);
UINT32 util_GetTagData(pUtility UtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList);
struct TagItem *util_AllocateTagItems(pUtility UtilBase, UINT32 numTags);
void util_FreeTagItems(pUtility UtilBase, struct TagItem *tagList);
void util_RefreshTagItemClones(pUtility UtilBase, struct TagItem *clone, const struct TagItem *original);
struct TagItem *util_CloneTagItems(pUtility UtilBase, const struct TagItem *tagList);
UINT32 util_PackBoolTags(pUtility UtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap);
UINT32 util_FilterTagItems(pUtility UtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic);
void util_FilterTagChanges(pUtility UtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply);
void util_MapTags(pUtility UtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType);
void util_ApplyTagChanges(pUtility UtilBase, struct TagItem *list, struct TagItem *changelist);

UINT32 util_CalcDate(pUtility UtilBase, UINT32 year, UINT32 month, UINT32 day);
void util_Os2Date(pUtility UtilBase, UINT32 amiga, struct ClockData *cd);
UINT32 util_Date2Os(pUtility UtilBase, struct ClockData *cd);
UINT32 util_CheckDate(pUtility UtilBase, struct ClockData *date);

STRPTR util_NamedObjectName(pUtility UtilBase, struct NamedObject *nos);
struct NamedObject *util_FindNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject);
BOOL util_AddNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, struct NamedObject *object);
BOOL util_AttemptRemNamedObject(pUtility UtilBase, struct NamedObject *nos);
BOOL util_RemNamedObject(pUtility UtilBase, struct NamedObject *object, struct Message *message);
BOOL util_ReleaseNamedObject(pUtility UtilBase, struct NamedObject *object);
struct NamedObject *util_AllocNamedObjectA(pUtility UtilBase, STRPTR name, struct TagItem *tagList);
void util_FreeNamedObject(pUtility UtilBase, struct NamedObject *object);

UINT8 util_ToUpper(pUtility UtilBase, UINT8 c);
UINT8 util_ToLower(pUtility UtilBase, UINT8 c);
INT32 util_Strlen(pUtility UtilBase, const char *str);
UINT8 *util_Strncpy(pUtility UtilBase, char *dst, const char *src, INT32 n);
INT32 util_Stricmp(pUtility UtilBase, const char *s1, const char *s2);
INT32 util_Strnicmp(pUtility UtilBase, const char *s1, const char *s2, INT32 n);
INT32 util_Strcmp(pUtility UtilBase, const char *s1, const char *s2);
char *util_Strcpy(pUtility UtilBase, char *to, const char *from);

static volatile APTR FuncTab[] = 
{
	(void(*)) util_OpenLib,
	(void(*)) util_CloseLib,
	(void(*)) util_ExpungeLib,
	(void(*)) util_ExtFuncLib,
	(void(*)) util_NextTagItem,
	(void(*)) util_FindTagItem,
	(void(*)) util_TagInArray,
	(void(*)) util_GetTagData,
	(void(*)) util_AllocateTagItems,
	(void(*)) util_FreeTagItems,
	(void(*)) util_RefreshTagItemClones,
	(void(*)) util_CloneTagItems,
	(void(*)) util_PackBoolTags,
	(void(*)) util_FilterTagItems,
	(void(*)) util_FilterTagChanges,
	(void(*)) util_MapTags,
	(void(*)) util_ApplyTagChanges,
	(void(*)) util_CalcDate,
	(void(*)) util_Os2Date,
	(void(*)) util_Date2Os,
	(void(*)) util_CheckDate,
	(void(*)) util_NamedObjectName,
	(void(*)) util_FindNamedObject,
	(void(*)) util_AddNamedObject,
	(void(*)) util_AttemptRemNamedObject,
	(void(*)) util_RemNamedObject,
	(void(*)) util_ReleaseNamedObject,
	(void(*)) util_AllocNamedObjectA,
	(void(*)) util_FreeNamedObject,
	(void(*)) util_ToUpper,
	(void(*)) util_ToLower,
	(void(*)) util_Strlen,
	(void(*)) util_Strncpy,
	(void(*)) util_Stricmp,
	(void(*)) util_Strnicmp,
	(void(*)) util_Strcmp,
	(void(*)) util_Strcpy,
	(APTR) ((UINT32)-1)
};

static const volatile APTR InitTab[4]=
{
	(APTR)sizeof(struct UtilityBase),
	(APTR)FuncTab,
	(APTR)NULL,
	(APTR)util_Init
};

static const volatile struct Resident ROMTag = 
{
	RTC_MATCHWORD,
	(struct Resident *)&ROMTag,
	(APTR)&EndResident,
	RTF_SINGLETASK,
	LIBRARY_VERSION,
	NT_LIBRARY,
	95,
	(char *)name,
	(char*)&version[7],
	0,
	&InitTab
};

static pUtility util_Init(pUtility UtilBase, UINT32 *segList, APTR SysBase)
{
	UtilBase->Library.lib_OpenCnt = 0;
	UtilBase->Library.lib_Node.ln_Pri = 0;
	UtilBase->Library.lib_Node.ln_Type = NT_LIBRARY;
	UtilBase->Library.lib_Node.ln_Name = (STRPTR)name;
	UtilBase->Library.lib_Version = LIBRARY_VERSION;
	UtilBase->Library.lib_Revision = LIBRARY_REVISION;
	UtilBase->Library.lib_IDString = (STRPTR)&version[7];	
	UtilBase->SysBase	= SysBase;
	return UtilBase;
}

static const char EndResident = 0;
