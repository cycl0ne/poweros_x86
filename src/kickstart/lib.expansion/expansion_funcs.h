#ifndef expansion_funcs_h
#define expansion_funcs_h

#include "expansion.h"

BOOL AddDosNode(INT32 bootPri, UINT32 flags, struct DeviceNode *deviceNode);
struct DeviceNode *MakeDosNode(struct ExpDosNode *parameter);

#define AddDosNode(a,b,c)	(((BOOL(*)(APTR, INT32, UINT32, struct DeviceNode *)) 	_GETVECADDR(ExpansionBase,5)) (ExpansionBase, a,b,c))
#define MakeDosNode(a)		(((struct DeviceNode *(*)(APTR, struct ExpDosNode *))	_GETVECADDR(ExpansionBase,6)) (ExpansionBase, a))

#define ScanPCIAll()	    (((void(*)(APTR)) _GETVECADDR(ExpansionBase,7))(ExpansionBase))
#define ScanPCIBus(a,b,c)   (((void(*)(struct ExpansionBase*, INT32, INT32, INT32)) _GETVECADDR(ExpansionBase,8))(ExpansionBase,a,b,c))

#define FindPCIDevice(a,b)	(((struct PCINode *(*)(struct ExpansionBase*, INT32, INT32)) _GETVECADDR(ExpansionBase,9))(ExpansionBase,a,b))

#define ReadPCIConfig(a,b,c,d,e)    (((UINT32(*)(struct ExpansionBase*, INT32, INT32, INT32, INT32, INT32)) _GETVECADDR(ExpansionBase,10))(ExpansionBase,a,b,c,d,e))
#define WritePCIConfig(a,b,c,d,e,f) (((BOOL (*)(struct ExpansionBase*, INT32, INT32, INT32, INT32, INT32, INT32)) _GETVECADDR(ExpansionBase,11))(ExpansionBase,a,b,c,d,e,f))

#define GetPCIMemorySize(a,b,c,d)    (((UINT32(*)(struct ExpansionBase*, INT32, INT32, INT32, INT32)) _GETVECADDR(ExpansionBase,12))(ExpansionBase,a,b,c,d))
#define SetPCILatency(a,b,c,d)    (((void(*)(struct ExpansionBase*, INT32, INT32, INT32, UINT8)) _GETVECADDR(ExpansionBase,13))(ExpansionBase,a,b,c,d))
#define EnablePCIMaster(a,b,c)   (((void(*)(struct ExpansionBase*, INT32, INT32, INT32)) _GETVECADDR(ExpansionBase,14))(ExpansionBase,a,b,c))

#endif
