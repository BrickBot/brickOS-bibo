/*
 * 
 *  legOS - the independent LEGO Mindstorms OS
 *  h8300-rcx.ld - linker command file
 *  (c) 1999 by Markus L. Noga <markus@noga.de>    
 * 
 */

OUTPUT_FORMAT("coff-h8300")
OUTPUT_ARCH(h8300)
ENTRY("_kmain")

MEMORY {
  rom (r)  : o = 0x0000, l = 0x8000

  ram      : o = 0x8000, l = 0x6000
  ram1     : o = 0xe000, l = 0x0f30   /* init text memory, freed later */
  lcddata  : o = 0xef30, l = 0x0020   /* display memory */
  ram2     : o = 0xef50, l = 0x00b0
  ram3     : o = 0xf000, l = 0x0b80
  romdata  : o = 0xfd80, l = 0x0040   /* port shadows, interrupt vectors */
  ram4     : o = 0xfdc0, l = 0x01c0

  eight    : o = 0xff80, l = 0x0100
}

SECTIONS {

  .rom : {
    /* used rom functions */
    
    _rom_reset_vector = 0x0000;
        
    _reset        = 0x03ae ;
    rom_memcpy    = 0x042a ;
    lcd_show      = 0x1b62 ;
    lcd_hide      = 0x1e4a ;
    lcd_number    = 0x1ff2 ;
    lcd_clear     = 0x27ac ;
    power_init    = 0x2964 ;
    sound_system  = 0x299a ;
    power_off     = 0x2a62 ;
    sound_playing = 0x3ccc ;

    _rom_dummy              = 0x046a ;
    _rom_ocia_trampoline    = 0x04cc ;
    _rom_ocia_return        = 0x04d4 ;
    _rom_ocib_trampoline    = 0x04d8 ;
    _rom_rxi_trampoline     = 0x0544 ;
    _rom_txi_trampoline     = 0x0550 ;
    _rom_tei_trampoline     = 0x055c ;
    _rom_adi_trampoline     = 0x0568 ;
    _rom_wovf_trampoline    = 0x0574 ;

  } > rom
    
  .text :	{
    ___text = . ;
    *(.text.ma)       /* must start with text.ma for clean entry */
    *(.text) 	 
    *(.rodata)
    *(.strings)
    *(.vectors)       /* vectors region (dummy) */
    ___text_end = . ;
  } > ram

  .tors : {
    ___ctors = . ;
    *(.ctors)
    ___ctors_end = . ;
    ___dtors = . ;
    *(.dtors)
    ___dtors_end = . ;
  } > ram

  .data : {
    ___data = . ;
    *(.data)
    *(.tiny)
    ___data_end = . ;
  } > ram

  .text.init : AT (___data_end) {
    ___text_init = . ;
    *(.text.in)
    ___text_init_end = . ;
  } > ram1
  ___text_init_load = LOADADDR(.text.init);
  ___text_init_size = SIZEOF(.text.init);

  .text.hi : AT (___data_end + ___text_init_size) {
    ___text_hi = . ;
    *(.text.hi)
    ___text_hi_end = . ;
  } > ram3
  ___text_hi_load = LOADADDR(.text.hi);
  ___text_hi_size = SIZEOF(.text.hi);

  /* firmware recognition string:
   * the ROM checks for this string when validating new firmware
   */
  .knock : AT (___data_end + ___text_init_size + ___text_hi_size) {
    /* "Do you byte, when I knock?" */
    LONG(0x446f2079)
    LONG(0x6f752062)
    LONG(0x7974652c)
    LONG(0x20776865)
    LONG(0x6e204920)
    LONG(0x6b6e6f63)
    SHORT(0x6b3f)
  } > rom

  .bss ___data_end : AT (___data_end + ___text_init_size + ___text_hi_size
                         + SIZEOF(.knock)) {
    ___bss     = . ;
    _bss_start = . ; 
    *(.bss)
    *(COMMON)
    ___bss_end = ALIGN(2) ;
    _mm_start  = ALIGN(2) ;    /* start memory management here */
    _end       = ALIGN(2) ;  
  } >ram

  .lcddata : {
    _display_memory =  0xef43 - 0xef30 ; 
    
  } > lcddata
    
  .romdata : {
    /* shadow registers */
    
    _rom_port1_ddr = 0x00 ; 
    _rom_port2_ddr = 0x01 ; 
    _rom_port3_ddr = 0x02 ; 
    _rom_port4_ddr = 0x03 ; 
    _rom_port5_ddr = 0x04 ; 
    _rom_port6_ddr = 0x05 ; 
    _rom_port7_pin = 0x06 ; 
    
    /* interrupt vectors */
    
    _reset_vector = 0x10 ; 
    _nmi_vector   = 0x12 ; 
    _irq0_vector  = 0x14 ; 
    _irq1_vector  = 0x16 ; 
    _irq2_vector  = 0x18 ; 
    _icia_vector  = 0x1a ; 
    _icib_vector  = 0x1c ; 
    _icic_vector  = 0x1e ; 
    _icid_vector  = 0x20 ; 
    _ocia_vector  = 0x22 ; 
    _ocib_vector  = 0x24 ; 
    _fovi_vector  = 0x26 ; 
    _cmi0a_vector = 0x28 ; 
    _cmi0b_vector = 0x2a ; 
    _ovi0_vector  = 0x2c ; 
    _cmi1a_vector = 0x2e ; 
    _cmi1b_vector = 0x30 ; 
    _ovi1_vector  = 0x32 ; 
    _eri_vector   = 0x34 ; 
    _rxi_vector   = 0x36 ; 
    _txi_vector   = 0x38 ; 
    _tei_vector   = 0x3a ; 
    _ad_vector    = 0x3c ; 
    _wovf_vector  = 0x3e ; 

  } > romdata

  .stack : {
    _stack = . ; 
    *(.stack)
  } > ram4

  .eight : {
    *(.eight)

    /* memory-mapped motor controller */    

    _motor_controller = 0x00;

    /* on-chip module registers (relative to 0xff80) */

    _T_IER     = 0x10 ;
    _T_CSR     = 0x11 ;
    _T_CNT     = 0x12 ;
    _T_OCRA    = 0x14 ;
    _T_OCRB    = 0x14 ;
    _T_CR      = 0x16 ;
    _T_OCR     = 0x17 ;
    _T_ICRA    = 0x18 ;
    _T_ICRB    = 0x1a ;
    _T_ICRC    = 0x1c ;
    _T_ICRD    = 0x1e ;
    _WDT_CSR   = 0x28 ;
    _WDT_CNT   = 0x29 ;
    _PORT1_PCR = 0x2c ;
    _PORT2_PCR = 0x2d ;
    _PORT3_PCR = 0x2e ;
    _PORT1_DDR = 0x30 ;
    _PORT2_DDR = 0x31 ;
    _PORT1     = 0x32 ;
    _PORT2     = 0x33 ;
    _PORT3_DDR = 0x34 ;
    _PORT4_DDR = 0x35 ;
    _PORT3     = 0x36 ;
    _PORT4     = 0x37 ;
    _PORT5_DDR = 0x38 ;
    _PORT6_DDR = 0x39 ;
    _PORT5     = 0x3a ;
    _PORT6     = 0x3b ;
    _PORT7     = 0x3e ;
    _STCR      = 0x43 ;
    _SYSCR     = 0x44 ;
    _T0_CR     = 0x48 ;
    _T0_CSR    = 0x49 ;
    _T0_CORA   = 0x4a ;
    _T0_CORB   = 0x4b ;
    _T0_CNT    = 0x4c ;
    _T1_CR     = 0x50 ;
    _T1_CSR    = 0x51 ;
    _T1_CORA   = 0x52 ;
    _T1_CORB   = 0x53 ;
    _T1_CNT    = 0x54 ;
    _S_MR      = 0x58 ;
    _S_BRR     = 0x59 ;
    _S_CR      = 0x5a ;
    _S_TDR     = 0x5b ;
    _S_SR      = 0x5c ;
    _S_RDR     = 0x5d ;
    _AD_A      = 0x60 ;
    _AD_A_H    = 0x60 ;
    _AD_A_L    = 0x61 ;
    _AD_B      = 0x62 ;
    _AD_B_H    = 0x62 ;
    _AD_B_L    = 0x63 ;
    _AD_C      = 0x64 ;
    _AD_C_H    = 0x64 ;
    _AD_C_L    = 0x65 ;
    _AD_D      = 0x66 ;
    _AD_D_H    = 0x66 ;
    _AD_D_L    = 0x67 ;
    _AD_CSR    = 0x68 ;
    _AD_CR     = 0x69 ;
    
    /* end of on-chip module registers */

  } > eight

  .stab 0 (NOLOAD) : {
    [ .stab ]
  }

  .stabstr 0 (NOLOAD) : {
    [ .stabstr ]
  }

} /* SECTIONS */
