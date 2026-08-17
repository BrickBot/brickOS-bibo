#include <stdio.h>
#include <string.h>

#define MAX_LINE	1024

int main(int argc, char **argv) {
  char buffer[MAX_LINE];
  const char *kernel_prefix;
  int prefix_len;
  
  // get kernel prefix
  //
  if(argc<2) {
    fprintf(stderr,"usage: makedepend | %s kernelprefix > .depend\n",argv[0]);
    return -1;
  }
  kernel_prefix=argv[1];
  prefix_len=strlen(kernel_prefix);
  
  // process dependencies line by line.
  //
  while(fgets(buffer,MAX_LINE-1,stdin)) {
    const char *file,*deps;
    
    buffer[1023]=0;
    file=strtok(buffer,":");
    deps=strtok(NULL  ,":");
    
    // leave out kernel prefix
    //
    if(file && *file && deps && *deps) {
      if(!strncmp(file,kernel_prefix,prefix_len))
	file+=prefix_len;

      fprintf(stdout,"%s:%s",file,deps);
    }
  }

  return 0;
}
