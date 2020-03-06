// LEGO Power Function sample code
// This program shows how to use some of the library fuctions.
// It contains examples of using PWM and other operation modes, and it shows
// how the extra address space can be used if you have two receivers set to
// the same channel.
// Usage: Use 2 IR receivers set to channel 1. Start with one switched on and
// one sswitched off. Start the program and press Prgm to set the IR receiver
// to use the extra address space. Then turn on the other IR receiver. (If you
// run the program again, press View instead of Prgm to skip the address space
// switch.) Watch as the program plays around with PWM speeds on blue and red
// output on both receivers.
//
//Bob Kojima 01-28-2008
//bob@fial.com
//v1.0.1

#include <dsensor.h>
#include <dbutton.h>
#include <powerfunctions.h>

#include <unistd.h>
#include <conio.h>

int main(int argc, char **argv)
{
	pf_init();
	pf_repeat = 0; // Default: don't resend IR messages

	// Wait for View key
	cputc_native_user(CHAR_p, CHAR_r, CHAR_g, CHAR_m);
	unsigned char k;
	while(!((k = dkey) & (KEY_PRGM | KEY_VIEW))) msleep(1);
	if(k == KEY_PRGM)
	{
		// Set active receivers to use the extra address space
		pf_align_toggle(PF_CH1, 0);
		pf_toggle_address(PF_CH1, 0);
	}
	cputc_native_user(CHAR_p, CHAR_f, CHAR_SPACE, CHAR_SPACE);

	// PWM with timeout
	pf_combo_pwm(PF_CH1, PWM_FWD5, PWM_FWD5, 1);
	msleep(400);

// For unknown reasons bibo hangs if I use dkey_wait or getchar.
//	dkey_wait(KEY_PRESSED, KEY_VIEW);

	cputc_native_user(CHAR_p, CHAR_f, CHAR_SPACE, CHAR_SPACE);
	// From here on control receivers in both address spaces

	int i;
	for(i = 0; i < 4; ++i)
	{
		// Single output without timeout
		pf_single_output_pwm(PF_CH1, PF_BLUE, PWM_REV1, 0);
		pf_single_output_pwm(PF_CH1, PF_RED, PWM_FWD1, 0);
		pf_single_output_pwm(PF_CH1, PF_BLUE, PWM_REV7, 1);
		pf_single_output_pwm(PF_CH1, PF_RED, PWM_FWD7, 1);
		sleep(1);

		// Reverse output directions
		pf_single_output(PF_CH1, PF_BLUE, PF_TOGGLE_DIR, 0);
		pf_single_output(PF_CH1, PF_RED, PF_TOGGLE_DIR, 0);
		pf_single_output(PF_CH1, PF_BLUE, PF_TOGGLE_DIR, 1);
		pf_single_output(PF_CH1, PF_RED, PF_TOGGLE_DIR, 1);

		// Gradually slow outputs to a halt
		int j;
		for(j = 0; j < 7; ++j)
		{
			msleep(400);
			pf_single_output(PF_CH1, PF_BLUE, PF_INC_NUM_PWM, 0);
			pf_single_output(PF_CH1, PF_RED, PF_INC_NUM_PWM, 0);
			pf_single_output(PF_CH1, PF_BLUE, PF_DEC_NUM_PWM, 1);
			pf_single_output(PF_CH1, PF_RED, PF_DEC_NUM_PWM, 1);
		}

		for(j = 0; j < 7; ++j)
		{
			msleep(400);
			pf_single_output(PF_CH1, PF_BLUE, PF_DEC_NUM_PWM, 0);
			pf_single_output(PF_CH1, PF_RED, PF_DEC_NUM_PWM, 0);
		}

		// Break (and then float) outputs
		pf_combo_pwm(PF_CH1, PWM_BRK, PWM_BRK, 0);
		pf_combo_pwm(PF_CH1, PWM_BRK, PWM_BRK, 1);
		sleep(1);
	}

	cls();
	return 0;
}

