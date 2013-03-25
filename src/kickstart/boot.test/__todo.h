static inline void
memcpy(void *dest, const void *src, uint32 size)
{
   asm volatile ("cld; rep movsb" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void
memset(void *dest, uint8 value, uint32 size)
{
   asm volatile ("cld; rep stosb" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static inline void
memcpy16(void *dest, const void *src, uint32 size)
{
   asm volatile ("cld; rep movsw" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void
memset16(void *dest, uint16 value, uint32 size)
{
   asm volatile ("cld; rep stosw" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static inline void
memcpy32(void *dest, const void *src, uint32 size)
{
   asm volatile ("cld; rep movsl" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void
memset32(void *dest, uint32 value, uint32 size)
{
   asm volatile ("cld; rep stosl" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

#define Atomic_Exchange(mem, reg) \
   asm volatile ("xchgl %0, %1" : "+r" (reg), "+m" (mem) :)

#define Atomic_Or(mem, reg) \
   asm volatile ("lock orl %1, %0" :"+m" (mem) :"r" (reg))
