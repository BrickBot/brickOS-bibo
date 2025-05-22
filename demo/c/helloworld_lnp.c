#include <conio.h>
#include <unistd.h>
#include <lnp.h>

int main(int argc, char **argv) {
#ifdef CONF_LNP
  cputc_native_user(0, CHAR_i, CHAR_r, 0);  // ir
  sleep(1);
#ifdef CONF_RCX_MESSAGE
  cputc_native_user(CHAR_L, CHAR_E, CHAR_G, CHAR_O);  // LEGO
  send_msg(0x09);
  sleep(1);
  cputc_native_user(CHAR_R, CHAR_e, CHAR_m, CHAR_m);  // Rem
  send_code(0x0002);    // Remote Message 2
  sleep(1);
#endif
  cputc_native_user(CHAR_I, CHAR_n, CHAR_t, CHAR_g);  // Intg
  lnp_integrity_puts("Hello, World!");
  sleep(1);
  cputc_native_user(CHAR_A, CHAR_d, CHAR_d, CHAR_r);  // Addr
  lnp_addressing_puts(0x00, 0x05, "Hello, World!");
  sleep(1);
#else
  cputc_native_user(CHAR_N, CHAR_O, CHAR_N, CHAR_E);  // None
  sleep(1);
#endif
  cls();
  return 0;
}
