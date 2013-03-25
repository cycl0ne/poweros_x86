/*
** x86 Linker Script
**
*/


ENTRY(start)
SECTIONS
{
    .text 0x108000 :
    {
        code = .; _code = .; __code = .;
        *(.text)
        . = ALIGN(4096);
    }

    .data :
    {
        data = .; _data = .; __data = .;
        *(.data)
        *(.rodata)
	   *(COMMON);	   
        . = ALIGN(4096);
    }

    .bss :
    {
        bss = .; _bss = .; __bss = .;
        *(.bss)
        . = ALIGN(4096);
    }
	/DISCARD/ : {
		*(*);
	}
    end = .; _end = .; __end = .;
}


/*
SECTIONS
{
	rom_start = .;
	.text 0x108000 : 
	{
		code = .; _code = .; __code = .;
		*(.multiboot_header)
		*(.text)
		. = ALIGN(4096)
	}
	
	.data : 
	{ 
        data = .; _data = .; __data = .;
        *(.data)
        *(.rodata)
	   hardcoded_load_address = .;
	   LONG(0x00108000);
	   hardcoded_rom_size = .;
	   LONG(rom_end - rom_start - 0x00108000);
        . = ALIGN(4096);
	}

	.bss :
	{
	   bss = .; _bss = .; __bss = .;
	   *(.bss)
	   . = ALIGN(4096);
	}

	end = .; _end = .; __end = .;
	rom_end = .;
}
*/
/*
SECTIONS
{
	.unmapped 0x00108000: AT (0) {
	unmapped_ktext_start = .;
	*(K_TEXT_START);
	unmapped_ktext_end = .;

	unmapped_kdata_start = .;
	*(K_DATA_START);
	unmapped_kdata_end = .;
		
	ktext_start = .;
	*(.text);
	ktext_end = .;
	
	kdata_start = .;
	*(.data);               
	*(.rodata*);            
	*(COMMON);              
	hardcoded_load_address = .;
	LONG(0x00108000);	
	hardcoded_ktext_size = .;
	LONG(ABSOLUTE(ktext_end - ktext_start));
	hardcoded_kdata_size = .;
	LONG(kdata_end - kdata_start);
	. = ALIGN(8);
	*(.bss)
	kdata_end = .;
	
	} > ram

	/DISCARD/ : {
		*(*);
	}
}

*/

