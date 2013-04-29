#include "boopsibase.h"
#include "tagitem.h"
#include "boopsi_internal.h"
#include "imageclass.h"
#include "vectorclass.h"
#include "exec_funcs.h"
#include "utility_funcs.h"

#define SysBase BOOPSIBase->SysBase
#define UtilBase BOOPSIBase->UtilBase

struct SysIData {
    UINT16	*si_ImageData[VS_NUMSTATES];/* ImageData for each visual state */
    UINT16	si_Depth;		/* Depth of image */
    UINT8	si_States;		/* Visual states available */
};

/* class work area */
struct SysIClassData {
    struct SignalSemaphore scd_Sem;	/* Semaphore for lockout */

    UINT16 scd_Width;		/* Width of work area */
    UINT16 scd_Height;		/* Height of work area */
    UINT16 scd_Depth;		/* Depth of work area */

//    struct BitMap scd_BM;	/* Image's BitMap */
//    struct RastPort scd_RP;	/* Image's RastPort */

//    struct TmpRas scd_TmpRas;	/* Flood Fill TmpRas */
//    UINT8 *scd_TmpRasPlane;	/* Flood Fill workspace */

//    struct AreaInfo scd_Area;	/* Area Fill AreaInfo */
//    INT16	scd_Array[ AREA_WORDSIZE ];	/* Area Fill workspace */
};

static UINT32 dispatchSysI(pBOOPSI BOOPSIBase, Class *cl, Object *o, Msg msg);

Class *initSysIClass(pBOOPSI BOOPSIBase)
{
    Class *cl = NULL;
    struct SysIClassData *scd;
    extern CSTRPTR SysIClassName;
    extern CSTRPTR ImageClassName;
	scd = (struct SysIClassData *) AllocVec( sizeof( struct SysIClassData ), MEMF_CLEAR | MEMF_PUBLIC );
    if ( scd )
    {
		cl = makePublicClass( BOOPSIBase, (ClassID)SysIClassName, (ClassID)ImageClassName, sizeof(struct SysIData), (HOOKFUNC)dispatchSysI );
    	if ( cl )
		{
			InitSemaphore( &scd->scd_Sem );
			cl->cl_UserData = (UINT32) scd;
			return( cl );
		} else
		{
	    FreeVec( scd );
		}
    }
    return cl;
}

static UINT32 dispatchSysI(pBOOPSI BOOPSIBase, Class *cl, Object *o, Msg msg)
{
#if 0
    struct TagItem *tags;
    UINT32	size;
    INT32	width, height;
    UINT32	which;
    struct VectorInfo *vi;
    struct DrawInfo *dri;
#endif
    Object *newobj = NULL;

    switch ( msg->MethodID )
    {
    case OM_NEW:
//		tags = ((struct opSet *) msg)->ops_AttrList;
//		which = GetTagData( SYSIA_Which| TAG_USER, ~0, tags );
#if 0
	    if ( !createStateImagery(
		    IM(newobj),			/* Object is an image */
		    INST_DATA(cl, newobj),	/* Its instance data */
		    (struct SysIClassData *)cl->cl_UserData,	/* Class data */
		    vi, width, height, dri ) )
	    {
			CoerceMessage(cl, newobj, OM_DISPOSE);
			return  NULL;
	    }
#endif
		return (UINT32)newobj;

    case IM_DRAW:
		return (UINT32) NULL; //drawSysI( cl, o, (struct impDraw *)msg );

    case OM_DISPOSE:
		//freeImageMem( (struct SysIData *)INST_DATA(cl,o) );
    default:
		return (UINT32)SSM(cl, o, msg);
    }
}

