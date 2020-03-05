/*! \file   calloc.c
    \brief  calloc function
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
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */
#include <stdlib.h>
#include <string.h>

//! allocate adjacent blocks of memory
/*! \param nmemb number of blocks (must be > 0)
    \param size  individual block size (must be >0)
    \return 0 on error, else pointer to block
*/
void *calloc(size_t nmemb, size_t size) {
  void *ptr;
  size_t original_size = size;

  if (nmemb == 0 || size == 0)
    return 0;
 
  size*=nmemb;

  // if an overflow occurred, size/nmemb will not equal original_size
  if (size/nmemb != original_size)
    return 0;
  
  if((ptr=malloc(size))!=NULL)
    memset(ptr,0,size);
    
  return ptr;
}
