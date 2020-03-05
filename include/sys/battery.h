/*! \file   include/sys/battery.h
    \brief  Internal Interface: battery handling
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
 *		    Paolo Masetti  <paolo.masetti@itlug.org>
 */

#ifndef __sys_battery_h__
#define __sys_battery_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////

// Battery is normal if over 6700mV, low if under 6300mV
#define BATTERY_NORMAL_THRESHOLD \
    ds_unscale((unsigned) (6700 * 0x618L / 0xABD4))
#define BATTERY_LOW_THRESHOLD \
    ds_unscale((unsigned) (6300 * 0x618L / 0xABD4))

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////
/**
 * get current battery voltage
 * @return int voltage in millivolts
 */
extern int get_battery_mv();

/**
 * cause the battery voltage sensor value to be refreshed
 */
extern void battery_refresh(void)
#ifdef CONF_RCX_COMPILER
__attribute__ ((rcx_interrupt))
#endif
;

#ifdef  __cplusplus
}
#endif

#endif // __sys_battery_h__
