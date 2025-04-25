/*! \file   p/stub.c
    \brief  A collection of dirty hacks to get Pascal compilation going.
    \author John Hansen <JohnBinder@aol.com>
    
    This stub provides the necessary runtime environment for basic Pascal.
*/

typedef unsigned size_t;
extern void* malloc(size_t);
extern void free(void*);
    
void _p_finalize(void)
{
  // do nothing;
}

void _p_atexit(void)
{
  // do nothing;
}

void _p_check_inoutres(void)
{
  // do nothing;
}

int _p_stdout = 0;

int _p_InOutRes = 0;

void _p_initialize(int argc, char *argv[], char *env[])
{
  // do nothing.
}

void _p_doinitproc(void)
{
  // do nothing.
}

void * _p_new(unsigned int size) {
  return malloc(size);
}

void _p_dispose(void* ptr) {
  free(ptr);
}
