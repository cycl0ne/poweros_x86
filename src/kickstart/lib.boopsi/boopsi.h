#ifndef boopsi_h
#define boopsi_h

#include "types.h"

typedef UINT32	Object;
typedef	UINT8	*ClassID;
typedef struct { 
    UINT32 MethodID;
}*Msg;


#define ROOTCLASS		"rootclass"
#define IMAGECLASS		"imageclass"
#define FRAMEICLASS		"frameiclass"
#define SYSICLASS		"sysiclass"
#define FILLRECTCLASS	"fillrectclass"
#define GADGETCLASS		"gadgetclass"
#define PROPGCLASS		"propgclass"
#define STRGCLASS		"strgclass"	
#define BUTTONGCLASS	"buttongclass"
#define FRBUTTONCLASS	"frbuttonclass"
#define GROUPGCLASS		"groupgclass"
#define ICCLASS			"icclass"
#define MODELCLASS		"modelclass"
#define ITEXTICLASS		"itexticlass"
#define POINTERCLASS	"pointerclass"

/* OM_NEW and OM_SET	*/
struct opSet {
    UINT32				MethodID;
    struct TagItem		*ops_AttrList;
    struct GadgetInfo	*ops_GInfo;
};

/* OM_NOTIFY, and OM_UPDATE	*/
struct opUpdate {
    UINT32				MethodID;
    struct TagItem		*opu_AttrList;
    struct GadgetInfo	*opu_GInfo;	
    UINT32				opu_Flags;
};

#define OPUF_INTERIM	(1<<0)

/* OM_GET	*/
struct opGet {
    UINT32				MethodID;
    UINT32				opg_AttrID;
    UINT32				*opg_Storage;
};

/* OM_ADDTAIL	*/
struct opAddTail {
    UINT32				MethodID;
    struct List			*opat_List;
};

/* OM_ADDMEMBER, OM_REMMEMBER	*/
#define  opAddMember opMember
struct opMember {
    UINT32				MethodID;
    Object				*opam_Object;
};

#endif
