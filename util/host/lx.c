/*! \file   lx.c
    \brief  BrickOS executable file support
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
 *  Contributor(s): everyone discussing LNP at LUGNET
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>

#include <lx.h>


#define ASSURED_WRITE(fd,buf,len) \
  if((rc=write(fd,buf,len))!=(len)) { \
    close(fd); \
    return rc; \
  }
  
#define ASSURED_READ(fd,buf,len) \
  if((rc=read(fd,buf,len))!=(len)) { \
    close(fd); \
    return rc; \
  }
  
  
int lx_write(const lx_t *lx,const char *filename) {
#if defined(_WIN32)
  int i,rc,fd=open(filename,O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,S_IRUSR | S_IWUSR | S_IRGRP);
#else
  int i,rc,fd=creat(filename,S_IRUSR | S_IWUSR | S_IRGRP);
#endif
  unsigned short tmp;
  
  if(fd<0)
    return fd;

  // write ID
  //
  ASSURED_WRITE(fd,"brickOS",8);
  // write header data in MSB
  //
  for(i=0; i<HEADER_FIELDS; i++) {
    tmp=htons( ((unsigned short*)lx)[i] );
    ASSURED_WRITE(fd,&tmp,2);
  }

  // write program text (is MSB, because H8 is MSB)
  //
  ASSURED_WRITE(fd,lx->text,lx->text_size + lx->data_size);
  
  // write relocation data in MSB
  //
  for(i=0; i<lx->num_relocs; i++) {
    tmp=htons( lx->reloc[i] );
    ASSURED_WRITE(fd,&tmp,2);
  }
  
  close(fd);
  return 0;
}

int lx_read(lx_t *lx,const char *filename) {
#if defined(_WIN32)
  int i,rc,fd=open(filename,O_RDONLY | O_BINARY);
#else
  int i,rc,fd=open(filename,O_RDONLY);
#endif
  char buffer[8];
  unsigned short tmp;
    
  if(fd<0)
    return fd;
  
  // check ID
  //
  ASSURED_READ(fd,buffer,8);
  if(strcmp((char*)buffer,"brickOS")) {
    close(fd);
    return -1;
  }
  
  // read header data in MSB
  //
  for(i=0; i<HEADER_FIELDS; i++) {
    ASSURED_READ(fd,&tmp,2);
    ((unsigned short*)lx)[i]= ntohs(tmp);
  }
  
  // read program text (is MSB, because H8 is MSB)
  //
  if((lx->text=malloc(lx->text_size + lx->data_size))==0) {
    close(fd);
    return -1;
  }
  ASSURED_READ(fd,lx->text,lx->text_size + lx->data_size);

  // read relocation data in MSB
  //
  if(lx->num_relocs) {
    if((lx->reloc=malloc(sizeof(unsigned short)*lx->num_relocs))==0) {
      close(fd);
      return -1;
    }

    for(i=0; i<lx->num_relocs; i++) {
      ASSURED_READ(fd,&tmp,2);
      lx->reloc[i]=ntohs(tmp);
    }
  }
  
  return 0;
}

void lx_relocate(lx_t *lx,unsigned short base) {
  unsigned short diff=base-lx->base;
  
  int i;
  
  for(i=0; i<lx->num_relocs; i++) {
    unsigned char *ptr=lx->text+lx->reloc[i];
    unsigned short off =(ptr[0]<<8) | ptr[1];
    
    off+=diff;
    ptr[0]=off >> 8;
    ptr[1]=off & 0xff;
  }
  
  lx->base=base;
}
