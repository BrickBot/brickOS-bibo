#include "swmux.h"

#ifdef CONF_DSENSOR
#ifdef CONF_DSENSOR_SWMUX


//sampled multiplexor ranges

//0xFE00 - sampled raw value for no sensors touched
#define SWMUX_0000_S    0xF000
#define SWMUX_0000_E    0xFFFF

//0x9640 - sampled raw value for only A touched
#define SWMUX_A000_S    0x9000
#define SWMUX_A000_E    0x9FFF

//Ox8180 - sampled raw value for A and B touched
#define SWMUX_AB00_S    0x8000
#define SWMUX_AB00_E    0x8BFF

//0x8D80 - sampled raw value for A and C touched
#define SWMUX_A0C0_S    0x8C00
#define SWMUX_A0C0_E    0x8FFF

//Ox7800 - sampled raw value for A, B, and C touched
#define SWMUX_ABC0_S    0x7000
#define SWMUX_ABC0_E    0x7FFF

//0xC000 - sampled raw value for only B touched
#define SWMUX_0B00_S    0xBFF0
#define SWMUX_0B00_E    0xC5FF

//0xA6C0 - sampled raw value for B and C touched
#define SWMUX_0BC0_S    0xA000
#define SWMUX_0BC0_E    0xAFFF

//OxD6C0 - sampled raw value for only C touched
#define SWMUX_00C0_S    0xD000
#define SWMUX_00C0_E    0xDFFF

//exta sensor D, when pressed A,B,C are masked out
//0x0600
#define SWMUX_XXXD_S    0x0000
#define SWMUX_XXXD_E    0x4FFF



unsigned char swmux_bits(unsigned int raw) {
    if(raw>=SWMUX_0000_S && raw <=SWMUX_0000_E) {	
	return 0;
    } else if (raw>=SWMUX_A000_S  && raw <=SWMUX_A000_E) {
	return SWMUX_A_MASK;
    } else if (raw>=SWMUX_AB00_S  && raw <=SWMUX_AB00_E) {
	return SWMUX_A_MASK|SWMUX_B_MASK;
    } else if (raw>=SWMUX_A0C0_S  && raw <=SWMUX_A0C0_E) {
	return SWMUX_A_MASK|SWMUX_C_MASK;
    } else if (raw>=SWMUX_ABC0_S  && raw <=SWMUX_ABC0_E) {
	return SWMUX_A_MASK|SWMUX_B_MASK|SWMUX_C_MASK;
    } else if (raw>=SWMUX_0B00_S  && raw <=SWMUX_0B00_E) {
	return SWMUX_B_MASK;
    } else if (raw>=SWMUX_0BC0_S  && raw <=SWMUX_0BC0_E) {
	return SWMUX_B_MASK|SWMUX_C_MASK;
    } else if (raw>=SWMUX_00C0_S  && raw <=SWMUX_00C0_E) {
	return SWMUX_C_MASK;
    } else if (raw>=SWMUX_XXXD_S  && raw <=SWMUX_XXXD_E) {
	return SWMUX_D_MASK;
    } else {
	return SWMUX_ERROR_MASK;
    }
}//endof swmux_val

#endif //CONF_DSENSOR_SWMUX
#endif //CONF_DSENSOR

