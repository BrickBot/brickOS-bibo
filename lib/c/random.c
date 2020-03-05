/*! \file   random.c
    \brief  A portable random number generator.
    \author Copyright (c) 2000 Markus L. Noga <markus@noga.de>

    See Numerical Recipes in C 2nd Edition, p. 279-281 for inspiration.    
*/

//#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
/////////////////////////////////////////////////////////////////////////////

#define PRANDOM_SLOTS 16
#define PRANDOM_MASK  0x3fc91ab4

#define PRANDOM_A     16807
#define PRANDOM_M     2147483647
#define PRANDOM_Q     127773
#define PRANDOM_R     2836


//! A pseudo-random number generator
typedef struct {
  long select;
  long state;
  long entry[PRANDOM_SLOTS];
} prandom_t;


/////////////////////////////////////////////////////////////////////////////
//
// Static variables
//
/////////////////////////////////////////////////////////////////////////////

static prandom_t generator;   	//!< the generator used.


/////////////////////////////////////////////////////////////////////////////
//
// Global functions
//
/////////////////////////////////////////////////////////////////////////////

//! a primitive randomizing step, using a linear congruential generator.
void randstep(long *val) {
  long k;
  k=(*val)/PRANDOM_Q;
  *val=PRANDOM_A*(*val-k*PRANDOM_Q)-PRANDOM_R*k;
  if(*val<0)
    *val += PRANDOM_M;
}

//! Initialize pseudo-random number generator from seed.
void srandom(unsigned _seed) {
  unsigned i;
  long seed=_seed;
  
  // catch zero seeds etc.
  seed= (seed==PRANDOM_MASK) ? seed : seed^PRANDOM_MASK;

  // initialize array.
  randstep(&seed);
  generator.select=seed & (PRANDOM_SLOTS-1);    
  for(i=0; i<PRANDOM_SLOTS; i++) {
    randstep(&seed);
    generator.entry[i]=seed;
  }
  randstep(&seed);
  generator.state=seed;
}

//! Return a pseudo-random number in the range of MIN_INT to MAX_INT.
long random() {
  long res=generator.entry[generator.select];
  
  randstep(&generator.state);
  generator.entry[generator.select]=generator.state;
  generator.select=res & (PRANDOM_SLOTS-1);

  return res;
}
