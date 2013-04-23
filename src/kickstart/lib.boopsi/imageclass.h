#ifndef INTUITION_IMAGECLASS_H
#define INTUITION_IMAGECLASS_H

#include <tagitem.h>

#define CUSTOMIMAGEDEPTH	(-1)

#define GADGET_BOX( g )	( (struct IBox *) &((struct Gadget *)(g))->LeftEdge )
#define IM_BOX( im )	( (struct IBox *) &((struct Image *)(im))->LeftEdge )
#define IM_FGPEN( im )	( (im)->PlanePick )
#define IM_BGPEN( im )	( (im)->PlaneOnOff )

/******************************************************/
#define IA_Dummy		(TAG_USER + 0x20000)
#define IA_Left			(IA_Dummy + 0x01)
#define IA_Top			(IA_Dummy + 0x02)
#define IA_Width		(IA_Dummy + 0x03)
#define IA_Height		(IA_Dummy + 0x04)
#define IA_FGPen		(IA_Dummy + 0x05)
#define IA_BGPen		(IA_Dummy + 0x06)
#define IA_Data			(IA_Dummy + 0x07)
#define IA_LineWidth		(IA_Dummy + 0x08)
#define IA_Pens			(IA_Dummy + 0x0E)
#define IA_Resolution		(IA_Dummy + 0x0F)

#define IA_APattern		(IA_Dummy + 0x10)
#define IA_APatSize		(IA_Dummy + 0x11)
#define IA_Mode			(IA_Dummy + 0x12)
#define IA_Font			(IA_Dummy + 0x13)
#define IA_Outline		(IA_Dummy + 0x14)
#define IA_Recessed		(IA_Dummy + 0x15)
#define IA_DoubleEmboss		(IA_Dummy + 0x16)
#define IA_EdgesOnly		(IA_Dummy + 0x17)

#define SYSIA_Size		(IA_Dummy + 0x0B)
#define SYSIA_Depth		(IA_Dummy + 0x0C)
#define SYSIA_Which		(IA_Dummy + 0x0D)
#define SYSIA_DrawInfo		(IA_Dummy + 0x18)

#define SYSIA_Pens		IA_Pens
#define IA_ShadowPen		(IA_Dummy + 0x09)
#define IA_HighlightPen		(IA_Dummy + 0x0A)

#define SYSIA_ReferenceFont	(IA_Dummy + 0x19)
#define IA_SupportsDisable	(IA_Dummy + 0x1a)
#define IA_FrameType		(IA_Dummy + 0x1b)

#define SYSISIZE_MEDRES	(0)
#define SYSISIZE_LOWRES	(1)
#define SYSISIZE_HIRES	(2)

#define DEPTHIMAGE	(0x00L)	/* Window depth gadget image */
#define ZOOMIMAGE	(0x01L)	/* Window zoom gadget image */
#define SIZEIMAGE	(0x02L)	/* Window sizing gadget image */
#define CLOSEIMAGE	(0x03L)	/* Window close gadget image */
#define SDEPTHIMAGE	(0x05L)	/* Screen depth gadget image */
#define LEFTIMAGE	(0x0AL)	/* Left-arrow gadget image */
#define UPIMAGE		(0x0BL)	/* Up-arrow gadget image */
#define RIGHTIMAGE	(0x0CL)	/* Right-arrow gadget image */
#define DOWNIMAGE	(0x0DL)	/* Down-arrow gadget image */
#define CHECKIMAGE	(0x0EL)	/* GadTools checkbox image */
#define MXIMAGE		(0x0FL)	/* GadTools mutual exclude "button" image */
#define	MENUCHECK	(0x10L)	/* Menu checkmark image */
#define AMIGAKEY	(0x11L)	/* Menu Amiga-key image */

#define FRAME_DEFAULT		0
#define FRAME_BUTTON		1
#define FRAME_RIDGE		2
#define FRAME_ICONDROPBOX	3

#define    IM_DRAW	0x202L	/* draw yourself, with "state"		*/
#define    IM_HITTEST	0x203L	/* return TRUE if click hits image	*/
#define    IM_ERASE	0x204L	/* erase yourself			*/
#define    IM_MOVE	0x205L	/* draw new and erase old, smoothly	*/

#define    IM_DRAWFRAME	0x206L	/* draw with specified dimensions	*/
#define    IM_FRAMEBOX	0x207L	/* get recommended frame around some box*/
#define    IM_HITFRAME	0x208L	/* hittest with dimensions		*/
#define    IM_ERASEFRAME 0x209L	/* hittest with dimensions		*/

#define    IDS_NORMAL		(0L)
#define    IDS_SELECTED		(1L)	/* for selected gadgets	    */
#define    IDS_DISABLED		(2L)	/* for disabled gadgets	    */
#define	   IDS_BUSY		(3L)	/* for future functionality */
#define    IDS_INDETERMINATE	(4L)	/* for future functionality */
#define    IDS_INACTIVENORMAL	(5L)	/* normal, in inactive window border */
#define    IDS_INACTIVESELECTED	(6L)	/* selected, in inactive border */
#define    IDS_INACTIVEDISABLED	(7L)	/* disabled, in inactive border */
#define	   IDS_SELECTEDDISABLED (8L)	/* disabled and selected    */

/* IM_FRAMEBOX	*/
struct impFrameBox {
    UINT32		MethodID;
    struct IBox	*imp_ContentsBox;	/* input: relative box of contents */
    struct IBox	*imp_FrameBox;		/* output: rel. box of encl frame  */
    struct DrawInfo	*imp_DrInfo;	/* NB: May be NULL */
    UINT32	imp_FrameFlags;
};

#define FRAMEF_SPECIFY	(1<<0)

/* IM_DRAW, IM_DRAWFRAME	*/
struct impDraw {
    UINT32		MethodID;
    struct CRastPort	*imp_RPort;
    struct {
	INT16	X;
	INT16	Y;
    } imp_Offset;

    UINT32		imp_State;
    struct DrawInfo	*imp_DrInfo;

    /* these parameters only valid for IM_DRAWFRAME */
    struct {
	INT16	Width;
	INT16	Height;
    } imp_Dimensions;
};

/* IM_ERASE, IM_ERASEFRAME	*/
/* NOTE: This is a subset of impDraw	*/
struct impErase {
    UINT32		MethodID;
    struct CRastPort	*imp_RPort;
    struct {
	INT16	X;
	INT16	Y;
    } imp_Offset;

    /* these parameters only valid for IM_ERASEFRAME */
    struct {
	INT16	Width;
	INT16	Height;
    }	imp_Dimensions;
};

/* IM_HITTEST, IM_HITFRAME	*/
struct impHitTest {
    UINT32		MethodID;
    struct {
	INT16	X;
	INT16	Y;
    } imp_Point;

    /* these parameters only valid for IM_HITFRAME */
    struct {
	INT16	Width;
	INT16	Height;
    } imp_Dimensions;
};

#endif
