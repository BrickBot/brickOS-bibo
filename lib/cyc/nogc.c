/* This file is part of the Cyclone Library.
   Copyright (C) 2001 AT&T

   This library is free software; you can redistribute it and/or it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place, Suite
   330, Boston, MA 02111-1307 USA. */

//*
//* Oct. 20, 2003
//* Zhongxi Shen, School of Computing, University of Utah
//* Modify it to fit BrickOS for Lego Mindstorms
//*


#include <mem.h> //* needed for size_t
#include <stdlib.h>

static size_t last_alloc_bytes = 0;

#define malloc_sizeb(p,n) (last_alloc_bytes = n)

/* hack: assumes this is called immediately after an allocation */
size_t GC_size(void *x) {
  return last_alloc_bytes;
}

size_t GC_get_heap_size() {
  return 0;
}

size_t GC_get_free_bytes() {
  return 0;
}

/* total number of bytes allocated */
static size_t total_bytes_allocd = 0;

void *GC_malloc(int x) {
  // FIX:  I'm calling calloc to ensure the memory is zero'd.  This
  // is because I had to define GC_calloc in runtime_cyc.c
  void *p = (void*)calloc(sizeof(char),x);
  total_bytes_allocd += malloc_sizeb(p,x);
  return p;
}

void *GC_malloc_atomic(int x) {
  // FIX:  I'm calling calloc to ensure the memory is zero'd.  This
  // is because I had to define GC_calloc in runtime_cyc.c
  void *p = (void*)calloc(sizeof(char),x);
  total_bytes_allocd += malloc_sizeb(p,x);
  return p;
}

void GC_free(void *x) {
  free(x);
}

unsigned int GC_gc_no = 0;
int GC_dont_expand = 0;
int GC_use_entire_heap = 0;

size_t GC_get_total_bytes() {
  return total_bytes_allocd;
}

void GC_set_max_heap_size(unsigned int sz) {
  return;
}
