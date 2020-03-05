/*! \file   covert.c
    \brief  convert two suitable dsrec files into a BrickOS executable
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
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s):
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#include <srecload.h>
#include <lx.h>

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#define DEFAULT_STACK_SIZE    1024    //!< default program stack size
#define RELOC_MAX           16384   //!< maximum number of relocations


#if (defined(__sun__) && defined(__svr4__)) || defined(BSD) // Solaris||BSD
#undef HAVE_GETOPT_LONG
#else
#define HAVE_GETOPT_LONG 1
#endif

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>

//! long command-line options
static const struct option long_options[]={
  {"stack"  ,required_argument,0,'s'},
  {"verbose",no_argument      ,0,'v'},
  {"display",no_argument      ,0,'d'},
  {0        ,0                ,0,0  }
};

#else // HAVE_GETOPT_LONG

#define getopt_long(ac, av, opt, lopt, lidx) (getopt((ac), (av), (opt)))

#endif // HAVE_GETOPT_LONG

int verbose_flag=0;                   //!< display some diagnostics if non-zero


//! build BrickOS executable from images
/*! the segment sizes and start offset need to be set already.
*/
void lx_from_images(lx_t *lx,image_t *img,unsigned short stack_size) {
  const unsigned char diff_hi=(img[1].base - img[0].base) >> 8;
  const unsigned char diff_lo=(img[1].base - img[0].base) & 0xff;
  const unsigned short size=img[0].text_size + img[0].data_size;
  
  int i;

  if(verbose_flag)
    printf("diff_hi=0x%02x diff_lo=0x%02x\n",diff_hi,diff_lo);
  
  // trivial checks
  //
  if(img[0].text_size != img[1].text_size || 
     img[0].data_size != img[1].data_size ||
     img[0].bss_size  != img[1].bss_size     ) {
       
    fprintf(stderr,"image size mismatch\n");
    exit(-1);
  }
  if(img[0].offset != img[1].offset) {
    fprintf(stderr,"starting offset mismatch\n");
    exit(-1);
  }
  if(!diff_hi || !diff_lo) {
    fprintf(stderr,"need images whose base offsets differ in high and low byte\n");
    exit(-1);
  }

  // create BrickOS executable header
  //
  lx->version   =0;
  lx->base      =img[0].base;
  lx->text_size =img[0].text_size;
  lx->data_size =img[0].data_size;
  lx->bss_size  =img[0].bss_size;
  lx->stack_size=stack_size;
  lx->offset    =img[0].offset;
  lx->num_relocs=0;

  if(verbose_flag)
    printf("base     =0x%04x offset   =0x%04x\n"
     "text_size=0x%04x data_size=0x%04x bss_size=0x%04x stack_size=0x%04x\n",
     lx->base,lx->offset,lx->text_size,lx->data_size,lx->bss_size,lx->stack_size);
  
  if((lx->reloc=malloc(RELOC_MAX*sizeof(unsigned short)))==NULL) {
    fprintf(stderr,"out of memory\n");
    exit(-1);
  }
  if((lx->text=malloc(size))==NULL) {
    fprintf(stderr,"out of memory\n");
    exit(-1);
  }
  memcpy(lx->text,img[0].text,size);
    
  // compare images & build relocation table.
  //
  for(i=0; i<size; i++) {
    unsigned char c0=img[0].text[i];
    unsigned char c1=img[1].text[i];

    if(verbose_flag)
      printf("0x%04x: %02x - %02x = %02x\n",i,c0,c1,0xff & (c1-c0));
      
    if(c0!=c1) {
      unsigned char l0=img[0].text[i+1];
      unsigned char l1=img[1].text[i+1];

      if(l0 == l1) {
  fprintf(stderr,"single byte difference at +0x%04x\n",i);
        exit(-1);
      } else {
  unsigned short addr0=((c0<<8) | l0) - img[0].base;
  unsigned short addr1=((c1<<8) | l1) - img[1].base;

  if(addr0!=addr1) {
    fprintf(stderr,"relocation offset mismatch at +0x%04x\n",i);
    exit(-1);
  }

  if(verbose_flag)
      printf("reloc[%d]=0x%04x\n",lx->num_relocs,i);
  
  lx->reloc[lx->num_relocs++]=(unsigned short)i++;
      }
    }
  }
}    

//! do everything.
int main(int argc, char **argv) {
  image_t img[2];
  lx_t lx;
  int opt;
  int display_flag = 0;
  unsigned short stack_size=DEFAULT_STACK_SIZE;
#ifdef HAVE_GETOPT_LONG
  int option_index;
#endif
      
  // read command-line options
  //  
  while((opt=getopt_long(argc, argv, "s:vd",
                        (struct option *)long_options, &option_index) )!=-1) {
    unsigned tmp;
    
    switch(opt) {
      case 's':
  sscanf(optarg,"%x",&tmp);
  stack_size=(unsigned short)tmp;
        break;
      case 'v':
  verbose_flag=1;
  break;
      case 'd':
  display_flag=1;
  break;
    }
  }           
  
  if(argc-optind<3) {
    fprintf(stderr,"usage: %s file.ds1 file.ds2 file.lx\n"
             "       [-s<stacksize>] [-v] [-d]\n"
             "       size in hex, please.\n",
             argv[0]);
    exit(1);
  }
  
  image_load(img  , argv[optind++]);
  image_load(img+1, argv[optind++]);
  lx_from_images(&lx,img,stack_size);
    
  if(lx_write(&lx, argv[optind])) {
    fprintf(stderr,"error writing %s\n",argv[optind]);
    return -1;
  } 

  if (display_flag) {
    printf("version=%d\n", lx.version);
    printf("base=0x%x\n", lx.base);
    printf("text_size=%d\n", lx.text_size);
    printf("data_size=%d\n", lx.data_size);
    printf("bss_size=%d\n", lx.bss_size);
    printf("stack_size=%d\n", lx.stack_size);
    printf("start_offset=%d\n", lx.offset);
    printf("num_relocs=%d\n", lx.num_relocs);
    printf("contiguous free memory required to download=%d\n", 
     lx.text_size + lx.data_size*2 + lx.bss_size);
  }
  
  return 0;
}
