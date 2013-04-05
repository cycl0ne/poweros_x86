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

static inline void drawpoint(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x, INT32 y)
{
	if (ClipPoint(rp, x, y)) rp->crp_PixMap->DrawPixel(rp, x, y, rp->crp_Foreground);
}

void drawrow(CoreGfxBase *CoreGfxBase, CRastPort *rp, INT32 x1, INT32 x2, INT32 y);

typedef struct {
	CoreGfxBase	*_CoreGfxBase;
	CRastPort *psd;
	INT32	x0, y0;		/* center*/
	INT32	rx, ry;		/* radii*/
	INT32	ax, ay;		/* start point*/
	INT32	bx, by;		/* end point*/
	int	adir;		/* start pt: 1=bottom half, -1=top half*/
	int	bdir;		/* end pt:  -1=bottom half,  1=top half*/
	int	type;		/* MWARC, MWARCOUTLINE, MWPIE, MWELLIPSE etc*/
} SLICE;

/* integer sin/cos tables*/
static short icos[360] = {
  1024, 1023, 1023, 1022, 1021, 1020, 1018, 1016, 1014, 1011,
  1008, 1005, 1001, 997, 993, 989, 984, 979, 973, 968,
  962, 955, 949, 942, 935, 928, 920, 912, 904, 895,
  886, 877, 868, 858, 848, 838, 828, 817, 806, 795,
  784, 772, 760, 748, 736, 724, 711, 698, 685, 671,
  658, 644, 630, 616, 601, 587, 572, 557, 542, 527,
  512, 496, 480, 464, 448, 432, 416, 400, 383, 366,
  350, 333, 316, 299, 282, 265, 247, 230, 212, 195,
  177, 160, 142, 124, 107, 89, 71, 53, 35, 17,
  0, -17, -35, -53, -71, -89, -107, -124, -142, -160,
  -177, -195, -212, -230, -247, -265, -282, -299, -316, -333,
  -350, -366, -383, -400, -416, -432, -448, -464, -480, -496,
  -512, -527, -542, -557, -572, -587, -601, -616, -630, -644,
  -658, -671, -685, -698, -711, -724, -736, -748, -760, -772,
  -784, -795, -806, -817, -828, -838, -848, -858, -868, -877,
  -886, -895, -904, -912, -920, -928, -935, -942, -949, -955,
  -962, -968, -973, -979, -984, -989, -993, -997, -1001, -1005,
  -1008, -1011, -1014, -1016, -1018, -1020, -1021, -1022, -1023, -1023,
  -1024, -1023, -1023, -1022, -1021, -1020, -1018, -1016, -1014, -1011,
  -1008, -1005, -1001, -997, -993, -989, -984, -979, -973, -968,
  -962, -955, -949, -942, -935, -928, -920, -912, -904, -895,
  -886, -877, -868, -858, -848, -838, -828, -817, -806, -795,
  -784, -772, -760, -748, -736, -724, -711, -698, -685, -671,
  -658, -644, -630, -616, -601, -587, -572, -557, -542, -527,
  -512, -496, -480, -464, -448, -432, -416, -400, -383, -366,
  -350, -333, -316, -299, -282, -265, -247, -230, -212, -195,
  -177, -160, -142, -124, -107, -89, -71, -53, -35, -17,
  0, 17, 35, 53, 71, 89, 107, 124, 142, 160,
  177, 195, 212, 230, 247, 265, 282, 299, 316, 333,
  350, 366, 383, 400, 416, 432, 448, 464, 480, 496,
  512, 527, 542, 557, 572, 587, 601, 616, 630, 644,
  658,
  671, 685, 698, 711, 724, 736, 748, 760, 772, 784,
  795, 806, 817, 828, 838, 848, 858, 868, 877, 886,
  895, 904, 912, 920, 928, 935, 942, 949, 955, 962,
  968, 973, 979, 984, 989, 993, 997, 1001, 1005, 1008,
  1011, 1014, 1016, 1018, 1020, 1021, 1022, 1023, 1023
  };

static short isin[360] = {
  0, 17, 35, 53, 71, 89, 107, 124, 142, 160,
  177, 195, 212, 230, 247, 265, 282, 299, 316, 333,
  350, 366, 383, 400, 416, 432, 448, 464, 480, 496,
  512, 527, 542, 557, 572, 587, 601, 616, 630, 644,
  658, 671, 685, 698, 711, 724, 736, 748, 760, 772,
  784, 795, 806, 817, 828, 838, 848, 858, 868, 877,
  886, 895, 904, 912, 920, 928, 935, 942, 949, 955,
  962, 968, 973, 979, 984, 989, 993, 997, 1001, 1005,
  1008, 1011, 1014, 1016, 1018, 1020, 1021, 1022, 1023, 1023,
  1024, 1023, 1023, 1022, 1021, 1020, 1018, 1016, 1014, 1011,
  1008, 1005, 1001, 997, 993, 989, 984, 979, 973, 968,
  962, 955, 949, 942, 935, 928, 920, 912, 904, 895,
  886, 877, 868, 858, 848, 838, 828, 817, 806, 795,
  784, 772, 760, 748, 736, 724, 711, 698, 685, 671,
  658, 644, 630, 616, 601, 587, 572, 557, 542, 527,
  512, 496, 480, 464, 448, 432, 416, 400, 383, 366,
  350, 333, 316, 299, 282, 265, 247, 230, 212, 195,
  177, 160, 142, 124, 107, 89, 71, 53, 35, 17,
  0, -17, -35, -53, -71, -89, -107, -124, -142, -160,
  -177, -195, -212, -230, -247, -265, -282, -299, -316, -333,
  -350, -366, -383, -400, -416, -432, -448, -464, -480, -496,
  -512, -527, -542, -557, -572, -587, -601, -616, -630, -644,
  -658, -671, -685, -698, -711, -724, -736, -748, -760, -772,
  -784, -795, -806, -817, -828, -838, -848, -858, -868, -877,
  -886, -895, -904, -912, -920, -928, -935, -942, -949, -955,
  -962, -968, -973, -979, -984, -989, -993, -997, -1001, -1005,
  -1008, -1011, -1014, -1016, -1018, -1020, -1021, -1022, -1023, -1023,
  -1024, -1023, -1023, -1022, -1021, -1020, -1018, -1016, -1014, -1011,
  -1008, -1005, -1001, -997, -993, -989, -984, -979, -973, -968,
  -962, -955, -949, -942, -935, -928, -920, -912, -904, -895,
  -886, -877, -868, -858, -848, -838, -828, -817, -806, -795,
  -784, -772, -760, -748, -736, -724, -711, -698, -685, -671,
  -658, -644, -630, -616, -601, -587, -572, -557, -542, -527,
  -512, -496, -480, -464, -448, -432, -416, -400, -383, -366,
  -350, -333, -316, -299, -282, -265, -247, -230, -212, -195,
  -177, -160, -142, -124, -107, -89, -71, -53, -35, -17
};

void cgfx_ArcAngle(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry, INT32 angle1, INT32 angle2, int type)
{
	int s = angle1 / 64;
	int e = angle2 / 64;
	int x = 0, y = 0;
	int fx = 0, fy = 0;
	int lx = 0, ly = 0;
	int i;
	CGfxPoint	pts[3];

	if ((s% 360) == (e % 360)) {
		s = 0;
		e = 360;
	} else {
		if (s > 360)
			s %= 360;
		if (e > 360)
			e %= 360;
		while (s < 0)
			s += 360;
		while (e < s)
			e += 360;
		if (s == e) {
			s = 0;
			e = 360;
		}
	}

	/* generate arc points*/
	for (i = s; i <= e; ++i) {
		/* add 1 to rx/ry to smooth small radius arcs*/
		x = ((long)  icos[i % 360] * (long) (rx + 1) / 1024) + x0;
		y = ((long) -isin[i % 360] * (long) (ry + 1) / 1024) + y0;
		if (i != s) {
			if (type == PIE) {
				/* use poly fill for expensive filling!*/
				pts[0].x = lx;
				pts[0].y = ly;
				pts[1].x = x;
				pts[1].y = y;
				pts[2].x = x0;
				pts[2].y = y0;
				/* note: doesn't handle patterns... FIXME*/
				FillPoly(psd, 3, pts);
			} else	/* MWARC*/
				Line(psd, lx, ly, x, y, TRUE);
		} else {
			fx = x;
			fy = y;
		}
		lx = x;
		ly = y;
	}

	if (type & OUTLINE) 
	{
		/* draw two lines from center to arc endpoints*/
		Line(psd, x0, y0, fx, fy, TRUE);
		Line(psd, x0, y0, lx, ly, TRUE);
	}
	FixCursor(psd);
}
#define CoreGfxBase slice->_CoreGfxBase
static int
clip_line(SLICE *slice, INT32 xe, INT32 ye, int dir, INT32 y, INT32 *x0,
	INT32 *x1)
{
#if 1
	/*
	 * kluge: handle 180 degree case
	 */
	if (y >= 0 && ye == 0) {
/*printf("cl %d,%d %d,%d %d,%d %d,%d %d,%d\n", xe, ye, y, dir,
slice->ax, slice->ay, slice->bx, slice->by, slice->adir, slice->bdir);*/
		/* bottom 180*/
		if (slice->adir < 0) {
			if (slice->ay || slice->by)
				return 1;
			if (slice->ax == -slice->bx)
				return 0;
		}
		return 3;
	}
#endif
	/* hline on the same vertical side with the given edge? */
	if ((y >= 0 && ye >= 0) || (y < 0 && ye < 0)) {
		INT32 x;

		if (ye == 0) x = xe; else
		x = (INT32)(long)xe * y / ye;

		if (x >= *x0 && x <= *x1) {
			if (dir > 0)
				*x0 = x;
			else
				*x1 = x;
			return 0;
		} else {
			if (dir > 0) {
				if (x <= *x0)
					return 0;
			} else {
				if (x >= *x1)
					return 0;
			}
		}
		return 3;
	}
	return 1;
}

/* relative offsets, direction from left to right. */
/* Mode indicates if we are in fill mode (1) or line mode (0) */

static void
draw_line(SLICE *slice, INT32 x0, INT32 y, INT32 x1, int mode)
{
	int	dbl = (slice->adir > 0 && slice->bdir < 0);
	int 	discard, ret;
	INT32	x2 = x0, x3 = x1;

	if (y == 0) {
		if (slice->type != PIE)
			return;
		/* edges on different sides */
		if ((slice->ay <= 0 && slice->by >= 0) ||
		    (slice->ay >= 0 && slice->by <= 0)) {
			if (slice->adir < 0)  {
				if (x1 > 0)
					x1 = 0;
			}
			if (slice->bdir > 0) {
				if (x0 < 0)
					x0 = 0;
			}
		} else {
			if (!dbl) {
				/* FIXME leaving in draws dot in center*/

			        if (slice->psd->crp_FillMode != FILL_SOLID && mode) 
				  ts_drawpoint(slice->_CoreGfxBase, slice->psd, slice->x0, slice->y0);
				else 
				  drawpoint(slice->_CoreGfxBase,slice->psd, slice->x0, slice->y0);
				return;
			}
		}
		if (slice->psd->crp_FillMode != FILL_SOLID && mode)
		  ts_drawrow(slice->_CoreGfxBase, slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0);
		else
		  drawrow(slice->_CoreGfxBase,slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0);
		return;
	}

	/* clip left edge / line */
	ret = clip_line(slice, slice->ax, slice->ay, slice->adir, y, &x0, &x1);

	if (dbl) {
		if (!ret) {
			/* edges separate line to two parts */
		        if (slice->psd->crp_FillMode != FILL_SOLID && mode)
			  ts_drawrow(slice->_CoreGfxBase, slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0 + y);
			else
			  drawrow(slice->_CoreGfxBase,slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0 + y);

			x0 = x2;
			x1 = x3;
		}
	} else {
		if (ret > 1) {
			return;
		}
	}

	discard = ret;
	ret = clip_line(slice, slice->bx, slice->by, slice->bdir, y, &x0, &x1);

	discard += ret;
	if (discard > 2 && !(dbl && ret == 0 && discard == 3)) {
		return;
	}
	if (discard == 2) {
		/* line on other side than slice */
		if (slice->adir < 0 || slice->bdir > 0) {
			return;
		}
	}
	if (slice->psd->crp_FillMode != FILL_SOLID && mode)
	  ts_drawrow(slice->_CoreGfxBase, slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0 + y);
	else
	  drawrow(slice->_CoreGfxBase,slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0 + y);
}

/*
 * draw one line segment or set of points, called from drawarc routine
 *
 * Note that this is called for all rows in one quadrant of the ellipse.
 * It mirrors vertically & horizontally to get the entire ellipse.
 *
 * It passes on co-ordinates for the *entire* ellipse - for pie and
 * arc, clipping is done later to ensure that only the requested angle
 * gets drawn.
 */
static void
drawarcsegment(SLICE *slice, INT32 xp, INT32 yp, int drawon)
{
	UINT32 dm = 0;
	int dc = 0;

	switch (slice->type) {
	case ELLIPSEFILL:
		/* draw ellipse fill segment*/
                /* First, save the dash settings, because we don't want to use them here */

	  if (slice->psd->crp_FillMode != FILL_SOLID) {
	    ts_drawrow(slice->_CoreGfxBase, slice->psd, slice->x0-xp, slice->x0+xp, slice->y0-yp);
	    ts_drawrow(slice->_CoreGfxBase, slice->psd, slice->x0-xp, slice->x0+xp, slice->y0+yp);
	  }
	  else {
	    SetDash(slice->psd, &dm, &dc); /* Must turn off the dash settings because of drawrow() */
	    drawrow(slice->_CoreGfxBase,slice->psd, slice->x0-xp, slice->x0+xp, slice->y0-yp);
	    drawrow(slice->_CoreGfxBase,slice->psd, slice->x0-xp, slice->x0+xp, slice->y0+yp);
	    SetDash(slice->psd, &dm, &dc);
	  }

	  return;

	case ELLIPSE:
	  if (!drawon) return;
		/* set four points symmetrically situated around a point*/
		drawpoint(slice->_CoreGfxBase,slice->psd, slice->x0 + xp, slice->y0 + yp);
		drawpoint(slice->_CoreGfxBase,slice->psd, slice->x0 - xp, slice->y0 + yp);
		drawpoint(slice->_CoreGfxBase,slice->psd, slice->x0 + xp, slice->y0 - yp);
		drawpoint(slice->_CoreGfxBase,slice->psd, slice->x0 - xp, slice->y0 - yp);
		return;

	case PIE:
		/* draw top and bottom halfs of pie*/
	    if (slice->psd->crp_FillMode == FILL_SOLID) SetDash(slice->psd, &dm, &dc);
		draw_line(slice, -xp, -yp, +xp, 1);
		draw_line(slice, -xp, +yp, +xp, 1);
		if (slice->psd->crp_FillMode == FILL_SOLID) SetDash(slice->psd, &dm, &dc);
		return;

	default:	/* MWARC, MWARCOUTLINE*/
		/* set four points symmetrically around a point and clip*/

		draw_line(slice, +xp, +yp, +xp, 0);
		draw_line(slice, -xp, +yp, -xp, 0);
		draw_line(slice, +xp, -yp, +xp, 0);
		draw_line(slice, -xp, -yp, -xp, 0);
		return;
	}
}

/* General routine to plot points on an arc.  Used by arc, pie and ellipse*/
static void
drawarc(SLICE *slice)
{
	INT32 xp, yp;		/* current point (based on center) */
	INT32 rx, ry;
	long Asquared;		/* square of x semi axis */
	long TwoAsquared;
	long Bsquared;		/* square of y semi axis */
	long TwoBsquared;
	long d;
	long dx, dy;

	int bit  = 0;
	int drawon = 1;

	rx = slice->rx;
	ry = slice->ry;

	xp = 0;
	yp = ry;
	Asquared = rx * rx;
	TwoAsquared = 2 * Asquared;
	Bsquared = ry * ry;
	TwoBsquared = 2 * Bsquared;
	d = Bsquared - Asquared * ry + (Asquared >> 2);
	dx = 0;
	dy = TwoAsquared * ry;

	if (slice->psd->crp_FillMode != FILL_SOLID)
	  set_ts_origin(slice->psd, slice->x0 - rx, slice->y0 - ry);

	while (dx < dy) {

		/*
		 * Only draw if one of the following conditions holds:
		 * - We're drawing an outline - i.e. slice->type is
		 *   not MWPIE or MWELLIPSEFILL
		 * - We're about to move on to the next Y co-ordinate
		 *   (i.e. we're drawing a filled shape and we're at
		 *   the widest point for this Y co-ordinate).
		 *   This is the case if d (the error term) is >0
		 * Otherwise, we draw multiple times, which messes up
		 * with SRC_OVER or XOR modes.
		 */
		if ((d > 0) || ((slice->type != PIE) && (slice->type != ELLIPSEFILL))) {
			if (slice->psd->crp_DashCount) {
				drawon = (slice->psd->crp_DashMask & (1 << bit)) ? 1 : 0;
				bit = (bit + 1) % slice->psd->crp_DashCount;
			} else
				drawon = 1;

			drawarcsegment(slice, xp, yp, drawon);
		}

		if (d > 0) {
			yp--;
			dy -= TwoAsquared;
			d -= dy;
		}
		xp++;
		dx += TwoBsquared;
		d += (Bsquared + dx);
	}

	d += ((3L * (Asquared - Bsquared) / 2L - (dx + dy)) >> 1);

	while (yp >= 0) {
	        if (slice->psd->crp_DashCount) {
	          drawon = (slice->psd->crp_DashMask & (1 << bit)) ? 1 : 0;
		  bit = (bit + 1) % slice->psd->crp_DashCount;
		}
	        else drawon = 1;

		drawarcsegment(slice, xp, yp, drawon);
		if (d < 0) {
			xp++;
			dx += TwoBsquared;
			d += dx;
		}
		yp--;
		dy -= TwoAsquared;
		d += (Asquared - dy);
	}

}
#undef CoreGfxBase
/**
 * Draw an arc or pie using start/end points.
 * Integer only routine.  To specify start/end angles,
 * use GdArcAngle.
 *
 * @param psd Destination surface.
 * @param x0 Center of arc (X co-ordinate).
 * @param y0 Center of arc (Y co-ordinate).
 * @param rx Radius of arc in X direction.
 * @param ry Radius of arc in Y direction.
 * @param ax Start of arc (X co-ordinate).
 * @param ay Start of arc (Y co-ordinate).
 * @param bx End of arc (X co-ordinate).
 * @param by End of arc (Y co-ordinate).
 * @param type Type of arc:
 * MWARC is a curved line.
 * MWARCOUTLINE is a curved line plus straight lines joining the ends
 * to the center of the arc.
 * MWPIE is a filled shape, like a section of a pie chart.
 *
 * FIXME: Buggy w/small angles
 */
void cgfx_Arc(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x0, INT32 y0, INT32 rx, INT32 ry,
	INT32 ax, INT32 ay, INT32 bx, INT32 by, int type)
{
	INT32	adir, bdir;
	SLICE	slice;

	if (rx <= 0 || ry <= 0) return;

	/*
	 * Calculate right/left side clipping, based on quadrant.
	 * dir is positive when right side is filled and negative when
	 * left side is to be filled.
	 *
	 * >= 0 is bottom half
	 */
	adir = (ay >= 0)?  1: -1;
	bdir = (by >= 0)? -1:  1;
#if 1
	/*
	 * The clip_line routine has problems around the 0 and
	 * 180 degree axes.
	 * This <fix> is required to make the clip_line algorithm
	 * work.  Getting these routines to work for all angles is
	 * a bitch.  And they're still buggy.  Doing this causes
	 * half circles to be outlined with a slightly bent line
	 * on the x axis. FIXME
	 */
	if (ay == 0) ++ay;
	if (by == 0) ++by;
#endif
	/* swap rightmost edge first */
	if (bx > ax) {
		INT32 swap;

		swap = ax;
		ax = bx;
		bx = swap;

		swap = ay;
		ay = by;
		by = swap;

		swap = adir;
		adir = bdir;
		bdir = swap;
	}

	/* check for entire area clipped, draw with per-point clipping*/
	if (ClipArea(psd, x0-rx, y0-ry, x0+rx, y0+ry) == CLIP_INVISIBLE) return;

	slice.psd = psd;
	slice.x0 = x0;
	slice.y0 = y0;
	slice.rx = rx;
	slice.ry = ry;
	slice.ax = ax;
	slice.ay = ay;
	slice.bx = bx;
	slice.by = by;
	slice.adir = adir;
	slice.bdir = bdir;
	slice.type = type;

	drawarc(&slice);

	if (type & OUTLINE) {
		/* draw two lines from rx,ry to arc endpoints*/
		Line(psd, x0, y0, x0+ax, y0+ay, TRUE);
		Line(psd, x0, y0, x0+bx, y0+by, TRUE);
	}

	FixCursor(psd);
}

/**
 * Draw an ellipse using the current clipping region and foreground color.
 * This draws in the outline of the ellipse, or fills it.
 * Integer only routine.
 *
 * @param psd Destination surface.
 * @param x Center of ellipse (X co-ordinate).
 * @param y Center of ellipse (Y co-ordinate).
 * @param rx Radius of ellipse in X direction.
 * @param ry Radius of ellipse in Y direction.
 * @param fill Nonzero for a filled ellipse, zero for an outline.
 */
void cgfx_Ellipse(CoreGfxBase *CoreGfxBase, CRastPort *psd, INT32 x, INT32 y, INT32 rx, INT32 ry, BOOL fill)
{
	SLICE	slice;

	if (rx < 0 || ry < 0) return;

	/* Check if the ellipse bounding box is either totally visible
	 * or totally invisible.  Draw with per-point clipping.
	 */
	switch (ClipArea(psd, x - rx, y - ry, x + rx, y + ry)) {
	case CLIP_VISIBLE:
		/*
		 * For size considerations, there's no low-level ellipse
		 * draw, so we've got to draw all ellipses
		 * with per-point clipping for the time being
		psd->DrawEllipse(psd, x, y, rx, ry, fill, gr_foreground);
		GdFixCursor(psd);
		return;
		 */
		break;

	case CLIP_INVISIBLE:
		return;
  	}

	slice.psd = psd;
	slice.x0 = x;
	slice.y0 = y;
	slice.rx = rx;
	slice.ry = ry;
	slice.type = fill? ELLIPSEFILL: ELLIPSE;
	/* other elements unused*/

	drawarc(&slice);
	FixCursor(psd);
}

