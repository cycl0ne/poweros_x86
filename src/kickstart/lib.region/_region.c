#include "regionbase.h"
#include "_region_int.h"
#include "exec_funcs.h"

#define SysBase RegionBase->SysBase

#define SHORTMIN(a,b)	(INT16)(MIN((INT16)(a),(INT16)(b)))
#define SHORTMAX(a,b)	(INT16)(MAX((INT16)(a),(INT16)(b)))

void gfx_int_FreeRR(RegionBase *RegionBase, struct RegionRectangle *rr)
{
	FreeVec(rr);
}

void gfx_int_RectSplit(RegionBase *RegionBase, struct Region *rgn, struct Rectangle r1, struct Rectangle *r2, UINT32 op)
{
	struct RegionRectangle *nr = NULL; 
	BOOL xrect = gfx_int_RectXRect( &r1, r2 );

	switch( op )
	{
		case( OPERATION_OR ):
		{
			if( xrect )
			{
				if( r2->MaxX < r1.MaxX ) 
				{
					if(nr = gfx_int_NewRect(RegionBase))
					{
						nr->bounds = r1;
						nr->bounds.MinX = r2->MaxX+1;
						gfx_int_AppendRR(rgn,nr);
						r1.MaxX = r2->MaxX;
					}
				}
				if( r2->MinX > r1.MinX )
				{
					if(nr = gfx_int_NewRect(RegionBase))
					{
						nr->bounds = r1;
						nr->bounds.MaxX = r2->MinX-1;
						gfx_int_AppendRR(rgn,nr);
						r1.MinX = r2->MinX;
					}
				}
				if( r2->MaxY < r1.MaxY ) 
				{
					if(nr = gfx_int_NewRect(RegionBase))
					{
						nr->bounds = r1;
						nr->bounds.MinY = r2->MaxY+1;
						gfx_int_AppendRR(rgn,nr);
						r1.MaxY = r2->MaxY;
					}
				}
				if( r2->MinY > r1.MinY )
				{
					if(nr = gfx_int_NewRect(RegionBase))
					{
						nr->bounds = r1;
						nr->bounds.MaxY = r2->MinY-1;
						gfx_int_AppendRR(rgn,nr);
						r1.MinY = r2->MinY;
					}
				}
			} else
			{
				if(nr = gfx_int_NewRect(RegionBase))
				{
					nr->bounds = r1;
					gfx_int_AppendRR(rgn,nr);
				}
			}
		}   break;
		case( OPERATION_AND ):
		{
			if( xrect )
			{
				if(nr = gfx_int_NewRect(RegionBase))
				{
					if( r2->MaxX < r1.MaxX ) r1.MaxX = r2->MaxX;
					if( r2->MinX > r1.MinX ) r1.MinX = r2->MinX;
					if( r2->MaxY < r1.MaxY ) r1.MaxY = r2->MaxY;
					if( r2->MinY > r1.MinY ) r1.MinY = r2->MinY;
					nr->bounds = r1;
					gfx_int_AppendRR(rgn,nr);
				}
			}
		}   break;
	}
}

void gfx_int_ReplaceRegion(RegionBase *RegionBase, struct Region *from, struct Region *to)
{
	short x,y;
    struct RegionRectangle *r;

	ClearRegion(to);
	*to = *from;
	if (to->RegionRectangle) to->RegionRectangle->Prev = to->RegionRectangle; // &to->
	from->RegionRectangle = 0;
	
    y = x = 15000;
    for (r = to->RegionRectangle; r ; r = r->Next)
    {
        x = SHORTMIN(x,r->bounds.MinX);
        y = SHORTMIN(y,r->bounds.MinY);
    }
    gfx_int_AdjustRegionRectangles(to,x,y);
    to->bounds.MinX += x;
    to->bounds.MaxX += x;
    to->bounds.MinY += y;
    to->bounds.MaxY += y;
}

struct RegionRectangle *gfx_int_NewRect(RegionBase *RegionBase)
{
	return (struct RegionRectangle *) AllocVec(sizeof(struct RegionRectangle),MEMF_FAST|MEMF_CLEAR);
}

void gfx_int_Intersect(struct Rectangle *a, struct Rectangle *b, struct Rectangle *c)
{
	c->MinX = MAX(a->MinX,b->MinX); c->MinY = MAX(a->MinY,b->MinY);
	c->MaxX = MIN(a->MaxX,b->MaxX); c->MaxY = MIN(a->MaxY,b->MaxY);
}

BOOL gfx_int_Obscured(struct Rectangle *r1, struct Rectangle *r2)
{
	if ((r1->MinX <= r2->MinX) &&
		(r1->MaxX >= r2->MaxX) &&
		(r1->MinY <= r2->MinY) &&
		(r1->MaxY >= r2->MaxY) )    return(TRUE);
	else                        	return(FALSE);
}

void gfx_int_OffsetRegionRectangle(struct RegionRectangle *r, UINT16 x,UINT16 y)
{
    r->bounds.MinX -= x;
    r->bounds.MaxX -= x;
    r->bounds.MinY -= y;
    r->bounds.MaxY -= y;
}

void gfx_int_AdjustRegionRectangles(struct Region *rgn, UINT16 x, UINT16 y)
{
    struct RegionRectangle *r;
    for (r = rgn->RegionRectangle; r ; r = r->Next) gfx_int_OffsetRegionRectangle(r,x,y);
}

BOOL gfx_int_RectXRect(struct Rectangle *r1, struct Rectangle *r2)
{
	if ((r2->MaxX < r1->MinX) ||
		(r2->MinX > r1->MaxX) ||
		(r2->MaxY < r1->MinY) ||
		(r2->MinY > r1->MaxY) ) return(FALSE);
	else return(TRUE);
}

void gfx_int_AdjustRegion(struct Region *newrgn, struct Region *rgn ,struct Rectangle *rect)
{
    if (rgn->RegionRectangle == 0)   newrgn->bounds = *rect;
    else
    {
		newrgn->bounds.MinX = SHORTMIN(rgn->bounds.MinX,rect->MinX);
		newrgn->bounds.MinY = SHORTMIN(rgn->bounds.MinY,rect->MinY);
		newrgn->bounds.MaxX = SHORTMAX(rgn->bounds.MaxX,rect->MaxX);
		newrgn->bounds.MaxY = SHORTMAX(rgn->bounds.MaxY,rect->MaxY);
		    
		if ( (newrgn->bounds.MinX != rgn->bounds.MinX) ||
			 (newrgn->bounds.MinY != rgn->bounds.MinY) )
		{
			gfx_int_AdjustRegionRectangles(rgn,newrgn->bounds.MinX-rgn->bounds.MinX, newrgn->bounds.MinY-rgn->bounds.MinY);
		}
    }
    newrgn->RegionRectangle = 0;
    rgn->bounds = newrgn->bounds;
}

void gfx_int_AppendRR(struct Region *rgn, struct RegionRectangle *r)
{
	struct RegionRectangle *p,*q;
	q = rgn->RegionRectangle; //q = &rgn->RegionRectangle;
	while (p = q->Next) q = p;
	q->Next = r;
	r->Next = 0;
	r->Prev = q;
}

void gfx_int_PrependRR(struct Region *rgn, struct RegionRectangle *r)
{
    struct RegionRectangle *p,*q;
    q = r->Prev = rgn->RegionRectangle; //&rgn->RegionRectangle;
    if(p = r->Next = q->Next) p->Prev = r;
    q->Next = r;
}

void gfx_int_RemoveRR(struct Region *rgn, struct RegionRectangle *r)
{
    if (r->Next) r->Next->Prev = r->Prev;
    if (r->Prev == rgn->RegionRectangle) rgn->RegionRectangle = r->Next; // &rgn->
    else r->Prev->Next = r->Next;
}

