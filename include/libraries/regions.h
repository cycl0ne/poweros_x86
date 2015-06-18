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
} Rect, *pRect;

/* static clip rectangle: drawing allowed if point within rectangle*/
typedef struct ClipRect {
	INT32 	x;		/* x coordinate of top left corner */
	INT32 	y;		/* y coordinate of top left corner */
	INT32 	width;		/* width of rectangle */
	INT32 	height;		/* height of rectangle */
} ClipRect, *pClipRect;

/* dynamically allocated multi-rectangle clipping region*/
typedef struct ClipRegion {
	INT32	size;		/* malloc'd # of rectangles*/
	INT32	numRects;	/* # rectangles in use*/
	INT32	type; 		/* region type*/
	pRect 	rects;		/* rectangle array*/
	Rect	extents;	/* bounding box of region*/
} ClipRegion, *pClipRegion;

#endif

