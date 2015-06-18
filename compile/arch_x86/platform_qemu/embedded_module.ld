
SECTIONS
{
    .text :
    {
        *(.text)
    }

    .resident :
    {
        *(.resident)
    }

    .data :
    {
        *(.data)
        *(.rodata)
    }

    .bss :
    {
        *(.bss)
    }
}

