#include "coregfx.h"
#include "rastport.h"
#include "regions.h"
#include "pixmap.h"
#include "exec_funcs.h"
#include "coregfx_funcs.h"

#define SysBase CoreGfxBase->SysBase

void set_ts_origin(CRastPort *rp, int x, int y);
void ts_drawpoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y);
void ts_drawrow(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 x2, INT32 y);
void drawrow(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 x2, INT32 y);

static inline void drawpoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y) { if (ClipPoint(rp, x, y)) rp->crp_PixMap->_DrawPixel(rp, x, y, rp->crp_Foreground); }

/**
 * Draw a polygon in the foreground color, applying clipping if necessary.
 * The polygon is only closed if the first point is repeated at the end.
 * Some care is taken to plot the endpoints correctly if the current
 * drawing mode is XOR.  However, internal crossings are not handled
 * correctly.
 *
 * @param psd Drawing surface.
 * @param count Number of points in polygon.
 * @param points The array of points.
 */
void cgfx_Poly(CoreGfxBase *CoreGfxBase, struct CRastPort *rp, int count, CGfxPoint *points)
{
  INT32 firstx;
  INT32 firsty;
  BOOL didline;

  if (count < 2) return;
  firstx = points->x;
  firsty = points->y;
  didline = FALSE;

  while (count-- > 1) {
	if (didline && (rp->crp_Mode == ROP_XOR)) drawpoint(CoreGfxBase, rp, points->x, points->y);
	/* note: change to drawline*/
	Line(rp, points[0].x, points[0].y, points[1].x, points[1].y, TRUE);
	points++;
	didline = TRUE;
  }
  if (rp->crp_Mode == ROP_XOR) {
	  points--;
	  if (points->x == firstx && points->y == firsty) drawpoint(CoreGfxBase, rp, points->x, points->y);
  }
  FixCursor(rp->crp_PixMap);
}

#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}

/*
 *     Find the index of the point with the smallest y.
 */
static int getPolyYBounds(CGfxPoint *pts, int n, int *by, int *ty)
{
    CGfxPoint *ptMin;
    int ymin, ymax;
    CGfxPoint *ptsStart = pts;

    ptMin = pts;
    ymin = ymax = (pts++)->y;

    while (--n > 0) {
        if (pts->y < ymin)
	{
            ptMin = pts;
            ymin = pts->y;
        }
	if(pts->y > ymax)
            ymax = pts->y;

        pts++;
    }

    *by = ymin;
    *ty = ymax;
    return(ptMin-ptsStart);
}

static int getPolyXBounds(CGfxPoint *pts, int n, int *bx, int *tx)
{
    CGfxPoint *ptMin;
    int xmin, xmax;
    CGfxPoint *ptsStart = pts;

    ptMin = pts;
    xmin = xmax = (pts++)->x;

    while (--n > 0) {
        if (pts->x < xmin)
	{
            ptMin = pts;
            xmin = pts->x;
        }
	if(pts->x > xmax)
            xmax = pts->x;

        pts++;
    }

    *bx = xmin;
    *tx = xmax;
    return(ptMin-ptsStart);
}

void cgfx_FillPoly(CoreGfxBase *CoreGfxBase, CRastPort *rp, int count, CGfxPoint *pointtable)
{
    INT32 xl = 0, xr = 0;     /* x vals of left and right edges */
    int dl = 0, dr = 0;         /* decision variables             */
    int ml = 0, m1l = 0;        /* left edge slope and slope+1    */
    int mr = 0, m1r = 0;        /* right edge slope and slope+1   */
    int incr1l = 0, incr2l = 0; /* left edge error increments     */
    int incr1r = 0, incr2r = 0; /* right edge error increments    */
    int dy;                     /* delta y                        */
    INT32 y;                  /* current scanline               */
    int left, right;            /* indices to first endpoints     */
    int i;                      /* loop counter                   */
    int nextleft, nextright;    /* indices to second endpoints    */
    CGfxPoint *ptsOut, *FirstPoint;/* output buffer                 */
    INT32 *width, *FirstWidth;/* output buffer                  */
    int imin;                   /* index of smallest vertex (in y)*/
    int ymin;                   /* y-extents of polygon           */
    int ymax;

    /*
     *  find leftx, bottomy, rightx, topy, and the index
     *  of bottomy.
     */

    imin = getPolyYBounds(pointtable, count, &ymin, &ymax);

    if (rp->crp_FillMode != FILL_SOLID) {
      int xmin, xmax;
      getPolyXBounds(pointtable, count, &xmin, &xmax);
      set_ts_origin(rp, xmin, ymin);
    }

    dy = ymax - ymin + 1;
    if ((count < 3) || (dy < 0)) return;
    ptsOut = FirstPoint	= (CGfxPoint *)AllocVec(sizeof(CGfxPoint) * dy, MEMF_FAST);
    width = FirstWidth	= (INT32 *)AllocVec(sizeof(INT32) * dy, MEMF_FAST);
    if(!FirstPoint || !FirstWidth)
    {
		if (FirstWidth) FreeVec(FirstWidth);
		if (FirstPoint) FreeVec(FirstPoint);
		return;
    }

    nextleft = nextright = imin;
    y = pointtable[nextleft].y;

    /*
     *  loop through all edges of the polygon
     */
    do {
        /*
         *  add a left edge if we need to
         */
        if (pointtable[nextleft].y == y) 
        {
            left = nextleft;

            /*
             *  find the next edge, considering the end
             *  conditions of the array.
             */
            nextleft++;
            if (nextleft >= count) nextleft = 0;

            /*
             *  now compute all of the random information
             *  needed to run the iterative algorithm.
             */
            BRESINITPGON(pointtable[nextleft].y-pointtable[left].y,
                         pointtable[left].x,pointtable[nextleft].x,
                         xl, dl, ml, m1l, incr1l, incr2l);
        }

        /*
         *  add a right edge if we need to
         */
        if (pointtable[nextright].y == y) 
        {
            right = nextright;

            /*
             *  find the next edge, considering the end
             *  conditions of the array.
             */
            nextright--;
            if (nextright < 0) nextright = count-1;

            /*
             *  now compute all of the random information
             *  needed to run the iterative algorithm.
             */
            BRESINITPGON(pointtable[nextright].y-pointtable[right].y,
                         pointtable[right].x,pointtable[nextright].x,
                         xr, dr, mr, m1r, incr1r, incr2r);
        }

        /*
         *  generate scans to fill while we still have
         *  a right edge as well as a left edge.
         */
        i = MIN(pointtable[nextleft].y, pointtable[nextright].y) - y;
		/* in case we're called with non-convex polygon */
		if(i < 0)
		{
			FreeVec(FirstWidth);
			FreeVec(FirstPoint);
			return;
		}
        while (i-- > 0) 
        {
            ptsOut->y = y;

            /*
             *  reverse the edges if necessary
             */
            if (xl < xr) 
            {
                *(width++) = xr - xl;
                (ptsOut++)->x = xl;
            }
            else 
            {
                *(width++) = xl - xr;
                (ptsOut++)->x = xr;
            }
            y++;

            /* increment down the edges */
            BRESINCRPGON(dl, xl, ml, m1l, incr1l, incr2l);
            BRESINCRPGON(dr, xr, mr, m1r, incr1r, incr2r);
        }
    }  while (y != ymax);

    /*
     * Finally, fill the spans
     */
    i = ptsOut-FirstPoint;
    ptsOut = FirstPoint;
    width = FirstWidth;
    while (--i >= 0) 
    {
		/* calc x extent from width*/
		int e = *width++ - 1;
		if (e >= 0) 
		{
		  if (rp->crp_FillMode != FILL_SOLID) 
			ts_drawrow(CoreGfxBase, rp, ptsOut->x, ptsOut->x + e, ptsOut->y);
		   else
			 drawrow(CoreGfxBase, rp, ptsOut->x, ptsOut->x + e, ptsOut->y);
		}
	++ptsOut;
    }

    FreeVec(FirstWidth);
    FreeVec(FirstPoint);
    FixCursor(rp->crp_PixMap);
}
