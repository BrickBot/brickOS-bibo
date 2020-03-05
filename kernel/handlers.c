/*! \file handlers.c
    \brief  Implementation: Handlers
    \author Jochen Hoenicke <hoenicke AT gmail.com>
    
    Handlers are short functions running with highest priority just below
    interrupts.  They are enqueued to the handler_queue by the interrupts or
    user events (such as semaphores) when the handler needs to run.  As soon
    as the interrupt returns and the user code releases the kernel locks that
    it may hold, the handlers are invoked in sequential order.
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
 *                  Ben Laurie <ben@algroup.co.uk>
 *                  Lou Sortman <lou (at) sunsite (dot) unc (dot) edu>
 */

#include <sys/handlers.h>



/**
 * The linked list of active handlers.
 */
handler_t * volatile handler_queue;
handler_t * volatile * volatile handler_tail = &handler_queue;

/**
 * The global kernel lock
 */
volatile char kernel_lock;


