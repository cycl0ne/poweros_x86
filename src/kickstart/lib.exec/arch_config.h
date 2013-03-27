#ifndef arch_config_h
#define arch_config_h

#include "types.h" 

static inline void arch_Intr_Enable(void) {
   asm volatile ("sti");
}

static inline void arch_Intr_Disable(void) {
   asm volatile ("cli");
}

static inline void arch_Halt(void) {
   asm volatile ("hlt");
}

static inline void memcpy(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsb" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void memset(void *dest, UINT8 value, UINT32 size)
{
   asm volatile ("cld; rep stosb" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static inline void memcpy16(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsw" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void memset16(void *dest, UINT16 value, UINT32 size)
{
   asm volatile ("cld; rep stosw" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

static inline void memcpy32(void *dest, const void *src, UINT32 size)
{
   asm volatile ("cld; rep movsl" : "+c" (size), "+S" (src), "+D" (dest) :: "memory");
}

static inline void memset32(void *dest, UINT32 value, UINT32 size)
{
   asm volatile ("cld; rep stosl" : "+c" (size), "+D" (dest) : "a" (value) : "memory");
}

#define Atomic_Exchange(mem, reg) \
   asm volatile ("xchgl %0, %1" : "+r" (reg), "+m" (mem) :)

#define Atomic_Or(mem, reg) \
   asm volatile ("lock orl %1, %0" :"+m" (mem) :"r" (reg))

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


//Configuration Struct.

typedef struct {
	UINT32	cpu_count;      /**< Number of processors detected. */
	volatile INT32 cpu_active;  /**< Number of processors that are up and running. */
	
	INT32	*base;
	INT32	kernel_size;          /**< Size of memory in bytes taken by kernel and stack */
	
	INT32	*stack_base;        /**< Base adddress of initial stack */
	INT32	stack_size;           /**< Size of initial stack */
	UINT32	*memory_base;
	UINT32	memory_size;
	UINT8	memory_prio;
	UINT32	memory_attribute;
	STRPTR	memory_name;
	STRPTR	arch_name;
} arch_config;

typedef struct registers
{
    UINT32 ds;                  // Data segment selector
    UINT32 edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    UINT32 int_no, err_code;    // Interrupt number and error code (if applicable)
    UINT32 eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

extern void int_0();
extern void int_1();
extern void int_2();
extern void int_3();
extern void int_4();
extern void int_5();
extern void int_6();
extern void int_7();
extern void int_8();
extern void int_9();
extern void int_10();
extern void int_11();
extern void int_12();
extern void int_13();
extern void int_14();
extern void int_15();
extern void int_16();
extern void int_17();
extern void int_18();
extern void int_19();
extern void int_20();
extern void int_21();
extern void int_22();
extern void int_23();
extern void int_24();
extern void int_25();
extern void int_26();
extern void int_27();
extern void int_28();
extern void int_29();
extern void int_30();
extern void int_31();
extern void int_32();
extern void int_33();
extern void int_34();
extern void int_35();
extern void int_36();
extern void int_37();
extern void int_38();
extern void int_39();
extern void int_40();
extern void int_41();
extern void int_42();
extern void int_43();
extern void int_44();
extern void int_45();
extern void int_46();
extern void int_47();
extern void int_48();
extern void int_49();
extern void int_50();
extern void int_51();
extern void int_52();
extern void int_53();
extern void int_54();
extern void int_55();
extern void int_56();
extern void int_57();
extern void int_58();
extern void int_59();
extern void int_60();
extern void int_61();
extern void int_62();
extern void int_63();


#endif
