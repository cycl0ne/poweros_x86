static __inline__ void WRITE_PORT_ULONG(UINT16 port, UINT32 value)
{
   __asm__ __volatile__ ("outl %0, %1" : :"a" (value), "d" (port));
}

static __inline__  UINT32 READ_PORT_ULONG(UINT16 port)
{
   UINT32 value;
   __asm__ __volatile__ ("inl %1, %0" :"=a" (value) :"d" (port));
   return value;
}

#define offsetof(type, member)  ((UINT32)(&((type*)NULL)->member))

