#ifndef REGIONS_H
#define REGIONS_H
#include "types.h"

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

