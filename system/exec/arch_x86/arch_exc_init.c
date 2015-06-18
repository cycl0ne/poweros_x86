#include "execbase_private.h"
#include "exec_interface.h"
#include "descriptor_tables.h"
#include "arch_config.h"

typedef unsigned int   u32int;
typedef          int   s32int;
typedef unsigned short u16int;
typedef          short s16int;
typedef unsigned char  u8int;
typedef          char  s8int;

idt_entry_t idt_entries[256];
idt_ptr_t   idt_ptr;

extern void idt_flush(UINT32);

static void idt_set_gate(u8int num, u32int base, u16int sel, u8int flags)
{
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    // We must uncomment the OR below when we get to using user-mode.
    // It sets the interrupt gate's privilege level to 3.
    idt_entries[num].flags   = flags /* | 0x60 */;
}

void arch_exc_init()
{
	idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
	idt_ptr.base  = (u32int)&idt_entries;

	arch_memset((u8int *)&idt_entries, 0, sizeof(idt_entry_t)*256);

	idt_set_gate( 0, (u32int)int_0, 0x08, 0x8E);
	idt_set_gate( 1, (u32int)int_1, 0x08, 0x8E);
	idt_set_gate( 2, (u32int)int_2, 0x08, 0x8E);
	idt_set_gate( 3, (u32int)int_3, 0x08, 0x8E);
	idt_set_gate( 4, (u32int)int_4, 0x08, 0x8E);
	idt_set_gate( 5, (u32int)int_5, 0x08, 0x8E);
	idt_set_gate( 6, (u32int)int_6, 0x08, 0x8E);
	idt_set_gate( 7, (u32int)int_7, 0x08, 0x8E);
	idt_set_gate( 8, (u32int)int_8, 0x08, 0x8E);
	idt_set_gate( 9, (u32int)int_9, 0x08, 0x8E);
	idt_set_gate(10, (u32int)int_10, 0x08, 0x8E);
	idt_set_gate(11, (UINT32)int_11, 0x08, 0x8E);
	idt_set_gate(12, (UINT32)int_12, 0x08, 0x8E);
	idt_set_gate(13, (UINT32)int_13, 0x08, 0x8E);
	idt_set_gate(14, (UINT32)int_14, 0x08, 0x8E);
	idt_set_gate(15, (UINT32)int_15, 0x08, 0x8E);
	idt_set_gate(16, (UINT32)int_16, 0x08, 0x8E);
	idt_set_gate(17, (UINT32)int_17, 0x08, 0x8E);
	idt_set_gate(18, (UINT32)int_18, 0x08, 0x8E);
	idt_set_gate(19, (UINT32)int_19, 0x08, 0x8E);
	idt_set_gate(20, (UINT32)int_20, 0x08, 0x8E);
	idt_set_gate(21, (UINT32)int_21, 0x08, 0x8E);
	idt_set_gate(22, (UINT32)int_22, 0x08, 0x8E);
	idt_set_gate(23, (UINT32)int_23, 0x08, 0x8E);
	idt_set_gate(24, (UINT32)int_24, 0x08, 0x8E);
	idt_set_gate(25, (UINT32)int_25, 0x08, 0x8E);
	idt_set_gate(26, (UINT32)int_26, 0x08, 0x8E);
	idt_set_gate(27, (UINT32)int_27, 0x08, 0x8E);
	idt_set_gate(28, (UINT32)int_28, 0x08, 0x8E);
	idt_set_gate(29, (UINT32)int_29, 0x08, 0x8E);
	idt_set_gate(30, (UINT32)int_30, 0x08, 0x8E);
	idt_set_gate(31, (UINT32)int_31, 0x08, 0x8E);
	idt_set_gate(32, (UINT32)int_32, 0x08, 0x8E);
	idt_set_gate(33, (UINT32)int_33, 0x08, 0x8E);
	idt_set_gate(34, (UINT32)int_34, 0x08, 0x8E);
	idt_set_gate(35, (UINT32)int_35, 0x08, 0x8E);
	idt_set_gate(36, (UINT32)int_36, 0x08, 0x8E);
	idt_set_gate(37, (UINT32)int_37, 0x08, 0x8E);
	idt_set_gate(38, (UINT32)int_38, 0x08, 0x8E);
	idt_set_gate(39, (UINT32)int_39, 0x08, 0x8E);
	idt_set_gate(40, (UINT32)int_40, 0x08, 0x8E);
	idt_set_gate(41, (UINT32)int_41, 0x08, 0x8E);
	idt_set_gate(42, (UINT32)int_42, 0x08, 0x8E);
	idt_set_gate(43, (UINT32)int_43, 0x08, 0x8E);
	idt_set_gate(44, (UINT32)int_44, 0x08, 0x8E);
	idt_set_gate(45, (UINT32)int_45, 0x08, 0x8E);
	idt_set_gate(46, (UINT32)int_46, 0x08, 0x8E);
	idt_set_gate(47, (UINT32)int_47, 0x08, 0x8E);
	idt_set_gate(48, (UINT32)int_48, 0x08, 0x8E);
	idt_set_gate(49, (UINT32)int_49, 0x08, 0x8E);
	idt_set_gate(50, (UINT32)int_50, 0x08, 0x8E);
	idt_set_gate(51, (UINT32)int_51, 0x08, 0x8E);
	idt_set_gate(52, (UINT32)int_52, 0x08, 0x8E);
	idt_set_gate(53, (UINT32)int_53, 0x08, 0x8E);
	idt_set_gate(54, (UINT32)int_54, 0x08, 0x8E);
	idt_set_gate(55, (UINT32)int_55, 0x08, 0x8E);
	idt_set_gate(56, (UINT32)int_56, 0x08, 0x8E);
	idt_set_gate(57, (UINT32)int_57, 0x08, 0x8E);
	idt_set_gate(58, (UINT32)int_58, 0x08, 0x8E);
	idt_set_gate(59, (UINT32)int_59, 0x08, 0x8E);
	idt_set_gate(60, (UINT32)int_60, 0x08, 0x8E);
	idt_set_gate(61, (UINT32)int_61, 0x08, 0x8E);
	idt_set_gate(62, (UINT32)int_62, 0x08, 0x8E);
	idt_set_gate(63, (UINT32)int_63, 0x08, 0x8E);
	
	idt_flush((u32int)&idt_ptr);
}

