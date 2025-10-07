OUTPUT_FORMAT(elf64-x86-64)

ENTRY(x64_main)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    kernel_start = .;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _text_start = .;
    .text : {
        *(.text .text.*)
    } :text
    _text_end = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _rodata_start = .;
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata
    _rodata_end = .;

    .note.gnu.build-id : {
        *(.note.gnu.build-id)
    } :rodata

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    _data_start = .;
    .data : {
        *(.data .data.*)
    } :data
    _data_end = .;

    _bss_start = .;
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data
    _bss_end = .;

    kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}
