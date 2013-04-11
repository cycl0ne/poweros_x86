#include "types.h"

#if 0

struct TagItem *NextTagItem(pUtility UtilBase, const struct TagItem **tagListPtr);
struct TagItem *FindTagItem(pUtility UtilBase, UINT32 tagValue, const struct TagItem *tagList);
BOOL TagInArray(pUtility UtilBase, UINT32 tagValue, UINT32 *tagArray);
UINT32 GetTagData(pUtility UtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList);
struct TagItem *AllocateTagItems(pUtility UtilBase, UINT32 numTags);
void FreeTagItems(pUtility UtilBase, struct TagItem *tagList);
void RefreshTagItemClones(pUtility UtilBase, struct TagItem *clone, const struct TagItem *original);
struct TagItem *CloneTagItems(pUtility UtilBase, const struct TagItem *tagList);
UINT32 PackBoolTags(pUtility UtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap);
UINT32 FilterTagItems(pUtility UtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic);
void FilterTagChanges(pUtility UtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply);
void MapTags(pUtility UtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType);
void ApplyTagChanges(pUtility UtilBase, struct TagItem *list, struct TagItem *changelist);

UINT32 CalcDate(pUtility UtilBase, UINT32 year, UINT32 month, UINT32 day);
void Amiga2Date(pUtility UtilBase, UINT32 amiga, struct ClockData *cd);
UINT32 Date2Amiga(pUtility UtilBase, struct ClockData *cd);
UINT32 CheckDate(pUtility UtilBase, struct ClockData *date);

STRPTR NamedObjectName(pUtility UtilBase, struct NamedObject *nos);
struct NamedObject *FindNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject);
BOOL AddNamedObject(pUtility UtilBase, struct NamedObject *nameSpace, struct NamedObject *object);
BOOL AttemptRemNamedObject(pUtility UtilBase, struct NamedObject *nos);
BOOL RemNamedObject(pUtility UtilBase, struct NamedObject *object, struct Message *message);
BOOL ReleaseNamedObject(pUtility UtilBase, struct NamedObject *object);
struct NamedObject *AllocNamedObjectA(pUtility UtilBase, STRPTR name, struct TagItem *tagList);
void FreeNamedObject(pUtility UtilBase, struct NamedObject *object);

UINT8 ToUpper(pUtility UtilBase, UINT8 c);
UINT8 ToLower(pUtility UtilBase, UINT8 c);
INT32 Strlen(pUtility UtilBase, const char *str);
UINT8 *Strncpy(pUtility UtilBase, char *dst, const char *src, INT32 n);
INT32 Stricmp(pUtility UtilBase, const char *s1, const char *s2);
INT32 Strnicmp(pUtility UtilBase, const char *s1, const char *s2, INT32 n);
INT32 Strcmp(pUtility UtilBase, const char *s1, const char *s2);
char *Strcpy(pUtility UtilBase, char *to, const char *from);



#endif

#define NextTagItem(a)				(((struct TagItem *(*)(pUtility UtilBase, const struct TagItem **tagListPtr))				_GETVECADDR(UtilBase, 5))(UtilBase,a))
#define FindTagItem(a,b)			(((struct TagItem *(*)(pUtility UtilBase, UINT32 tagValue, const struct TagItem *tagList))	_GETVECADDR(UtilBase, 6))(UtilBase,a,b))
#define TagInArray(a,b)				(((BOOL  (*)(pUtility UtilBase, UINT32 tagValue, UINT32 *tagArray))									_GETVECADDR(UtilBase, 7))(UtilBase,a,b))
#define GetTagData(a,b,c)			(((UINT32(*)(pUtility UtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList))	_GETVECADDR(UtilBase, 8))(UtilBase,a,b,c))
#define AllocateTagItems(a)			(((struct TagItem *(*)(pUtility UtilBase, UINT32 numTags))											_GETVECADDR(UtilBase, 9))(UtilBase,a))
#define FreeTagItems(a)				(((void  (*)(pUtility UtilBase, struct TagItem *tagList))											_GETVECADDR(UtilBase,10))(UtilBase,a))
#define RefreshTagItemClones(a,b)	(((void  (*)(pUtility UtilBase, struct TagItem *clone, const struct TagItem *original))				_GETVECADDR(UtilBase,11))(UtilBase,a,b))
#define CloneTagItems(a)			(((struct TagItem *(*)(pUtility UtilBase, const struct TagItem *tagList))							_GETVECADDR(UtilBase,12))(UtilBase,a))
#define PackBoolTags(a,b,c)			(((UINT32(*)(pUtility UtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap))	_GETVECADDR(UtilBase,13))(UtilBase,a,b,c))
#define FilterTagItems(a,b,c)		(((UINT32(*)(pUtility UtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic))			_GETVECADDR(UtilBase,14))(UtilBase,a,b,c))
#define FilterTagChanges(a,b,c)		(((void  (*)(pUtility UtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply))	_GETVECADDR(UtilBase,15))(UtilBase,a,b,c))
#define MapTags(a,b,c)				(((void  (*)(pUtility UtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType))	_GETVECADDR(UtilBase,16))(UtilBase,a,b,c))
#define ApplyTagChanges(a,b)		(((void  (*)(pUtility UtilBase, struct TagItem *list, struct TagItem *changelist))					_GETVECADDR(UtilBase,17))(UtilBase,a,b))

#define CalcDate(a,b,c)				(((UINT32(*)(pUtility UtilBase, UINT32 year, UINT32 month, UINT32 day))	_GETVECADDR(UtilBase,18))(UtilBase,a,b,c))
#define Amiga2Date(a,b)				(((void  (*)(pUtility UtilBase, UINT32 amiga, struct ClockData *cd))	_GETVECADDR(UtilBase,19))(UtilBase,a,b))
#define Date2Amiga(a)				(((UINT32(*)(pUtility UtilBase, struct ClockData *cd))					_GETVECADDR(UtilBase,20))(UtilBase,a))
#define CheckDate(a)				(((UINT32(*)(pUtility UtilBase, struct ClockData *date))				_GETVECADDR(UtilBase,21))(UtilBase,a))

#define NamedObjectName(a)			(((STRPTR(*)(pUtility UtilBase, struct NamedObject *nos))									_GETVECADDR(UtilBase,22))(UtilBase,a))
#define FindNamedObject(a,b,c)		(((struct NamedObject *(*)(pUtility UtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject)) _GETVECADDR(UtilBase,23))(UtilBase,a,b,c))
#define AddNamedObject(a,b)			(((BOOL(*)(pUtility UtilBase, struct NamedObject *nameSpace, struct NamedObject *object))	_GETVECADDR(UtilBase,24))(UtilBase,a,b))
#define AttemptRemNamedObject(a)	(((BOOL(*)(pUtility UtilBase, struct NamedObject *nos))										_GETVECADDR(UtilBase,25))(UtilBase,a))
#define RemNamedObject(a,b)			(((BOOL(*)(pUtility UtilBase, struct NamedObject *object, struct Message *message))			_GETVECADDR(UtilBase,26))(UtilBase,a,b))
#define ReleaseNamedObject(a)		(((BOOL(*)(pUtility UtilBase, struct NamedObject *object))									_GETVECADDR(UtilBase,27))(UtilBase,a))
#define AllocNamedObjectA(a,b)		(((struct NamedObject *(*)(pUtility UtilBase, STRPTR name, struct TagItem *tagList))		_GETVECADDR(UtilBase,28))(UtilBase,a,b))
#define FreeNamedObject(a)			(((void(*)(pUtility UtilBase, struct NamedObject *object))									_GETVECADDR(UtilBase,29))(UtilBase,a))

#define ToUpper(a)					(((UINT8 (*)(pUtility UtilBase, UINT8 c))								_GETVECADDR(UtilBase,30))(UtilBase,a))
#define ToLower(a)					(((UINT8 (*)(pUtility UtilBase, UINT8 c))								_GETVECADDR(UtilBase,31))(UtilBase,a))
#define Strlen(a)					(((INT32 (*)(pUtility UtilBase, const char *str))						_GETVECADDR(UtilBase,32))(UtilBase,a))
#define Strncpy(a,b,c)				(((UINT8*(*)(pUtility UtilBase, char *dst, const char *src, INT32 n))	_GETVECADDR(UtilBase,33))(UtilBase,a,b,c))
#define Stricmp(a,b)				(((INT32 (*)(pUtility UtilBase, const char *s1, const char *s2))		_GETVECADDR(UtilBase,34))(UtilBase,a,b))
#define Strnicmp(a,b,c)				(((INT32 (*)(pUtility UtilBase, const char *s1, const char *s2, INT32 n))	_GETVECADDR(UtilBase,35))(UtilBase,a,b,c))
#define Strcmp(a,b)					(((INT32 (*)(pUtility UtilBase, const char *s1, const char *s2))			_GETVECADDR(UtilBase,36))(UtilBase,a,b))
#define Strcpy(a,b)					(((char *(*)(pUtility UtilBase, char *to, const char *from))				_GETVECADDR(UtilBase,37))(UtilBase,a,b))

#define Rand()						(((INT32(*)(pUtility UtilBase))					_GETVECADDR(UtilBase,38))(UtilBase))
#define Random()					(((INT32(*)(pUtility UtilBase))					_GETVECADDR(UtilBase,39))(UtilBase))
#define SRand(a)					(((void (*)(pUtility UtilBase,UINT32 seed))		_GETVECADDR(UtilBase,40))(UtilBase,a))
#define SRandom(a)					(((void (*)(pUtility UtilBase,UINT32 seed))		_GETVECADDR(UtilBase,41))(UtilBase,a))
