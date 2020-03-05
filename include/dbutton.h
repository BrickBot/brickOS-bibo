/*! \file   include/dbutton.h
    \brief  Interface: query raw button states
    \author Markus L. Noga <markus@noga.de>

    \warning this is raw, unprocessed input. buttons will bounce.
    please use dkey functions instead.
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
 */

#ifndef __dbutton_h__
#define __dbutton_h__

#ifdef  __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////////////

#define	BUTTON_ONOFF	0x0002	//!< the on/off button
#define BUTTON_RUN	0x0004	//!< the run button
#define BUTTON_VIEW	0x4000	//!< the view button
#define BUTTON_PROGRAM	0x8000	//!< the program button

// buttons are active low
//

//! true if any of the specified buttons is released
//
#define RELEASED(state,button)	((state) & (button))

//! true if all of the specified buttons are pressed
//
#define PRESSED(state,button)	(!RELEASED(state,button))

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! get button states
/*! masks as defined above
 */
extern inline int dbutton(void)
{
  int rv;
  __asm__ __volatile__("\n\
      	mov.b @_PORT4,%0l\n\
	mov.b @_PORT7,%0h\n\
"	      :"=r"(rv)	// output
	      :	      	// input
	      :"cc"	// clobbered
  );

  return rv;
}

#ifdef  __cplusplus
}
#endif

#endif // __dbutton_h__
