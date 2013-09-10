#ifndef expansion_funcs_h
#define expansion_funcs_h

#include "expansion.h"
#include "expansionbase.h"

BOOL AddDosNode(INT32 bootPri, UINT32 flags, struct DeviceNode *deviceNode);
struct DeviceNode *MakeDosNode(struct ExpDosNode *parameter);

UINT32 PCIConfigRead32(const PCIAddress *addr, UINT16 offset);
UINT16 PCIConfigRead16(const PCIAddress *addr, UINT16 offset);
UINT8 PCIConfigRead8(const PCIAddress *addr, UINT16 offset);
void PCIConfigWrite32(const PCIAddress *addr, UINT16 offset, UINT32 data);
void PCIConfigWrite16(const PCIAddress *addr, UINT16 offset, UINT16 data);
void PCIConfigWrite8(const PCIAddress *addr, UINT16 offset, UINT8 data);
BOOL PCIScanBus(PCIScanState *state);
BOOL PCIFindDevice(UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut);
void PCISetBAR(const PCIAddress *addr, INT32 index, UINT32 value);
UINT32 PCIGetBARAddr(const PCIAddress *addr, INT32 index);
void PCISetMemEnable(const PCIAddress *addr, BOOL enable);

#define AddDosNode(a,b,c)	(((BOOL(*)(APTR, INT32, UINT32, struct DeviceNode *)) 	_GETVECADDR(ExpansionBase,5)) (ExpansionBase, a,b,c))
#define MakeDosNode(a)		(((struct DeviceNode *(*)(APTR, struct ExpDosNode *))	_GETVECADDR(ExpansionBase,6)) (ExpansionBase, a))

#define PCIConfigRead8(a,b)		(((UINT8(*)(APTR, const PCIAddress *addr, UINT16))						_GETVECADDR(ExpansionBase,7))(ExpansionBase,a,b))
#define PCIConfigRead16(a,b)	(((UINT16(*)(APTR, const PCIAddress *addr, UINT16))						_GETVECADDR(ExpansionBase,8))(ExpansionBase,a,b))
#define PCIConfigRead32(a,b)	(((UINT32(*)(APTR, const PCIAddress *addr, UINT16))						_GETVECADDR(ExpansionBase,9))(ExpansionBase,a,b))
#define PCIConfigWrite8(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))	_GETVECADDR(ExpansionBase,10))(ExpansionBase,a,b,c))
#define PCIConfigWrite16(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))	_GETVECADDR(ExpansionBase,11))(ExpansionBase,a,b,c))
#define PCIConfigWrite32(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))	_GETVECADDR(ExpansionBase,12))(ExpansionBase,a,b,c))

#define PCIScanBus(a)			(((BOOL(*)(APTR, PCIScanState *state))										_GETVECADDR(ExpansionBase,13))(ExpansionBase,a))
#define PCIFindDevice(a,b,c)	(((BOOL(*)(APTR, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut))	_GETVECADDR(ExpansionBase,14))(ExpansionBase,a,b,c))
#define PCISetBAR(a,b,c)		(((void(*)(APTR, const PCIAddress *addr, INT32 index, UINT32 value))		_GETVECADDR(ExpansionBase,15))(ExpansionBase,a,b,c))
#define PCIGetBARAddr(a,b)		(((UINT32(*)(APTR, const PCIAddress *addr, INT32 index))					_GETVECADDR(ExpansionBase,16))(ExpansionBase,a,b))
#define PCISetMemEnable(a,b)	(((void(*)(APTR, const PCIAddress *addr, BOOL enable))						_GETVECADDR(ExpansionBase,17))(ExpansionBase,a,b))

#endif
