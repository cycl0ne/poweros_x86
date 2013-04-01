#ifndef LAYERS_H
#define LAYERS_H
#include "types.h"
#include "rastport.h"

typedef struct Rectangle
{
    INT16   MinX, MinY;
    INT16   MaxX, MaxY;
} Rectangle;

typedef struct Rect32
{
    INT32	MinX, MinY;
    INT32	MaxX, MaxY;
} Rect32;

typedef struct tPoint
{
    INT16 x,y;
} Point;

typedef struct RegionRectangle
{
	struct RegionRectangle *Next,*Prev;
	Rectangle bounds;
} RegionRectangle;

typedef struct Region
{
	Rectangle bounds;
	RegionRectangle *RegionRectangle;
} Region;

typedef struct ClipRect
{
	struct ClipRect *Next;
	struct ClipRect *prev;
	struct  Layer   *lobs;
	PixMap			*BitMap;
	Rectangle   	bounds;
	void    		*_p1;
	void    		*_p2;
	INT32    		reserved;
	INT32    		Flags;
} ClipRect;


typedef struct Layer {
	struct Layer		*front,*back;   
	ClipRect			*ClipRect;  /* read by roms to find first cliprect */
	struct CRastPort	*rp;        
	Rectangle   		bounds;    
	UINT8		reserved[4];
	UINT16		priority;               /* system use only */
	UINT16		Flags;                  /* obscured ?, Virtual BitMap? */
	PixMap		*SuperBitMap;
	ClipRect	*SuperClipRect; /* super bitmap cliprects if VBitMap != 0*/
								  /* else damage cliprect list for refresh */
	APTR    Window;               /* reserved for user interface use */
	INT16   Scroll_X,Scroll_Y;
	ClipRect *cr,*cr2,*crnew;	/* used by dedice */
	ClipRect *SuperSaveClipRects; /* preallocated cr's */
	ClipRect *_cliprects;   	/* system use during refresh */
	struct  Layer_Info  *LayerInfo; 	/* points to head of the list */
	struct  SignalSemaphore Lock;
	struct  Hook *BackFill;
	UINT32  reserved1;
	Region	*ClipRegion;
	Region	*saveClipRects;   	/* used to back out when in trouble*/
	INT16   Width,Height;		/* system use */
	UINT8   reserved2[18];
	/* this must stay here */
    Region	*DamageList;    /* list of rectangles to refresh */
} Layer;


#endif
