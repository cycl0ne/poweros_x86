#ifndef REGION_FUNCS_H
#define REGION_FUNCS_H

void AndRectRegion(Region *rgn, Rectangle *rect);
BOOL AndRegionRegion(Region *rgnsrc, Region *rgndst);
BOOL ClearRectRegion(Region *rgn, Rectangle *rect);
void ClearRegion(Region *rgn);
void DisposeRegion(Region *rgn);
Region *NewRegion();
BOOL OrRectRegion(Region *rgn, Rectangle *rect);
BOOL OrRegionRegion(Region *rgnsrc, Region *rgndst);
BOOL XorRectRegion(Region *rgn, Rectangle *rect);
BOOL XorRegionRegion(Region *rgnsrc, Region *rgndst);

//#define OpenLib(x)  	(((APTR(*)(APTR,APTR)) _GETVECADDR(RegionBase,1))(RegionBase,x))
//#define CloseLib(x) 	(((APTR(*)(APTR,APTR)) _GETVECADDR(RegionBase,2))(RegionBase,x))
//#define ExpungeLib(x)	(((APTR(*)(APTR,APTR)) _GETVECADDR(RegionBase,3))(RegionBase,x))
//#define ExtFuncLib(x)	(((APTR(*)(APTR,APTR)) _GETVECADDR(RegionBase,4))(RegionBase,x))

#define AndRectRegion(a,b)		(((void(*)(struct RegionBase *, Region *,Rectangle *))	_GETVECADDR(RegionBase, 5))(RegionBase,a,b))
#define AndRegionRegion(a,b)	(((BOOL(*)(struct RegionBase *, Region *,Region *)) 	_GETVECADDR(RegionBase, 6))(RegionBase,a,b))
#define ClearRectRegion(a,b)	(((BOOL(*)(struct RegionBase *, Region *,Rectangle *))	_GETVECADDR(RegionBase, 7))(RegionBase,a,b))
#define ClearRegion(a)			(((void(*)(struct RegionBase *, Region *)) 				_GETVECADDR(RegionBase, 8))(RegionBase,a))
#define DisposeRegion(a)		(((void(*)(struct RegionBase *, Region *)) 				_GETVECADDR(RegionBase, 9))(RegionBase,a))
#define NewRegion(a)			(((Region*(*)(struct RegionBase *)) 					_GETVECADDR(RegionBase,10))(RegionBase))

#define OrRectRegion(a,b)		(((BOOL(*)(struct RegionBase *, Region *,Rectangle *))	_GETVECADDR(RegionBase,11))(RegionBase,a,b))
#define OrRegionRegion(a,b)		(((BOOL(*)(struct RegionBase *, Region *,Region *)) 	_GETVECADDR(RegionBase,12))(RegionBase,a,b))
#define XorRectRegion(a,b)		(((BOOL(*)(struct RegionBase *, Region *,Rectangle *))	_GETVECADDR(RegionBase,13))(RegionBase,a,b))
#define XorRegionRegion(a,b)	(((BOOL(*)(struct RegionBase *, Region *,Region *)) 	_GETVECADDR(RegionBase,14))(RegionBase,a,b))

#endif
