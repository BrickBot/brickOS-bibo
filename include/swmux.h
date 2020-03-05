/*! \file   include/swmux.h
    \brief  Interface: direct control of a touch sensor multiplexor
    \author Mark Falco  (falcom@onebox.com)

  The TouchSensor Multiplexor is a device which allows three TouchSensors to
  be connected to a single input port. 

  This code applies to the touch sensor multiplexor (swmux) that can be ordered
  from http://www.techno-stuff.com, tweaking the ranges in swmux.c should make it
  work with other similar multiplexors
 */

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License
 *  at http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *  the License for the specific language governing rights and
 *  limitations under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Mark Falco  (falcom@onebox.com)
 */

#ifndef SWMUX_H
#define SWMUX_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>
#ifdef CONF_DSENSOR
#ifdef CONF_DSENSOR_SWMUX



/*bit masks for for checking the parsed value*/
#define SWMUX_A_MASK 1
#define SWMUX_B_MASK 2
#define SWMUX_C_MASK 4
#define SWMUX_D_MASK 8
#define SWMUX_ERROR_MASK 16 //value not recognized


/*Takes in raw sensor values and converts into a bitfield
  indicating the state of the attached touch sensors.
  If there is an error interepting the raw sensor value
  the error bit will be set, in which case the other bits
  should be ignored.  The bitfield is EDCBA, and can be examined
  by "anding" the field with the above masks.  The E in EDCBA is
  error in case you're wondering.
*/
unsigned char swmux_bits(unsigned int raw);

  //these macros will make do the anding for  you
#define SWMUX_A(bitfield) (bitfield & SWMUX_A_MASK)
#define SWMUX_B(bitfield) (bitfield & SWMUX_B_MASK)
#define SWMUX_C(bitfield) (bitfield & SWMUX_C_MASK)
#define SWMUX_D(bitfield) (bitfield & SWMUX_D_MASK)
#define SWMUX_ERROR(bitfield) (bitfield & SWMUX_ERROR_MASK)

#ifdef  __cplusplus
}//extern C
#endif

#endif //CONF_DSENSOR_SWMUX
#endif //CONF_DSENSOR

#endif //SWMUX_H
