/*! \file   vis.c
    \brief  Implementation: visualization of BrickOS state
    \author Markus L. Noga <markus@noga.de>
*/

/*
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 *
 *  The Original Code is legOS code, released October 17, 1999.
 *
 *  The Initial Developer of the Original Code is Markus L. Noga.
 *  Portions created by Markus L. Noga are Copyright (C) 1999
 *  Markus L. Noga. All Rights Reserved.
 *
 *  Contributor(s): Markus L. Noga <markus@noga.de>
 */

#include <sys/vis.h>

#ifdef CONF_VIS

#include <conio.h>
#include <sys/irq.h>
#include <dsensor.h>
#include <dmotor.h>
#include <sys/program.h>
#include <sys/tm.h>
#include <sys/battery.h>
#include <sys/time.h>

///////////////////////////////////////////////////////////////////////////////
//
// Variables
//
///////////////////////////////////////////////////////////////////////////////

unsigned char vis_refresh_period  = 100; //!< VIS refresh period in ms

static void vis_refresh(void*);
timer_t vis_refresh_timer = {0, vis_refresh };

unsigned char vis_animated_man;
unsigned char vis_animated_man_ctr;

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

static void vis_refresh(void* dummy) {
#ifdef CONF_DSENSOR
    bit_iload(AD_C_H,0x7);
    dlcd_store(LCD_S1_ACTIVE);
    
    bit_iload(AD_B_H,0x7);
    dlcd_store(LCD_S2_ACTIVE);
    
    bit_iload(AD_A_H,0x7);
    dlcd_store(LCD_S3_ACTIVE);
#endif

#ifdef CONF_PROGRAM
    if(nb_tasks <= nb_system_tasks) {
		vis_animated_man = 0;
		lcd_show(man_stand);
    } else {
		if (vis_animated_man_ctr-- == 0) {
		    vis_animated_man ^= 1;
		    vis_animated_man_ctr = 4;
		    lcd_show(vis_animated_man == 0 ? man_stand : man_run);
		}
    }
#endif
    
#ifdef CONF_BATTERY_INDICATOR
    {
	if (BATTERY > BATTERY_NORMAL_THRESHOLD)
	    dlcd_hide(LCD_BATTERY_X);
	else if (BATTERY < BATTERY_LOW_THRESHOLD)
	    dlcd_show(LCD_BATTERY_X);
    }
#endif // CONF_BATTERY_INDICATOR
    add_timer(vis_refresh_period, &vis_refresh_timer);
}

void vis_init(void) {
    add_timer(vis_refresh_period, &vis_refresh_timer);
}
void vis_shutdown(void) {
    remove_timer(&vis_refresh_timer);
}

#endif  // CONF_VIS
