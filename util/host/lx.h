/*! \file   lx.h
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
 
#ifndef __lx_h__
#define __lx_h__

#define HEADER_FIELDS 8       //!< number of header fields stored on disk

typedef struct {
  unsigned short version;     //!< version number
  unsigned short base;        //!< current text segment base address
  unsigned short text_size;   //!< size of read-only segment 
  unsigned short data_size;   //!< size of initialized data segment
  unsigned short bss_size;    //!< size of uninitialized data segment
  unsigned short stack_size;  //!< stack size
  unsigned short offset;      //!< start offset from text
  unsigned short num_relocs;  //!< number of relocations.
  
  unsigned char  *text;       //!< program text (not stored on disk)
  unsigned short *reloc;      //!< relocations (not stored on disk)
} lx_t;       	      	      //!< the BrickOS executable type



//! write a BrickOS exectutable to a file
int lx_write(const lx_t *lx,const char *filename);

//! read a BrickOS executable from a file
int lx_read(lx_t *lx,const char *filename);

//! relocate a BrickOS executable to a new base address (may be called repeatedly).
void lx_relocate(lx_t *lx,unsigned short base);

#endif // __lx_h__

