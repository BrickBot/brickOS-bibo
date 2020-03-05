/*! \file   srecload.c
    \brief  load symbol-srec files
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
 *  Contributor(s): Markus L. Noga
 */

/*
 *  Copyright (C) 1998, 1999, Kekoa Proudfoot.  All Rights Reserved.
 *
 *  License to copy, use, and modify this software is granted provided that
 *  this notice is retained in any copies of any part of this software.
 *
 *  The author makes no guarantee that this software will compile or
 *  function correctly.  Also, if you use this software, you do so at your
 *  own risk.
 *
 *  Kekoa Proudfoot
 *  kekoa@graphics.stanford.edu
 *  10/3/98
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "srec.h"
#include "srecload.h"

#define IMG_LOWEST    0x0000
#define IMG_HIGHEST   0xffff
#define IMG_MAXSIZE   0x10000


//! load symbols from symbolsrec file
/*! \return current line in file

    the symbol section is enclosed in $$ 
    symbol line format is SYMBOL $ADDR
*/
static int symbols_load(image_t *img,FILE *file) {
  char buf[256];
  int doubledollar=0;
  unsigned short text=0,
                 text_end=0,
      	      	 data=0,
      	      	 data_end=0,
                 bss=0,
                 bss_end=0,
                 ctors=0,
                 ctors_end=0,
                 dtors=0,
                 dtors_end=0;
  int line=0;
    
  // read in symbols
  //
  while(doubledollar<2 && fgets(buf, 256, file)) {
    int i;
    line++;

    // skip empty lines
    //
    for(i=0; buf[i] && isspace(buf[i]); i++)
      ;
    if (!buf[i])
	continue;
    
    if(buf[i]=='$' && buf[i+1]=='$') {
      // delimiter?
      //
      doubledollar++;
    } else {
      // symbol / address pair
      //
      char *symbol;
      unsigned short address;
      
      if(doubledollar<1) {
	fprintf(stderr,"malformed symbolsrec file at line %d\n",line);
	exit(-1);
      }
      
      // read symbol
      //
      symbol=buf+i;
      while(buf[i] && buf[i]!=' ')
	i++;
      if(buf[i]!=' ') {
	fprintf(stderr,"malformed symbolsrec file at line %d\n",line);
	exit(-1);
      }
      buf[i]=0;

      if (buf[i+1]=='$')
	  i++;
      
      address=(unsigned short) strtoul(buf+i+1,NULL,16);
  
      // retain relevant offsets
      //
      if(!strcmp(symbol,"___text"))
	text=address; 
      else if(!strcmp(symbol,"___text_end"))
	text_end=address; 
      else if(!strcmp(symbol,"___data"))
	data=address; 
      else if(!strcmp(symbol,"___data_end"))
	data_end=address; 
      else if(!strcmp(symbol,"___bss"))
	bss=address; 
      else if(!strcmp(symbol,"___bss_end"))
	bss_end=address; 
      else if(!strcmp(symbol,"___ctors"))
	ctors=address; 
      else if(!strcmp(symbol,"___ctors_end"))
	ctors_end=address; 
      else if(!strcmp(symbol,"___dtors"))
	dtors=address; 
      else if(!strcmp(symbol,"___dtors_end"))
	dtors_end=address; 
    }   
  }

  // save general file information
  //
  img->base     =text;  
  //  added ctor/dtor sections for C++.  They reside between text and data
  //   so they've been added to text for simplicity.  -stephen 14Jan01
  img->text_size=(text_end - text) + (ctors_end - ctors) + (dtors_end - dtors);
  img->data_size=data_end - data;
  img->bss_size = bss_end - bss;
  
  return line;
}


void image_load(image_t *img,const char *filename)
{
  FILE *file;
  char buf[256];
  srec_t srec;
  int line;
  unsigned short size;

  if ((file = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "%s: failed to open\n", filename);
    exit(1);
  }

  // read symbols from file
  //
  line=symbols_load(img,file);
  size=img->text_size+img->data_size;
  
  if((img->text=calloc(size,1))== NULL) {
    fprintf(stderr, "out of memory\n");
    exit(1);
  }
        
  // Build an image of the srecord data 
  //
  while (fgets(buf, 256, file)) {
    int i,error;
    line++;

    // skip empty lines
    //
    for(i=0; buf[i] && isspace(buf[i]); i++)
      ;
    if (!buf[i])
	continue;

    // decode line
    //
    if ((error = srec_decode(&srec, buf)) < 0) {
	char *errstr = srec_strerror(error);
	if (errstr) {
	  fprintf(stderr, "%s: %s on line %d\n", filename, errstr, line);
	  exit(1);
	}
    }
    
    // handle lines
    //
    if (srec.type == 1) {
      if (srec.addr < img->base || srec.addr + srec.count > img->base + size) {
	fprintf(stderr, "%s: address [0x%4.4lX, 0x%4.4lX] out of bounds [0x%4.4X-0x%4.4X] on line %d\n",filename, srec.addr, (srec.addr + srec.count), img->base, (img->base + size), line);
	exit(1);
      }

      memcpy(img->text + srec.addr - img->base, srec.data, srec.count);
    }
    else if (srec.type == 9) {
      img->offset = srec.addr - img->base;
    }
  }
}

int srec_load (char *name, unsigned char *image, int maxlen, unsigned short *start)
{
    FILE *file;
    char buf[256];
    srec_t srec;
    int line = 0;
    int length = 0;
    int strip = 0;

    /* Initialize starting address */
    *start = IMAGE_START;

    /* Open file */
    if ((file = fopen(name, "r")) == NULL) {
		fprintf(stderr, "%s: ERROR- failed to open\n", name);
		exit(1);
    }

    /* Clear image to zero */
    memset(image, 0, maxlen);

    /* Read image file */
    while (fgets(buf, sizeof(buf), file)) {
	int error, i;
	line++;
	/* Skip blank lines */
	for (i = 0; buf[i]; i++)
	    if (!isspace(buf[i]))
		break;
	if (!buf[i])
	    continue;
	/* Decode line */
	if ((error = srec_decode(&srec, buf)) < 0) {
	    if (error != SREC_INVALID_CKSUM) {
		fprintf(stderr, "%s: %s on line %d\n",
			name, srec_strerror(error), line);
		exit(1);
	    }
	}
	/* Detect Firm0309.lgo header, set strip=1 if found */
	if (srec.type == 0) {
	    if (srec.count == 16)
		if (!strncmp((char*)srec.data, "?LIB_VERSION_L00", 16))
		    strip = 1;
	}
	/* Process s-record data */
	else if (srec.type == 1) {
	    if (srec.addr < IMAGE_START ||
		srec.addr + srec.count > IMAGE_START + maxlen) {
		fprintf(stderr, "%s: address out of bounds on line %d\n",
			name, line);
		exit(1);
	    }
	    if (srec.addr + srec.count - IMAGE_START > length)
		length = srec.addr + srec.count - IMAGE_START;
	    memcpy(&image[srec.addr - IMAGE_START], &srec.data, srec.count);
	}
	/* Process image starting address */
	else if (srec.type == 9) {
	    if (srec.addr < IMAGE_START ||
		srec.addr > IMAGE_START + maxlen) {
		fprintf(stderr, "%s: address out of bounds on line %d\n",
			name, line);
		exit(1);
	    }
	    *start = srec.addr;
	}
    }

    /* Strip zeros */
#ifdef FORCE_ZERO_STRIPPING
    strip = 1;
#endif

    if (strip) {
	int pos;
	for (pos = IMAGE_MAXLEN - 1; pos >= 0 && image[pos] == 0; pos--);
	length = pos + 1;
    }

    /* Check length */
    if (length == 0) {
	fprintf(stderr, "%s: image contains no data\n", name);
	exit(1);
    }

    return length;
}
