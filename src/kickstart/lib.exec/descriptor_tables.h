#include "types.h"

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
struct gdt_entry_struct
{
    UINT16 limit_low;           // The lower 16 bits of the limit.
    UINT16 base_low;            // The lower 16 bits of the base.
    UINT8  base_middle;         // The next 8 bits of the base.
    UINT8  access;              // Access flags, determine what ring this segment can be used in.
    UINT8  granularity;
    UINT8  base_high;           // The last 8 bits of the base.
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

// This struct describes a GDT pointer. It points to the start of
// our array of GDT entries, and is in the format required by the
// lgdt instruction.
struct gdt_ptr_struct
{
    UINT16 limit;               // The upper 16 bits of all selector limits.
    UINT32 base;                // The address of the first gdt_entry_t struct.
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// A struct describing an interrupt gate.
struct idt_entry_struct
{
    UINT16 base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
    UINT16 sel;                 // Kernel segment selector.
    UINT8  always0;             // This must always be zero.
    UINT8  flags;               // More flags. See documentation.
    UINT16 base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
    UINT16 limit;
    UINT32 base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;


