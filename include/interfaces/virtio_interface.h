#ifndef virtio_interface_h
#define virtio_interface_h

typedef struct VIOBase *pVIOBase;

#define VirtioWrite8(a,b,c)			_LIBCALL(VIOBase,  5, void , (pVIOBase, uint16_t, uint16_t, uint8_t), (VIOBase, a, b, c))
#define VirtioWrite16(a,b,c)		_LIBCALL(VIOBase,  6, void , (pVIOBase, uint16_t, uint16_t, uint16_t), (VIOBase, a, b, c))
#define VirtioWrite32(a,b,c)		_LIBCALL(VIOBase,  7, void , (pVIOBase, uint16_t, uint16_t, uint32_t), (VIOBase, a, b, c))
#define VirtioRead8(a,b)			_LIBCALL(VIOBase,  8, uint8_t , (pVIOBase, uint16_t, uint16_t), (VIOBase, a, b))
#define VirtioRead16(a,b)			_LIBCALL(VIOBase,  9, uint16_t, (pVIOBase, uint16_t, uint16_t), (VIOBase, a, b))
#define VirtioRead32(a,b)			_LIBCALL(VIOBase, 10, uint32_t, (pVIOBase, uint16_t, uint16_t), (VIOBase, a, b))
#define VirtioExchangeFeatures(a)	_LIBCALL(VIOBase, 11, void, (pVIOBase, pVirtIODevice), (VIOBase, a))
#define VirtioAllocateQueues(a,b)	_LIBCALL(VIOBase, 12, int32_t, (pVIOBase, pVirtIODevice, int32_t), (VIOBase, a,b))
#define VirtioInitQueues(a)			_LIBCALL(VIOBase, 13, int32_t, (pVIOBase, pVirtIODevice), (VIOBase, a))
#define VirtioFreeQueues(a)			_LIBCALL(VIOBase, 14, void, (pVIOBase, pVirtIODevice), (VIOBase, a))
#define VirtioHostSupports(a,b)		_LIBCALL(VIOBase, 15, int32_t, (pVIOBase, pVirtIODevice, int32_t), (VIOBase, a,b))
#define VirtioGuestSupports(a,b)	_LIBCALL(VIOBase, 16, int32_t, (pVIOBase, pVirtIODevice, int32_t), (VIOBase, a,b))

#endif
