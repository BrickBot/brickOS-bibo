/*! \file   include/sys/program.h
    \brief  Internal Interface: program data structures and functions
    \author Markus L. Noga <markus@noga.de>
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

#ifndef __program_h__
#define __program_h__

#ifdef  __cplusplus
extern "C" {
#endif

#include <config.h>

#ifdef CONF_PROGRAM

#include <tm.h>

///////////////////////////////////////////////////////////////////////
//
// Definitions
//
///////////////////////////////////////////////////////////////////////


#ifndef DOXYGEN_SHOULD_SKIP_INTERNALS
/**
 * The program control structure
 * @internal
 */
typedef struct {
  void *text; 	      	//!< origin of text segment
  void *data; 	      	//!< origin of data segment (imm. after text)
  void *bss;  	      	//!< origin of bss segment (imm. after data)
  void *data_orig;    	//!< origin of backup copy of data segment

  size_t text_size;     //!< text segment size in bytes
  size_t data_size;     //!< data segment size in bytes
  size_t bss_size;      //!< bss  segment size in bytes
  size_t stack_size;  	//!< stack segment size in bytes
  size_t start; 	//!< offset from text segment to start into.
  priority_t prio;    	//!< priority to run this program at

  size_t downloaded;  	//!< number of bytes downloaded so far.
} program_t;

/**
 * @internal
 */
typedef enum {
  CMDacknowledge,     	//!< 1:
  CMDdelete, 	      	//!< 1+ 1: b[nr]
  CMDcreate, 	      	//!< 1+12: b[nr] s[textsize] s[datasize]
			//               s[bsssize]  s[stacksize]
			//               s[start]    b[prio]
  CMDoffsets, 	      	//!< 1+ 7: b[nr] s[text] s[data] s[bss]
  CMDdata,   	      	//!< 1+>3: b[nr] s[offset] array[data]
  CMDrun,     	      	//!< 1+ 1: b[nr]
  CMDirmode,		//!< 1+ 1: b[0=near/1=far]
  CMDsethost,			//!< 1+ 1: b[hostaddr]
  CMDlast     	      	//!< ?
} packet_cmd_t;

#endif /* DOXYGEN_SHOULD_SKIP_INTERNALS */

///////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////

//! check if a given program is valid.
/*! \return 0 if invalid */
extern int program_valid(unsigned nr);

//! display the current program number, or '-' if not a valid program
extern void show_program_num();

//! returns true if a program is currently running; false otherwise
extern inline char is_program_running();

//! sets whether any system keys (e.g. On-Off, Run) may be used by a user program
extern inline void allow_system_keys_access(char keys);

//! initialize program support
extern void program_init();

//! shutdown program support
extern void program_shutdown();

#endif // CONF_PROGRAM

#ifdef  __cplusplus
}
#endif

#endif // __program_h__
