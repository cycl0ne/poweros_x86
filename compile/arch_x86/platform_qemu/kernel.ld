
ENTRY(start)
SECTIONS
{

	.multiboot 0x108000 :
	{
        *(.multiboot)
        . = ALIGN(4096);
	}

    .text :
    {
        __code = .;
        *(.text)
        . = ALIGN(4096);
    }

    .resident :
    {
		. = ALIGN(4);
        __resident_tags_start = .;
        *(.resident)
        . = ALIGN(4);
        __resident_tags_end = .;
        . = ALIGN(4096);
    }

    .data :
    {
        __data = .;
        *(.data)
        *(.rodata)
        . = ALIGN(4096);
    }

    .bss :
    {
        __bss = .;
        *(.bss)
        . = ALIGN(4096);
    }


    __end = .;
}


