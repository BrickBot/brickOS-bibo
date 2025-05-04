/*! \file   genlds.c
    \brief  BrickOS linker script generation
    \author Markus L. Noga <markus@noga.de>
*/

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is legOS code, released October 2, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 2000
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */
 
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_SYMBOLS 65536		//!< max symbols, enough for the RCX
#define MAX_SYMLEN  256			//!< max symbol length. 

#define BASE2_OFFSET 0x0210

//! symbol type.
typedef struct {
  unsigned addr;
  char text[MAX_SYMLEN];
} symbol_t;

//! the list of symbols
symbol_t symbols[MAX_SYMBOLS];

//! read the kernel symbols from a file
static unsigned read_symbols(FILE *f,symbol_t *symbols,unsigned max,unsigned *ram) {
  char buffer[MAX_SYMLEN];
  unsigned i=0;
  *ram = 0;
  
  for(; i<max; ) {
    char symtype;
    
    if(!fgets(buffer,MAX_SYMLEN,f))
      break;
    
    buffer[MAX_SYMLEN-1]=0;
    sscanf(buffer,"%x %c %s",&(symbols[i].addr),&symtype,symbols[i].text);

    // keep global symbols
    //
    if(symtype=='T' || symtype=='D' || symtype=='B') {
      // check for start of user RAM
      //
      if(!strcmp(symbols[i].text,"_mm_start"))
        *ram=symbols[i].addr+2;
      
      // skip special symbols
      //
      if(!strcmp(symbols[i].text,"_main"))
	continue;
      if(!strncmp(symbols[i].text,"___text",7))
	continue;
      if(!strncmp(symbols[i].text,"___data",7))
	continue;
      if(!strncmp(symbols[i].text,"___bss" ,6))
	continue;
      
      i++;
    }
  }
  
  return i;
}


//! print the kernel symbols in linker script format.
static void print_symbols(FILE *f,symbol_t *symbols,unsigned num_symbols) {
  unsigned i;
  for(i=0; i<num_symbols; i++)
    fprintf(f,"    %s = 0x%04x ;\n",symbols[i].text,symbols[i].addr-0x8000);
}


//! print the linker script header.
static void print_header(FILE *f,
		         const char *now,const char *kernel_name,
			 unsigned ram,unsigned kernlen, unsigned ramlen) {
    fprintf(f,
"/*\n\
 * \n\
 *  dynamic linker command file\n\
 *  generated: %s\n\
 *  kernel   : %s\n\
 *  app start: 0x%04x\n\
 * \n\
 *  BrickOS for LEGO(R) Mindstorms(TM)\n\
 *  Originally: legOS - the independent LEGO Mindstorms OS\n\
 *              (c) 1999 by Markus L. Noga <markus@noga.de>    \n\
 * \n\
 * \n\
 * The BASE1 and BASE2 lines below are kernel-size based values\n\
 * that may be used in Makefile.common\n\
 * By default, BASE1 = 0xb000 and BASE2 = 0xb210\n\
 * Calculated, BASE1 = kernel offset + kernel length and BASE2 = BASE1 + 0x%04x\n\
 * \n\
BASE1 = 0x%04x\n\
BASE2 = 0x%04x\n\
 * \n\
 */\n\
\n\
OUTPUT_FORMAT(\"symbolsrec\")\n\
OUTPUT_ARCH(h8300)\n\
ENTRY(\"_main\")\n\
\n\
MEMORY {\n\
  rom   : o = 0x0000, l = 0x8000\n\
  kern  : o = 0x8000, l = 0x%04x\n\
  ram   : o = 0x%04x, l = 0x%04x\n\
  stack : o = 0xfefc, l = 0x0004\n\
  eight : o = 0xff00, l = 0x0100\n\
}\n\
\n\
SECTIONS {\n\
\n\
  .rom : {\n\
    /* used rom functions */\n\
    \n\
    _rom_reset_vector = 0x0000;\n\
        \n\
    _reset        = 0x03ae ;\n\
    lcd_show      = 0x1b62 ;\n\
    lcd_hide      = 0x1e4a ;\n\
    lcd_number    = 0x1ff2 ;\n\
    lcd_clear     = 0x27ac ;\n\
    power_init    = 0x2964 ;\n\
    sound_system  = 0x299a ;\n\
    power_off     = 0x2a62 ;\n\
    sound_playing = 0x3ccc ;\n\
\n\
    _rom_dummy_handler   = 0x046a ;\n\
    _rom_ocia_handler    = 0x04cc ;\n\
    _rom_ocia_return     = 0x04d4 ;\n\
    \n\
  } > rom\n\
\n\
  .kernel :	{\n\
    /* kernel symbols (relative to 0x8000) */\n\
",
    now, kernel_name, ram, BASE2_OFFSET, ram, ram + BASE2_OFFSET,
    kernlen, ram, ramlen);
}


//! print the linker script footer.
static void print_footer(FILE *f) {
  fprintf(f,
"    /* end of kernel symbols */\n\
  }  > kern\n\
      \n\
  .text BLOCK(2) :	{\n\
    ___text = . ;\n\
    *(.text) 	      /* must start with text for clean entry */			\n\
    *(.rodata)\n\
    *(.strings)\n\
    *(.vectors)       /* vectors region (dummy) */\n\
    *(.persist)\n\
\n\
    ___text_end = . ;\n\
  }  > ram\n\
\n\
  .tors BLOCK(2) : {\n\
    ___ctors = . ;\n\
    *(.ctors)\n\
    ___ctors_end = . ;\n\
    ___dtors = . ;\n\
    *(.dtors)\n\
    ___dtors_end = . ;\n\
  }  > ram\n\
\n\
  .data BLOCK(2) : {\n\
    ___data = . ;\n\
    *(.data)\n\
    *(.tiny)\n\
    ___data_end = . ;\n\
  }  > ram\n\
\n\
  .bss BLOCK(2) : {\n\
    ___bss = . ;\n\
    *(.bss)\n\
    *(COMMON)\n\
    ___bss_end = ALIGN(2) ;\n\
  }  >ram\n\
\n\
  .stack : {\n\
    _stack = . ; \n\
    *(.stack)\n\
  }  > stack\n\
\n\
  .eight 0xff00: {\n\
    *(.eight)\n\
\n\
    /* on-chip module registers (relative to 0xff00) */\n\
\n\
    _T_IER = 0x90 ;\n\
    _T_CSR = 0x91 ;\n\
    _T_CNT = 0x92 ;\n\
    _T_OCRA = 0x94 ;\n\
    _T_OCRB = 0x94 ;\n\
    _T_CR = 0x96 ;\n\
    _T_OCR = 0x97 ;\n\
    _T_ICRA = 0x98 ;\n\
    _T_ICRB = 0x9a ;\n\
    _T_ICRC = 0x9c ;\n\
    _T_ICRD = 0x9e ;\n\
    _WDT_CSR = 0xa8 ;\n\
    _WDT_CNT = 0xa9 ;\n\
    _PORT1_PCR = 0xac ;\n\
    _PORT2_PCR = 0xad ;\n\
    _PORT3_PCR = 0xae ;\n\
    _PORT1_DDR = 0xb0 ;\n\
    _PORT2_DDR = 0xb1 ;\n\
    _PORT1 = 0xb2 ;\n\
    _PORT2 = 0xb3 ;\n\
    _PORT3_DDR = 0xb4 ;\n\
    _PORT4_DDR = 0xb5 ;\n\
    _PORT3 = 0xb6 ;\n\
    _PORT4 = 0xb7 ;\n\
    _PORT5_DDR = 0xb8 ;\n\
    _PORT6_DDR = 0xb9 ;\n\
    _PORT5 = 0xba ;\n\
    _PORT6 = 0xbb ;\n\
    _PORT7 = 0xbe ;\n\
    _STCR = 0xc3 ;\n\
    _SYSCR = 0xc4 ;\n\
    _T0_CR = 0xc8 ;\n\
    _T0_CSR = 0xc9 ;\n\
    _T0_CORA = 0xca ;\n\
    _T0_CORB = 0xcb ;\n\
    _T0_CNT = 0xcc ;\n\
    _T1_CR = 0xd0 ;\n\
    _T1_CSR = 0xd1 ;\n\
    _T1_CORA = 0xd2 ;\n\
    _T1_CORB = 0xd3 ;\n\
    _T1_CNT = 0xd4 ;\n\
    _S_MR = 0xd8 ;\n\
    _S_BRR = 0xd9 ;\n\
    _S_CR = 0xda ;\n\
    _S_TDR = 0xdb ;\n\
    _S_SR = 0xdc ;\n\
    _S_RDR = 0xdd ;\n\
    _AD_A = 0xe0 ;\n\
    _AD_A_H = 0xe0 ;\n\
    _AD_A_L = 0xe1 ;\n\
    _AD_B = 0xe2 ;\n\
    _AD_B_H = 0xe2 ;\n\
    _AD_B_L = 0xe3 ;\n\
    _AD_C = 0xe4 ;\n\
    _AD_C_H = 0xe4 ;\n\
    _AD_C_L = 0xe5 ;\n\
    _AD_D = 0xe6 ;\n\
    _AD_D_H = 0xe6 ;\n\
    _AD_D_L = 0xe7 ;\n\
    _AD_CSR = 0xe8 ;\n\
    _AD_CR = 0xe9 ;\n\
    \n\
    /* end of on-chip module registers */\n\
    \n\
  }  > eight\n\
\n\
  .stab 0 (NOLOAD) : {\n\
    [ .stab ]\n\
  }\n\
\n\
  .stabstr 0 (NOLOAD) : {\n\
    [ .stabstr ]\n\
  }\n\
\n\
} /* SECTIONS */\n\
"
    ); 
}


int main(int argc, char *argv[]) {
  const char *kernel_name;
  unsigned num_symbols;
  unsigned ram,kernlen,ramlen;
  time_t now_time;
  char *now;
  
  // determine kernel name
  //
  if(argc<2) {
    fprintf(stderr,"usage: %s kernelname < kernel.map\n",argv[0]);
    return -1;
  }
  kernel_name=argv[1];

  // parse kernel symbols
  //   
  num_symbols=read_symbols(stdin,symbols,MAX_SYMBOLS,&ram);
  
  // calculate kernel and ram size
  //
  kernlen=ram    - 0x8000;
  ramlen =0xfefc - ram;
 
  // create timestamp
  //
  now_time=time(NULL); 
  now     =ctime(&now_time);

  // print linker script
  //
  print_header (stdout,now,kernel_name,ram,kernlen,ramlen);
  print_symbols(stdout,symbols,num_symbols);
  print_footer (stdout);
  
  return 0;
}
