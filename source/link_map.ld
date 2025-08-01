ENTRY(eventHandlerShim)
GROUP(libgcc.a libc.a libm.a)

/* Everything is loaded from a single segment, which is both writable and
   executable, so the linker will complain:
   device_build/pdex_unstripped.elf has a LOAD segment with RWX permissions

   Obviously there are some security implications with the code segment
   being writable, but since our inputs are very limited, that's not
   something we worry about very much.  The warning is kind of annoying and
   we can try to fix that with PHDRS command, but doing so causes a load
   error on the device.  Rather than digging deeper into that rabbit hole,
   we will just live with the extra warning.
*/

SECTIONS
{
   .text :
   {
      *(.text)
      *(.text.*)

      KEEP(*(.init))
      KEEP(*(.fini))

      /* .ctors */
      *crtbegin.o(.ctors)
      *crtbegin?.o(.ctors)
      *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
      *(SORT(.ctors.*))
      *(.ctors)

      /* .dtors */
      *crtbegin.o(.dtors)
      *crtbegin?.o(.dtors)
      *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
      *(SORT(.dtors.*))
      *(.dtors)

      *(.rodata*)

      KEEP(*(.eh_frame*))
   }

   .data :
   {
      __etext = .;

      __data_start__ = .;
      *(vtable)
      *(.data*)

      . = ALIGN(4);
      /* preinit data */
      PROVIDE_HIDDEN (__preinit_array_start = .);
      KEEP(*(.preinit_array))
      PROVIDE_HIDDEN (__preinit_array_end = .);

      . = ALIGN(4);
      /* init data */
      PROVIDE_HIDDEN (__init_array_start = .);
      KEEP(*(SORT(.init_array.*)))
      KEEP(*(.init_array))
      PROVIDE_HIDDEN (__init_array_end = .);

      . = ALIGN(4);
      /* finit data */
      PROVIDE_HIDDEN (__fini_array_start = .);
      KEEP(*(SORT(.fini_array.*)))
      KEEP(*(.fini_array))
      PROVIDE_HIDDEN (__fini_array_end = .);

      . = ALIGN(4);
      /* All data end */
      __data_end__ = .;
   }

   .bss :
   {
      . = ALIGN(4);
      __bss_start__ = .;
      *(.bss*)
      *(COMMON)
      *(COM)
      . = ALIGN(4);
      __bss_end__ = .;
   }

   /DISCARD/ :
   {
      *(.ARM.exidx)
   }
}
