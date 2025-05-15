/*
 * 
 *  legOS - the independent LEGO Mindstorms OS
 *  h8300.rcx - linker command file
 *  (c) 1999 by Markus L. Noga <markus@noga.de>    
 * 
 */

OUTPUT_FORMAT("coff-h8300")
OUTPUT_ARCH(h8300)
ENTRY("_main")

MEMORY {
  rom      : o = 0x0000, l = 0xffff
}

SECTIONS {

  .text : {
    *(.text) 	      /* must start with text for clean entry */
    *(.rodata)
    *(.strings)
    *(.vectors)       /* vectors region (dummy) */
    *(.data)
    *(.bss)
  } > rom

} /* SECTIONS */
