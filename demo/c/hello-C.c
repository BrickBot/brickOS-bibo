#include <conio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  cputc_native_user(CHAR_H, CHAR_E, CHAR_PARALLEL, CHAR_O); // HELLO
  sleep(1);
  cputc_native_user(CHAR_L, CHAR_E, CHAR_G, CHAR_O);        //LEGO
  sleep(1);
  cls();
  sleep(1);
  
  return 0;
}
