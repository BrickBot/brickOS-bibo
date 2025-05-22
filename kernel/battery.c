/*! \file   battery.c
    \brief  Inplementation: battery functions and low battery LCD indicator handling
    \author Paolo Masetti <paolo.masetti@itlug.org>
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
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 *        Paolo Masetti  <paolo.masetti@itlug.org>
 */

#include <sys/irq.h>
#include <dsensor.h>
#include <dlcd.h>

#include <sys/battery.h>

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! return the approximate battery voltage (in milliVolts)
#if defined(CONF_DSENSOR)
int get_battery_mv()
{
    // From librcx documentation:
    //   "Units are strange, multiply by 43988 then divide by 1560 to get mV."
    long b = ds_scale(BATTERY) * 0xABD4L;
    return (int)(b / 0x618L);
}
#endif // CONF_DSENSOR
