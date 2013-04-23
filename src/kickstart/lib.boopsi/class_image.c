#include "boopsibase.h"
#include "tagitem.h"
#include "pack.h"
#include "boopsi_internal.h"
#include "imageclass.h"
#include "exec_funcs.h"
#include "utility_funcs.h"
#include "coregfx_funcs.h"

#define SysBase BOOPSIBase->SysBase
#define UtilBase BOOPSIBase->UtilBase
#define CoreGfxBase BOOPSIBase->CoreGfxBase

static UINT32	imageDispatch(pBOOPSI BOOPSIBase, Class *cl, Object *o, Msg msg);
static UINT32 getImageAttr(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct opGet *msg);
static INT32 hittestImage(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impHitTest *msg);
static UINT32 eraseImage(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impErase *msg);
static UINT32 drawImage( pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impDraw *msg);

struct Image
{
    INT16 LeftEdge;
    INT16 TopEdge;
    INT16 Width;
    INT16 Height;
    INT16 Depth;
    UINT16 *ImageData;
    UINT32 PlanePick, PlaneOnOff;
    struct Image *NextImage;
};

struct IIData {
    struct Image	iid_Image;
};

UINT32 imagePackTable[] =
{
    PACK_STARTTABLE( IA_Dummy ),
    PACK_ENTRY( IA_Dummy, IA_Left,	Image, LeftEdge,	PKCTRL_WORD ),
    PACK_ENTRY( IA_Dummy, IA_Top,	Image, TopEdge,		PKCTRL_WORD ),
    PACK_ENTRY( IA_Dummy, IA_Width,	Image, Width,		PKCTRL_WORD ),
    PACK_ENTRY( IA_Dummy, IA_Height,Image, Height,		PKCTRL_WORD ),
    PACK_ENTRY( IA_Dummy, IA_FGPen,	Image, PlanePick,	PKCTRL_ULONG),
    PACK_ENTRY( IA_Dummy, IA_BGPen,	Image, PlaneOnOff,	PKCTRL_ULONG),
    PACK_ENTRY( IA_Dummy, IA_Data,	Image, ImageData,	PKCTRL_ULONG),
    PACK_ENDTABLE,    
};

#define setImageAttrs( cl, o, msg ) \
    PackStructureTags( IM(o), imagePackTable, ((struct opSet *)msg)->ops_AttrList )

#define IM(o)	((struct Image *)(o))

Class	*initImageClass(pBOOPSI BOOPSIBase)
{
    extern CSTRPTR	ImageClassName;
    extern CSTRPTR	RootClassName;
    return (makePublicClass(BOOPSIBase, (ClassID)ImageClassName, (ClassID)RootClassName, sizeof(struct IIData), (HOOKFUNC)imageDispatch));
}

static UINT32 imageDispatch(pBOOPSI BOOPSIBase, Class *cl, Object *o, Msg msg)
{
    Object  *newobj;


    switch ( msg->MethodID  )
    {
		case OM_NEW:
		newobj = (Object *) SSM( cl, o, msg );
		if ( newobj )
		{
			o = (Object *)&((struct IIData *) INST_DATA( cl, newobj ))->iid_Image;
			IM(o)->Depth = 32;
			IM(o)->Width = 80;
			IM(o)->Height = 40;
			PackStructureTags( IM(o), imagePackTable, ((struct opSet *)msg)->ops_AttrList );
		}
		return ( (UINT32) newobj );

		case OM_SET:
			PackStructureTags( IM(o), imagePackTable, ((struct opSet *)msg)->ops_AttrList );
			return ( (UINT32) 1 );

		case OM_GET:
			return ( getImageAttr(BOOPSIBase, cl, o, (struct opGet *)msg  ) );

		case IM_DRAW:
		case IM_DRAWFRAME:
			return ( drawImage(BOOPSIBase, cl, o, (struct impDraw *)msg ) );

		case IM_HITFRAME:
		case IM_HITTEST:
			return ( hittestImage(BOOPSIBase, cl, o, (struct impHitTest *)msg ) );

		case IM_ERASE:
		case IM_ERASEFRAME:
			return ( eraseImage(BOOPSIBase, cl, o, (struct impErase *)msg ) );
			
		case OM_DISPOSE:
		default:
		return ( SSM( cl, o, msg ) );
    }
}

static UINT32 drawImage( pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impDraw *msg)
{
    UINT32	retval = 1;

    if ( msg->MethodID == IM_DRAWFRAME )
    {
		msg->MethodID = IM_DRAW;
		retval =  SM( o, (Msg)msg );
		msg->MethodID = IM_DRAWFRAME;
		return ( retval );
    }

//void cgfx_Blit(CoreGfxBase *CoreGfxBase, CRastPort *dstrp, INT32 dstx, INT32 dsty, INT32 width, INT32 height, CRastPort *srcrp, INT32 srcx, INT32 srcy, int rop)
	
//!    drawImageGrunt( msg->imp_RPort, IM(o), msg->imp_Offset.X, msg->imp_Offset.Y, 0x00C0 );
    return retval;
}

static UINT32 eraseImage(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impErase *msg)
{
    unsigned int	width, height;
    unsigned int	left, top;

    width	= IM(o)->Width;
    height	= IM(o)->Height;

    if ( msg->MethodID == IM_ERASEFRAME )
    {
		width = msg->imp_Dimensions.Width;
		height = msg->imp_Dimensions.Height;
    }

    left	= IM(o)->LeftEdge + msg->imp_Offset.X;
    top		= IM(o)->TopEdge + msg->imp_Offset.Y;

    FillRect( msg->imp_RPort, left, top, left + width - 1, top + height - 1 );
    return(1);
}

static INT32 hittestImage(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct impHitTest *msg)
{
    struct IBox	box;
    box = *IM_BOX( IM(o) );

    if ( msg->MethodID == IM_HITFRAME )
    {
		box.Width = msg->imp_Dimensions.Width;
		box.Height = msg->imp_Dimensions.Height;
    }
    return ( (INT32)ptInBox( (Point*)&msg->imp_Point, &box ) );
}

static UINT32 getImageAttr(pBOOPSI BOOPSIBase, Class *cl, Object *o, struct opGet *msg)
{
    struct TagItem gettags[2];
    gettags[0].ti_Tag = msg->opg_AttrID;
    gettags[0].ti_Data = (UINT32) msg->opg_Storage;
    gettags[1].ti_Tag = TAG_DONE;

    return (UnpackStructureTags( IM(o), (UINT32 *)imagePackTable, gettags ) );
}


