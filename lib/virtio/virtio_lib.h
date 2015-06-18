/**
* File: /virtio．h
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef virtio_lib_h
#define virtio_lib_h

#include "types.h"
#include "libraries.h"
#include "exec_interface.h"
#include "pci.h"
#include "virtio.h"
#include "virtio_interface.h"

typedef struct VIOBase
{
	Library_t	Library;
	pSysBase	SysBase;

} VIOBase_t, *pVIOBase;


static __inline__ void
IO_Out8(UINT16 port, UINT8 value)
{
   __asm__ __volatile__ ("outb %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out16(UINT16 port, UINT16 value)
{
   __asm__ __volatile__ ("outw %0, %1" : :"a" (value), "d" (port));
}

static __inline__ void
IO_Out32(UINT16 port, UINT32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static __inline__ UINT8
IO_In8(UINT16 port)
{
   UINT8 value;
   __asm__ __volatile__ ("inb %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT16
IO_In16(UINT16 port)
{
   UINT16 value;
   __asm__ __volatile__ ("inw %1, %0" :"=a" (value) :"d" (port));
   return value;
}

static __inline__ UINT32
IO_In32(UINT16 port)
{
   UINT32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

#endif
