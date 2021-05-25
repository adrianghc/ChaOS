ENTRY(_start)
SECTIONS
{
. = 0x20000000;
.init : { *(.init) }
.text : { *(.text) }

. = 0x20100000;
.lib  : { *(.lib) }
.data : { *(.data) }
.bss  : { *(.bss) }
}
