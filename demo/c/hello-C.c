#include <conio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // Display a friendly message on the RCX screen
  cputc_native_user(CHAR_H, CHAR_E, CHAR_PARALLEL, CHAR_O); // HELLO
  sleep(1);
  cputc_native_user(CHAR_L, CHAR_E, CHAR_G, CHAR_O);        // LEGO
  sleep(1);
  cls();
  sleep(1);
  
  // Use different methods and display the same message upside down
  cputc_hex_4(0x00);
  cputc_native_3(CHAR_PARALLEL);
  cputc_hex(0x03, 2);
  cputc_hex(0x04, 1);
  sleep(1);
  cputw(0x0637);    // LEGO
  sleep(1);
  cls();
  sleep(1);
  
  return 0;
}
