#ifndef __srecload_h__
#define __srecload_h__


#define IMAGE_START     0x8000
#define IMAGE_MAXLEN    0x7000

//! wrapper for an s-record image
typedef struct {
  unsigned short base;        	  //!< base address
  unsigned short text_size;	  //!< length from base address
  unsigned short data_size;
  unsigned short bss_size;
  unsigned short offset;      	  //!< start offset from base address
  
  unsigned char *text;	      	  //!< text at base address
} image_t;


//! load an RCX s-record file and return its image
/*! \param filename file to load
    \param img      image to store to
*/
extern void image_load(image_t *img, const char *filename);

extern int srec_load (char *name, unsigned char *image, int maxlen, unsigned short *start);

#endif
