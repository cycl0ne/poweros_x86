/**
* File: /expansion_interface．h
* User: cycl0ne
* Date: 2014-11-18
* Time: 09:35 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef expansion_interface_H
#define expansion_interface_H
#include "expansion.h"
#include "pci.h"
#include "dos_io.h"

#if 1
#define AddDosNode(a,b,c)				_LIBCALL(ExpansionBase, 5, BOOL, (APTR, INT32, UINT32, pDosEntry), (ExpansionBase,a,b,c))
#define MakeDosNode(a)					_LIBCALL(ExpansionBase, 6, pDosEntry, (APTR, struct ExpDosNode *), (ExpansionBase,a))
#define PCIConfigRead8(a,b)				_LIBCALL(ExpansionBase, 7, UINT8, (APTR, const PCIAddress *addr, UINT16), (ExpansionBase,a,b))
#define PCIConfigRead16(a,b)			_LIBCALL(ExpansionBase, 8, UINT16, (APTR, const PCIAddress *addr, UINT16), (ExpansionBase,a,b))
#define PCIConfigRead32(a,b)			_LIBCALL(ExpansionBase, 9, UINT32, (APTR, const PCIAddress *addr, UINT16), (ExpansionBase,a,b))
#define PCIConfigWrite8(a,b,c)			_LIBCALL(ExpansionBase, 10, void, (APTR, const PCIAddress *addr, UINT16 offset, UINT32 data), (ExpansionBase,a,b,c))
#define PCIConfigWrite16(a,b,c)			_LIBCALL(ExpansionBase, 11, void, (APTR, const PCIAddress *addr, UINT16 offset, UINT32 data), (ExpansionBase,a,b,c))
#define PCIConfigWrite32(a,b,c)			_LIBCALL(ExpansionBase, 12, void, (APTR, const PCIAddress *addr, UINT16 offset, UINT32 data), (ExpansionBase,a,b,c))
#define PCIScanBus(a)					_LIBCALL(ExpansionBase, 13, BOOL, (APTR, PCIScanState *state), (ExpansionBase,a))
#define PCIFindDevice(a,b,c)			_LIBCALL(ExpansionBase, 14, BOOL, (APTR, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut), (ExpansionBase,a,b,c))
#define PCISetBAR(a,b,c)				_LIBCALL(ExpansionBase, 15, void, (APTR, const PCIAddress *addr, INT32 index, UINT32 value), (ExpansionBase,a,b,c))
#define PCIGetBARAddr(a,b)				_LIBCALL(ExpansionBase, 16, UINT32, (APTR, const PCIAddress *addr, INT32 index), (ExpansionBase,a,b))
#define PCISetMemEnable(a,b)			_LIBCALL(ExpansionBase, 17, void, (APTR, const PCIAddress *addr, BOOL enable), (ExpansionBase,a,b))
#define PCIGetIntrLine(a)				_LIBCALL(ExpansionBase, 18, UINT8, (APTR, const PCIAddress *addr), (ExpansionBase,a))
#define PCIGetIntrPin(a)				_LIBCALL(ExpansionBase, 19, UINT8, (APTR, const PCIAddress *addr), (ExpansionBase,a))
#define PCIFindDeviceByUnit(a,b,c,d)	_LIBCALL(ExpansionBase, 20, BOOL, (APTR, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut, INT32 unit), (ExpansionBase,a,b,c,d))

#else

#define AddDosNode(a,b,c)		(((BOOL(*)(APTR, INT32, UINT32, pDosEntry)) 								_GETVECADDR(ExpansionBase,5)) (ExpansionBase, a,b,c))
#define MakeDosNode(a)			(((pDosEntry(*)(APTR, struct ExpDosNode *))									_GETVECADDR(ExpansionBase,6)) (ExpansionBase, a))

#define PCIConfigRead8(a,b)		(((UINT8(*)(APTR, const PCIAddress *addr, UINT16))							_GETVECADDR(ExpansionBase,7))(ExpansionBase,a,b))
#define PCIConfigRead16(a,b)	(((UINT16(*)(APTR, const PCIAddress *addr, UINT16))							_GETVECADDR(ExpansionBase,8))(ExpansionBase,a,b))
#define PCIConfigRead32(a,b)	(((UINT32(*)(APTR, const PCIAddress *addr, UINT16))							_GETVECADDR(ExpansionBase,9))(ExpansionBase,a,b))
#define PCIConfigWrite8(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))		_GETVECADDR(ExpansionBase,10))(ExpansionBase,a,b,c))
#define PCIConfigWrite16(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))		_GETVECADDR(ExpansionBase,11))(ExpansionBase,a,b,c))
#define PCIConfigWrite32(a,b,c)	(((void(*)(APTR, const PCIAddress *addr, UINT16 offset, UINT32 data))		_GETVECADDR(ExpansionBase,12))(ExpansionBase,a,b,c))

#define PCIScanBus(a)			(((BOOL(*)(APTR, PCIScanState *state))										_GETVECADDR(ExpansionBase,13))(ExpansionBase,a))
#define PCIFindDevice(a,b,c)	(((BOOL(*)(APTR, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut))	_GETVECADDR(ExpansionBase,14))(ExpansionBase,a,b,c))
#define PCISetBAR(a,b,c)		(((void(*)(APTR, const PCIAddress *addr, INT32 index, UINT32 value))		_GETVECADDR(ExpansionBase,15))(ExpansionBase,a,b,c))
#define PCIGetBARAddr(a,b)		(((UINT32(*)(APTR, const PCIAddress *addr, INT32 index))					_GETVECADDR(ExpansionBase,16))(ExpansionBase,a,b))
#define PCISetMemEnable(a,b)	(((void(*)(APTR, const PCIAddress *addr, BOOL enable))						_GETVECADDR(ExpansionBase,17))(ExpansionBase,a,b))

#define PCIGetIntrLine(a)		(((UINT8(*)(APTR, const PCIAddress *addr))									_GETVECADDR(ExpansionBase,18))(ExpansionBase,a))
#define PCIGetIntrPin(a)		(((UINT8(*)(APTR, const PCIAddress *addr))									_GETVECADDR(ExpansionBase,19))(ExpansionBase,a))
#define PCIFindDeviceByUnit(a,b,c,d)	(((BOOL(*)(APTR, UINT16 vendorId, UINT16 deviceId, PCIAddress *addrOut, INT32 unit))	_GETVECADDR(ExpansionBase,20))(ExpansionBase,a,b,c,d))

#endif

#endif
