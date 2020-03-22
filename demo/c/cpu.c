#include <unistd.h>
#include <conio.h>
#include <time.h>
#include <dsensor.h>

 // Calibrated delay loop (in milliseconds)

 // Explanation: The number of states per instruction is given
 // in the comment field.  The state counts assume instruction
 // and stack memory fetches are in external RAM.  Each
 // iteration of the outer loop takes 16000 states which
 // is one millisecond (the RCX cpu clock is 16MHz).
 
extern void delay_ms(int ms);

__asm__("\n\
 .text\n\
 .align 1\n\
 _delay_ms:\n\
   ; subtract 3 for function call overhead (approximate)\n\
   mov #664-3,r1  ;12\n\
 \n\
 1:\n\
   subs #1,r1     ;6\n\
   mov r1,r1      ;6\n\
   bne 1b         ;12\n\
 \n\
   ; delay to make outer loop exactly 16000 states\n\
   jmp 2f         ;14\n\
 2:\n\
   jmp 2f         ;14\n\
 2:\n\
 \n\
   mov #664,r1    ;12\n\
 \n\
   subs #1,r0     ;6\n\
   mov r0,r0      ;6\n\
   bne 1b         ;12\n\
 \n\
   rts            ;20\n\
 ");

// displays cpu % available
void cpu()
{
   time_t t0, t;
 
   t0 = get_system_up_time();
   delay_ms(1000);
   t = get_system_up_time();
 
   lcd_int(100L * 1000L / (t - t0));
}
 
int main(int argc, char **argv)
{
   while (!shutdown_requested())
     {

     // how much cpu you normally get (32%)
     cputc_native_user(CHAR_i, CHAR_d, CHAR_l, CHAR_e);  // idle
     msleep(100);
     cpu();
     sleep(3);
 
#ifdef CONF_DSENSOR_ROTATION
     // with two rotation sensors enabled (23%)
     cputc_native_user(CHAR_2, CHAR_r, CHAR_o, CHAR_t); // 2rot
     msleep(100);
     ds_rotation_on(&SENSOR_1);
     ds_rotation_on(&SENSOR_2);
     cpu();
     ds_rotation_off(&SENSOR_1);
     ds_rotation_off(&SENSOR_2);
     sleep(3);
#endif
 
#ifdef CONF_DSENSOR_EDGECOUNT
     // with two edge count sensors enabled
     cputc_native_user(CHAR_2, CHAR_e, CHAR_d, CHAR_g); // 2edg
     msleep(100);
     ds_edgecount_on(&SENSOR_1);
     ds_edgecount_on(&SENSOR_2);
     cpu();
     ds_edgecount_off(&SENSOR_1);
     ds_edgecount_off(&SENSOR_2);
     sleep(3);
#endif
 
     // with A/D interrupts disabled (95%)
     cputc_native_user(CHAR_n, CHAR_o, CHAR_A, CHAR_d);  // noAd
     msleep(100);
     bit_clear(&AD_CSR, 6);
     cpu();
     bit_set(&AD_CSR, 6);
     sleep(3);

     // with all IRQs but NMI disabled (101% ;-)
     cputc_native_user(CHAR_n, CHAR_I, CHAR_r, CHAR_q);  // nIrq
     msleep(100);
     __asm__ __volatile__("\torc  #0x80,ccr\n":::"cc");
     cpu();
     __asm__ __volatile__("\tandc #0x7f,ccr\n":::"cc");
     sleep(3);
     }
 
   return 0;
}
 

