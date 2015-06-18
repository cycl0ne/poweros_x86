#ifndef utility_interface_h
#define utility_interface_h

typedef struct UtilBase *pUtilBase;

//return TRUE if node 'a' is less than or equal to node 'b'
typedef BOOL(*pNodeCmpLe)(Node* a, Node* b);

#if 1

#define NextTagItem(a)				_LIBCALL(UtilBase,  5, struct TagItem *, (pUtilBase, const struct TagItem **tagListPtr), (UtilBase,a))
#define FindTagItem(a,b)	    	_LIBCALL(UtilBase,  6, struct TagItem *, (pUtilBase, UINT32 tagValue, const struct TagItem *tagList), (UtilBase,a,b))
#define TagInArray(a,b)	        	_LIBCALL(UtilBase,  7, BOOL  , (pUtilBase, UINT32 tagValue, UINT32 *tagArray), (UtilBase,a,b))
#define GetTagData(a,b,c)	    	_LIBCALL(UtilBase,  8, UINT32, (pUtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList), (UtilBase,a,b,c))
#define AllocateTagItems(a)		    _LIBCALL(UtilBase,  9, struct TagItem *, (pUtilBase, UINT32 numTags), (UtilBase,a))
#define FreeTagItems(a)		        _LIBCALL(UtilBase, 10, void  , (pUtilBase, struct TagItem *tagList), (UtilBase,a))
#define RefreshTagItemClones(a,b)	_LIBCALL(UtilBase, 11, void  , (pUtilBase, struct TagItem *clone, const struct TagItem *original), (UtilBase,a,b))
#define CloneTagItems(a)			_LIBCALL(UtilBase, 12, struct TagItem *, (pUtilBase, const struct TagItem *tagList), (UtilBase,a))
#define PackBoolTags(a,b,c)			_LIBCALL(UtilBase, 13, UINT32, (pUtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap), (UtilBase,a,b,c))
#define FilterTagItems(a,b,c)		_LIBCALL(UtilBase, 14, UINT32, (pUtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic), (UtilBase,a,b,c))
#define FilterTagChanges(a,b,c)		_LIBCALL(UtilBase, 15, void  , (pUtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply), (UtilBase,a,b,c))
#define MapTags(a,b,c)				_LIBCALL(UtilBase, 16, void  , (pUtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType), (UtilBase,a,b,c))
#define ApplyTagChanges(a,b)		_LIBCALL(UtilBase, 17, void  , (pUtilBase, struct TagItem *list, struct TagItem *changelist), (UtilBase,a,b))
#define CalcDate(a,b,c)				_LIBCALL(UtilBase, 18, UINT32, (pUtilBase, UINT32 year, UINT32 month, UINT32 day), (UtilBase,a,b,c))
#define Amiga2Date(a,b)				_LIBCALL(UtilBase, 19, void  , (pUtilBase, UINT32 amiga, struct ClockData *cd), (UtilBase,a,b))
#define Date2Amiga(a)				_LIBCALL(UtilBase, 20, UINT32, (pUtilBase, struct ClockData *cd), (UtilBase,a))
#define CheckDate(a)				_LIBCALL(UtilBase, 21, UINT32, (pUtilBase, struct ClockData *date), (UtilBase,a))
#define NamedObjectName(a)			_LIBCALL(UtilBase, 22, STRPTR, (pUtilBase, struct NamedObject *nos), (UtilBase,a))
#define FindNamedObject(a,b,c)		_LIBCALL(UtilBase, 23, struct NamedObject *, (pUtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject), (UtilBase,a,b,c))
#define AddNamedObject(a,b)			_LIBCALL(UtilBase, 24, BOOL, (pUtilBase, struct NamedObject *nameSpace, struct NamedObject *object), (UtilBase,a,b))
#define AttemptRemNamedObject(a)	_LIBCALL(UtilBase, 25, BOOL, (pUtilBase, struct NamedObject *nos), (UtilBase,a))
#define RemNamedObject(a,b)			_LIBCALL(UtilBase, 26, BOOL, (pUtilBase, struct NamedObject *object, struct Message *message), (UtilBase,a,b))
#define ReleaseNamedObject(a)		_LIBCALL(UtilBase, 27, BOOL, (pUtilBase, struct NamedObject *object), (UtilBase,a))
#define AllocNamedObjectA(a,b)		_LIBCALL(UtilBase, 28, struct NamedObject *, (pUtilBase, STRPTR name, struct TagItem *tagList), (UtilBase,a,b))
#define FreeNamedObject(a)			_LIBCALL(UtilBase, 29, void, (pUtilBase, struct NamedObject *object), (UtilBase,a))
#define ToUpper(a)					_LIBCALL(UtilBase, 30, UINT8 , (pUtilBase, UINT8 c), (UtilBase,a))
#define ToLower(a)					_LIBCALL(UtilBase, 31, UINT8 , (pUtilBase, UINT8 c), (UtilBase,a))
#define Strlen(a)					_LIBCALL(UtilBase, 32, INT32 , (pUtilBase, STRPTR ), (UtilBase,a))
#define Strncpy(a,b,c)				_LIBCALL(UtilBase, 33, UINT8*, (pUtilBase, STRPTR, STRPTR, INT32 n), (UtilBase,a,b,c))
#define Stricmp(a,b)				_LIBCALL(UtilBase, 34, INT32 , (pUtilBase, STRPTR, STRPTR), (UtilBase,a,b))
#define Strnicmp(a,b,c)				_LIBCALL(UtilBase, 35, INT32 , (pUtilBase, STRPTR, STRPTR, INT32 n), (UtilBase,a,b,c))
#define Strcmp(a,b)					_LIBCALL(UtilBase, 36, INT32 , (pUtilBase, STRPTR, STRPTR), (UtilBase,a,b))
#define Strcpy(a,b)					_LIBCALL(UtilBase, 37, char *, (pUtilBase, STRPTR, STRPTR), (UtilBase,a,b))
#define Rand()						_LIBCALL(UtilBase, 38, INT32, (pUtilBase), (UtilBase))
#define Random()					_LIBCALL(UtilBase, 39, INT32, (pUtilBase), (UtilBase))
#define SRand(a)					_LIBCALL(UtilBase, 40, void , (pUtilBase,UINT32 seed), (UtilBase,a))
#define SRandom(a)					_LIBCALL(UtilBase, 41, void , (pUtilBase,UINT32 seed), (UtilBase,a))
#define Strchr(a,b)					_LIBCALL(UtilBase, 42, STRPTR, (pUtilBase, const STRPTR, INT32), (UtilBase,a,b))
#define Strtok(a,b)					_LIBCALL(UtilBase, 43, STRPTR, (pUtilBase, STRPTR, CONST_STRPTR), (UtilBase,a,b))
#define Strtok_r(a,b,c)				_LIBCALL(UtilBase, 44, STRPTR, (pUtilBase, STRPTR, CONST_STRPTR, char **), (UtilBase,a,b,c))
#define Strpbrk(a,b)				_LIBCALL(UtilBase, 45, STRPTR, (pUtilBase, CONST_STRPTR, CONST_STRPTR), (UtilBase,a,b))
#define Strspn(a,b)					_LIBCALL(UtilBase, 46, INT32, (pUtilBase, CONST_STRPTR, CONST_STRPTR), (UtilBase,a,b))
#define Strrchr(a,b)				_LIBCALL(UtilBase, 47, STRPTR, (pUtilBase, CONST_STRPTR, INT32), (UtilBase,a,b))
#define Strcspn(a,b)				_LIBCALL(UtilBase, 48, INT32, (pUtilBase, CONST_STRPTR, CONST_STRPTR), (UtilBase,a,b))
#define CallHookPkt(a,b,c)			_LIBCALL(UtilBase, 49, INT32, (pUtilBase, struct Hook*, APTR, APTR), (UtilBase,a,b,c))
#define Strcat(a,b)					_LIBCALL(UtilBase, 50, STRPTR, (pUtilBase, STRPTR, CONST_STRPTR), (UtilBase,a,b))
#define StrDup(a)					_LIBCALL(UtilBase, 51, STRPTR , (pUtilBase, CONST_STRPTR ), (UtilBase,a))
#define QuickSort(a,b)              _LIBCALL(UtilBase, 52, void , (pUtilBase, pList, pNodeCmpLe), (UtilBase,a,b))

#else

#define NextTagItem(a)				(((struct TagItem *(*)(pUtilBase, const struct TagItem **tagListPtr))				_GETVECADDR(UtilBase, 5))(UtilBase,a))
#define FindTagItem(a,b)			(((struct TagItem *(*)(pUtilBase, UINT32 tagValue, const struct TagItem *tagList))	_GETVECADDR(UtilBase, 6))(UtilBase,a,b))
#define TagInArray(a,b)				(((BOOL  (*)(pUtilBase, UINT32 tagValue, UINT32 *tagArray))									_GETVECADDR(UtilBase, 7))(UtilBase,a,b))
#define GetTagData(a,b,c)			(((UINT32(*)(pUtilBase, Tag tagValue, UINT32 defaultVal, const struct TagItem *tagList))	_GETVECADDR(UtilBase, 8))(UtilBase,a,b,c))
#define AllocateTagItems(a)			(((struct TagItem *(*)(pUtilBase, UINT32 numTags))											_GETVECADDR(UtilBase, 9))(UtilBase,a))
#define FreeTagItems(a)				(((void  (*)(pUtilBase, struct TagItem *tagList))											_GETVECADDR(UtilBase,10))(UtilBase,a))
#define RefreshTagItemClones(a,b)	(((void  (*)(pUtilBase, struct TagItem *clone, const struct TagItem *original))				_GETVECADDR(UtilBase,11))(UtilBase,a,b))
#define CloneTagItems(a)			(((struct TagItem *(*)(pUtilBase, const struct TagItem *tagList))							_GETVECADDR(UtilBase,12))(UtilBase,a))
#define PackBoolTags(a,b,c)			(((UINT32(*)(pUtilBase, UINT32 initialFlags, struct TagItem *tagList, struct TagItem *boolMap))	_GETVECADDR(UtilBase,13))(UtilBase,a,b,c))
#define FilterTagItems(a,b,c)		(((UINT32(*)(pUtilBase, struct TagItem *tagList, Tag *filterArray, UINT32 logic))			_GETVECADDR(UtilBase,14))(UtilBase,a,b,c))
#define FilterTagChanges(a,b,c)		(((void  (*)(pUtilBase, struct TagItem *changeList, const struct TagItem *originalList, BOOL apply))	_GETVECADDR(UtilBase,15))(UtilBase,a,b,c))
#define MapTags(a,b,c)				(((void  (*)(pUtilBase, struct TagItem *tagList, struct TagItem *mapList, UINT32 mapType))	_GETVECADDR(UtilBase,16))(UtilBase,a,b,c))
#define ApplyTagChanges(a,b)		(((void  (*)(pUtilBase, struct TagItem *list, struct TagItem *changelist))					_GETVECADDR(UtilBase,17))(UtilBase,a,b))

#define CalcDate(a,b,c)				(((UINT32(*)(pUtilBase, UINT32 year, UINT32 month, UINT32 day))	_GETVECADDR(UtilBase,18))(UtilBase,a,b,c))
#define Amiga2Date(a,b)				(((void  (*)(pUtilBase, UINT32 amiga, struct ClockData *cd))	_GETVECADDR(UtilBase,19))(UtilBase,a,b))
#define Date2Amiga(a)				(((UINT32(*)(pUtilBase, struct ClockData *cd))					_GETVECADDR(UtilBase,20))(UtilBase,a))
#define CheckDate(a)				(((UINT32(*)(pUtilBase, struct ClockData *date))				_GETVECADDR(UtilBase,21))(UtilBase,a))

#define NamedObjectName(a)			(((STRPTR(*)(pUtilBase, struct NamedObject *nos))									_GETVECADDR(UtilBase,22))(UtilBase,a))
#define FindNamedObject(a,b,c)		(((struct NamedObject *(*)(pUtilBase, struct NamedObject *nameSpace, STRPTR name, struct NamedObject *lastObject)) _GETVECADDR(UtilBase,23))(UtilBase,a,b,c))
#define AddNamedObject(a,b)			(((BOOL(*)(pUtilBase, struct NamedObject *nameSpace, struct NamedObject *object))	_GETVECADDR(UtilBase,24))(UtilBase,a,b))
#define AttemptRemNamedObject(a)	(((BOOL(*)(pUtilBase, struct NamedObject *nos))										_GETVECADDR(UtilBase,25))(UtilBase,a))
#define RemNamedObject(a,b)			(((BOOL(*)(pUtilBase, struct NamedObject *object, struct Message *message))			_GETVECADDR(UtilBase,26))(UtilBase,a,b))
#define ReleaseNamedObject(a)		(((BOOL(*)(pUtilBase, struct NamedObject *object))									_GETVECADDR(UtilBase,27))(UtilBase,a))
#define AllocNamedObjectA(a,b)		(((struct NamedObject *(*)(pUtilBase, STRPTR name, struct TagItem *tagList))		_GETVECADDR(UtilBase,28))(UtilBase,a,b))
#define FreeNamedObject(a)			(((void(*)(pUtilBase, struct NamedObject *object))									_GETVECADDR(UtilBase,29))(UtilBase,a))

#define ToUpper(a)					(((UINT8 (*)(pUtilBase, UINT8 c))					_GETVECADDR(UtilBase,30))(UtilBase,a))
#define ToLower(a)					(((UINT8 (*)(pUtilBase, UINT8 c))					_GETVECADDR(UtilBase,31))(UtilBase,a))

#define Strlen(a)					(((INT32 (*)(pUtilBase, STRPTR ))					_GETVECADDR(UtilBase,32))(UtilBase,a))
#define Strncpy(a,b,c)				(((UINT8*(*)(pUtilBase, STRPTR, STRPTR, INT32 n))	_GETVECADDR(UtilBase,33))(UtilBase,a,b,c))
#define Stricmp(a,b)				(((INT32 (*)(pUtilBase, STRPTR, STRPTR))				_GETVECADDR(UtilBase,34))(UtilBase,a,b))
#define Strnicmp(a,b,c)				(((INT32 (*)(pUtilBase, STRPTR, STRPTR, INT32 n))	_GETVECADDR(UtilBase,35))(UtilBase,a,b,c))
#define Strcmp(a,b)					(((INT32 (*)(pUtilBase, STRPTR, STRPTR))				_GETVECADDR(UtilBase,36))(UtilBase,a,b))
#define Strcpy(a,b)					(((char *(*)(pUtilBase, STRPTR, STRPTR))				_GETVECADDR(UtilBase,37))(UtilBase,a,b))

#define Rand()						(((INT32(*)(pUtilBase))					_GETVECADDR(UtilBase,38))(UtilBase))
#define Random()					(((INT32(*)(pUtilBase))					_GETVECADDR(UtilBase,39))(UtilBase))
#define SRand(a)					(((void (*)(pUtilBase,UINT32 seed))		_GETVECADDR(UtilBase,40))(UtilBase,a))
#define SRandom(a)					(((void (*)(pUtilBase,UINT32 seed))		_GETVECADDR(UtilBase,41))(UtilBase,a))

#define Strchr(a,b)					(((STRPTR(*)(pUtilBase, const STRPTR, INT32))		_GETVECADDR(UtilBase,42))(UtilBase,a,b))
#define Strtok(a,b)					(((STRPTR(*)(pUtilBase, STRPTR, CONST_STRPTR))		_GETVECADDR(UtilBase,43))(UtilBase,a,b))

#define Strtok_r(a,b,c)				(((STRPTR(*)(pUtilBase, STRPTR, CONST_STRPTR, char **))		_GETVECADDR(UtilBase,44))(UtilBase,a,b,c))
#define Strpbrk(a,b)				(((STRPTR(*)(pUtilBase, CONST_STRPTR, CONST_STRPTR))		_GETVECADDR(UtilBase,45))(UtilBase,a,b))
#define Strspn(a,b)					(((INT32(*)(pUtilBase, CONST_STRPTR, CONST_STRPTR))			_GETVECADDR(UtilBase,46))(UtilBase,a,b))

#define Strrchr(a,b)				(((STRPTR(*)(pUtilBase, CONST_STRPTR, INT32))				_GETVECADDR(UtilBase,47))(UtilBase,a,b))
#define Strcspn(a,b)				(((INT32(*)(pUtilBase, CONST_STRPTR, CONST_STRPTR))			_GETVECADDR(UtilBase,48))(UtilBase,a,b))

#define CallHookPkt(a,b,c)			(((INT32(*)(pUtilBase, struct Hook*, APTR, APTR))			_GETVECADDR(UtilBase,49))(UtilBase,a,b,c))
#define Strcat(a,b)					(((STRPTR(*)(pUtilBase, STRPTR, CONST_STRPTR))			_GETVECADDR(UtilBase,50))(UtilBase,a,b))
#define StrDup(a)					(((STRPTR (*)(pUtilBase, CONST_STRPTR ))					_GETVECADDR(UtilBase,51))(UtilBase,a))

#endif

#endif
