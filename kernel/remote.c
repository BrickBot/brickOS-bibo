/*! \file   remote.c
    \brief  Implementation: LEGO Infrared Remote Control and data structures
    \author Ross Crawford <rcrawford@csi.com>
*/

/*
 * Copyright (c) 2001 Ross Crawford
 *
 *  The contents of this file are subject to the Mozilla Public License
 *  Version 1.0 (the "License"); you may not use this file except in
 *  compliance with the License. You may obtain a copy of the License at
 *  http://www.mozilla.org/MPL/
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 *  License for the specific language governing rights and limitations
 *  under the License.
 */

/*
 *  2002.04.23 - Ted Hess <thess@kitschensync.net>
 *
 *  - Integrated into legOS 0.2.6. Added lr_startup(), lr_shutdown()
 *    Release input buffer while processing keys
 *
 */

#include <remote.h>

#if defined(CONF_LR_HANDLER)

#include <sys/handlers.h>
#include <sys/time.h>
#include <lnp.h>
#include <tm.h>

///////////////////////////////////////////////////////////////////////////////
//
// Internal Variables
//
///////////////////////////////////////////////////////////////////////////////

static void lr_timeout(void *dummy);

timer_t lr_timeout_timer = {0, lr_timeout};

unsigned int lr_curkeys;   // mask of keys currently "ON"
lr_handler_t lr_handler;   // the user event handler

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! Key state dispatcher (keyup / keydown)
static void lr_process(unsigned int lr_keys)
{
    unsigned int keys_diff, k;
    
    // If keys pressed has changed
    if (lr_keys != lr_curkeys) {
	
	if (lr_handler) {
	    // Get mask of keys pressed & released since last event
	    keys_diff = (lr_keys ^ lr_curkeys);
	    
	    // send event to user handler for each change
	    while (keys_diff) {
		k = keys_diff & -keys_diff;
		keys_diff &= ~k;

		if (lr_keys & k)
		    lr_handler(LREVT_KEYON,k);
		else
		    lr_handler(LREVT_KEYOFF,k);
	    }
	}
	
	// store key mask for next time
	lr_curkeys = lr_keys;
    }
}

//! Called from LNP when key data available
void lr_getdata(unsigned int x)
{
    // If previous data hasn't been processed yet, this will be lost
    lr_process(x);

    // Reset timeout
    remove_timer(&lr_timeout_timer);
    if (x)
	add_timer(LR_TIMEOUT, &lr_timeout_timer);
}


void lr_timeout(void * dummy) {
    lr_process(0);
}


//! Init remote key dispatcher
void lr_init()
{
    // start with all keys off, set initial timeout, clear user handler
    lr_curkeys = 0;
    lnp_remote_set_handler(lr_getdata);
    lr_handler = NULL;
}

//! Shutdown protocol handler and terminate thread?
void lr_shutdown()
{
    // Disconnect protocol handler
    lnp_remote_set_handler(LNP_DUMMY_REMOTE);
    lr_handler = NULL;
    return;
}

#endif  // CONF_LR_HANDLER
