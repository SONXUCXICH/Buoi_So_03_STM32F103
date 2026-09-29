ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    /* Vector table */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Code */
    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text.*)

        *(.rodata)
        *(.rodata.*)

        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* Initialized data */
    .data : AT(_etext)
    {
        . = ALIGN(4);

        _sdata = .;

        *(.data)
        *(.data.*)

        . = ALIGN(4);

        _edata = .;
    } > RAM

    _sidata = LOADADDR(.data);

    /* Zero initialized data */
    .bss :
    {
        . = ALIGN(4);

        _sbss = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);

        _ebss = .;
    } > RAM

    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
