#ifndef REGIONS_H
#define REGIONS_H
#include "types.h"

/* region types */
#define REGION_ERROR	0
#define REGION_NULL		1
#define REGION_SIMPLE	2
#define REGION_COMPLEX	3

/* RectInRegion return codes*/
#define RECT_OUT	0	/* rectangle not in region*/
#define RECT_ALLIN	1	/* rectangle all in region*/
#define RECT_PARTIN	2	/* rectangle partly in region*/

#define	MIN_COORD	((INT32) -32768)	/* minimum coordinate value */
#define	MAX_COORD	((INT32) 32767)	/* maximum coordinate value */

/* REGRECT used in region routines*/
typedef struct Rect {
	INT32	left;
	INT32	top;
	INT32	right;
	INT32	bottom;
} Rect;

/* static clip rectangle: drawing allowed if point within rectangle*/
typedef struct ClipRect {
	INT32 	x;		/* x coordinate of top left corner */
	INT32 	y;		/* y coordinate of top left corner */
	INT32 	width;		/* width of rectangle */
	INT32 	height;		/* height of rectangle */
} ClipRect;

/* dynamically allocated multi-rectangle clipping region*/
typedef struct ClipRegion {
	INT32	size;		/* malloc'd # of rectangles*/
	INT32	numRects;	/* # rectangles in use*/
	INT32	type; 		/* region type*/
	Rect 	*rects;		/* rectangle array*/
	Rect	extents;	/* bounding box of region*/
} ClipRegion;

#if 0
// Org. AOS Structures
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
#endif

#endif

