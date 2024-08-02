/*! \file program.c
    \brief  Implementation: program data structures and functions
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
 *                  Frank Cremer <frank@demon.holly.nl>
 */

/*
 *  2000.05.01 - Paolo Masetti <paolo.masetti@itlug.org>
 *
 *  - Added "CMDirmode" for changing via LNP IR mode
 *
 *  2001.05.10 - Matt Ahrens <mahrens@acm.org>
 *
 *    - Added free memory and batter life display
 *      Press "view" repeatedly while no programs are running to see
 *
 *  2002.4.23 - Ted Hess <thess@kitschensync.net>
 *
 *  - Added Remote key handler
 */

#include <sys/program.h>

#ifdef CONF_PROGRAM

#include <sys/tm.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/lnp.h>
#include <sys/lnp-logical.h>
#include <sys/dmotor.h>
#include <sys/dsensor.h>
#include <sys/dkey.h>
#include <sys/mm.h>
#include <sys/battery.h>
#include <dsound.h>
#include <remote.h>

#include <conio.h>

#define SYNTHKEY_PACKET 0x10
#define SYNTHKEY_STOP   0x11
#define SYNTHKEY_START  0x12
#define SYNTHKEY_RESET_SHUTOFF_TIMER 0x13

///////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
///////////////////////////////////////////////////////////////////////////////

static signed cprog = -1;                //!< the current program
static unsigned char synthetic_key;
static waitqueue_t *synthetic_waitqueue;

///////////////////////////////////////////////////////////////////////////////
//
// Internal Variables
//
///////////////////////////////////////////////////////////////////////////////

const unsigned char min_length[]={
   1, // CMDacknowledge
   2, // CMDdelete
  13, // CMDcreate
   8, // CMDoffsets
   4, // CMDdata
   2, // CMDrun
   2, // CMDirmode
   2  // CMDsethost
};

static program_t programs[PROG_MAX];       //!< the programs

static unsigned char* buffer_ptr;          //!< packet receive buffer
volatile unsigned char packet_len;         //!< packet length
volatile unsigned char packet_src;         //!< packet sender

static unsigned char system_reserved_keys = KEYS_SYSTEM; //!< allow user programs to use the On-Off button

#if 0
#define debugs(a) { cputs(a); msleep(500); }
#define debugw(a) { cputw(a); msleep(500); }
#elif 0
#define debugs(s) {debug_printf("LNP:%s",s);}
#define debugw(x) {debug_printf("LNP:%04x",x);}
#else
#define debugs(a)
#define debugw(a)
#endif

// Forward ref
int lrkey_handler(unsigned int etype, unsigned int key);

///////////////////////////////////////////////////////////////////////////////
//
// Functions
//
///////////////////////////////////////////////////////////////////////////////

//! packet handler, called from interrupt
/*! allocates buffer, copies data and wakes parser task.
*/
static void packet_producer(const unsigned char *data,
			    unsigned char length, unsigned char src) {
    // old packet still unhandled or empty packet?
    //
    if(packet_len>0 || length==0)
	return;
    
    if (buffer_ptr != 0)
	return;
    
    buffer_ptr = malloc(length);
    memcpy(buffer_ptr,data,length);
    packet_len=length;
    packet_src=src;
    synthetic_key = SYNTHKEY_PACKET;
    wakeup(synthetic_waitqueue);
}


//! check if a given program is valid.
/*! \return 0 if invalid */
int program_valid(unsigned nr) {
  program_t *prog=programs+nr;

  return (nr < PROG_MAX) &&
         (prog->text_size>0) &&
         (prog->text_size+prog->data_size==prog->downloaded);
}

//! display the current program number, or '-' if not a valid program
void show_program_num() {
  if (program_valid(cprog)) {
    cputc_hex_0(cprog+1);
  } else {
    cputc_native_0(CHAR_DASH);
  }
}

int find_next_program() {
    int i;
    int next_prog = -1;
    for (i = cprog + 1; i < PROG_MAX; i++) {
	if (program_valid(i)) {
	    next_prog = i;
        break;
	}
    }
    // handle wrap-around of program number
    if (-1 == next_prog) {
        for (i = 0; i <= cprog && i < PROG_MAX; i++) {
            if (program_valid(i)) {
                next_prog = i;
                break;
            }
        }
    }
    return next_prog;
}

//! reutrns true if a program is running; false otherwise
char is_program_running() {
  return nb_tasks > nb_system_tasks;
}

void allow_system_keys_access(char keys) {
  system_reserved_keys = (KEYS_SYSTEM ^ keys);
}


//! stop program
static void program_stop(void) {
    unsigned char count_down = 40;
    
    // Kindly request that all user tasks shutdown
    shutdown_tasks(T_USER);
    // Wait a bit
    while (--count_down && (nb_tasks > nb_system_tasks)) {
	cputc_native_user(CHAR_S, CHAR_T, CHAR_O, CHAR_P);  // STOP
	msleep(100);
    }
    
    if (nb_tasks > nb_system_tasks) {
	// Wait no longer.
	killall(T_USER);
    }
}

//! run the given program
static void program_run(unsigned nr) {
    unsigned char dkey_old;
    program_t *prog=programs+nr;
    waitqueue_t dkey_entry, synth_entry;
    
    // initialize data segments
    //
    memcpy(prog->data,prog->data_orig,prog->data_size);
    memset(prog->bss,0,prog->bss_size);

    // ensure state is properly initialized for program execution
    //
    lnp_addressing_set_handler(0, LNP_DUMMY_ADDRESSING);

    // execute the program
    execi((void*) (((char*)prog->text) + prog->start  ),
	  0,0,prog->prio,prog->stack_size);

    // Wait for program to terminate
    grab_kernel_lock();
    dkey_old = dkey;
    add_to_waitqueue(&dkey_program_waitqueue, &dkey_entry);
    add_to_waitqueue(&synthetic_waitqueue, &synth_entry);
    while (!shutdown_requested() && nb_tasks > nb_system_tasks) {
      dkey_old = dkey;
      // poll every 200 ms if program terminated
      wait_timeout(200);
      if (synthetic_key) {
        if (synthetic_key == SYNTHKEY_STOP)
          break;
        synthetic_key = 0;
      }
      if ((dkey & ~dkey_old & (system_reserved_keys)) != 0)
          break;
    }
    remove_from_waitqueue(&dkey_entry);
    remove_from_waitqueue(&synth_entry);
    if ((dkey & KEY_ONOFF) != 0) {
      // if ONOFF pressed, enqueue synthetic OFF key
      synthetic_key = KEY_ONOFF;
    }
    release_kernel_lock();
    program_stop();
    
    // Free blocks allocated by user processes.
    mm_reaper();
    
    // Reset motors, sensors, sound & LNP as
    // programs may have motors running,
    // sensors active or handlers set.
    //
    // Programs that exit on their own
    // are assumed to take the necessary
    // actions themselves.
    //
#ifdef CONF_DSOUND
    dsound_stop();
#endif
#ifdef CONF_DMOTOR
    dm_init();
#endif
#ifdef CONF_DSENSOR
    ds_init();
#endif
    lnp_init();
#ifdef CONF_LR_HANDLER
    // Reset remote button handler, in case the program cleared it.
    lr_init();
    lr_set_handler(lrkey_handler);
#endif
    lnp_addressing_set_handler(0, &packet_producer);
    allow_system_keys_access(KEY_NONE);
}

//! packet command parser task
__TEXT_HI__ static void answer_packet() {
    packet_cmd_t cmd;
    unsigned char nr=0;
    program_t *prog=programs; // to avoid a silly warning
    const static unsigned char acknowledge=CMDacknowledge;
    unsigned char msg[8];

    debugw(*(size_t*)buffer_ptr);
    
    // handle trivial errors
    //
    cmd=buffer_ptr[0];
    if (cmd>=CMDlast || packet_len<min_length[cmd])
	return;
    
    // Get program number, validate value
    if((cmd > CMDacknowledge) && (cmd <= CMDrun)) {
	nr = buffer_ptr[1];
	if(nr > PROG_MAX)
	    return;
	debugw(nr);
	prog = programs+nr;
    }
	
	
    switch( cmd ) {
	// handle IR CMDs
    case CMDirmode:
	if (buffer_ptr[1]==0) {
	    debugs("nearmodeIR");
	    lnp_logical_range(0);
	    debugs("OK");
	    lnp_addressing_write(&acknowledge,1,packet_src,0);
	} else {
	    debugs("farmodeIR");
	    lnp_logical_range(1);
	    debugs("OK");
	    lnp_addressing_write(&acknowledge,1,packet_src,0);
	}
	break;
	       
    case CMDsethost:
	// ACK before we change our address
	lnp_addressing_write(&acknowledge,1,packet_src,0);
	lnp_set_hostaddr(buffer_ptr[1]);
	break;
  
    case CMDdelete:
	debugs("dele");
		
	if(nb_tasks == nb_system_tasks) {
	    if(prog->text)
		free(prog->text);
	    memset(prog,0,sizeof(program_t));
		    
	    if(nr == cprog)
            cputc_native_0(CHAR_DASH);
	    debugs("OK");
		    
	    lnp_addressing_write(&acknowledge,1,packet_src,0);
	}
	break;
		
    case CMDcreate:
	debugs("crea");
	if(!prog->text) {
	    memcpy(&(prog->text_size),buffer_ptr+2,11);
		    
	    if((prog->text=malloc(prog->text_size+
				  2*prog->data_size+
				  prog->bss_size  ))) {
		prog->data=prog->text+prog->text_size;
		prog->bss=prog->data+prog->data_size;
		prog->data_orig=prog->bss +prog->bss_size;
		prog->downloaded=0;
			
		debugs("OK");
			
		cputw(prog->text_size + prog->data_size);
		cprog = nr;
			
		msg[0]=CMDacknowledge;
		msg[1]=nr;
		memcpy(msg+2,prog,6);
		lnp_addressing_write(msg,8,packet_src,0);
	    } else
		memset(prog,0,sizeof(program_t));
	}
	break;
		
    case CMDdata:
	debugs("data");
	if(prog->text) {
	    size_t offset=*(size_t*)(buffer_ptr+2);
	    if(offset<=prog->downloaded) {
		if(offset==prog->downloaded) {
		    memcpy(prog->text+offset,buffer_ptr+4,packet_len-4);
		    prog->downloaded+=packet_len-4;
			    
		    if(program_valid(nr)) {
			// copy original data segment and we're done.
			//
			memcpy(prog->data_orig,prog->data,prog->data_size);
			cls();
		    } else
			cputw(prog->text_size + prog->data_size
			      - prog->downloaded);
		    debugs("OK");
		} else
		    debugs("OLD");
	    }
	    lnp_addressing_write(&acknowledge,1,packet_src,0);
	}
	break;
		
    case CMDrun:
	debugs("run");
	if(program_valid(nr)) {
	    cprog = nr;
	    synthetic_key = SYNTHKEY_START;
		    
	    debugs("OK");
	    lnp_addressing_write(&acknowledge,1,packet_src,0);
	}
	break;
		
    default:
	debugs("error");
    }
    packet_len=0;
    free(buffer_ptr);
    buffer_ptr = 0;
}


static int program_event(void) {
    unsigned char dkey_old, result;
    waitqueue_t dkey_entry, synth_entry;
    unsigned int start_timer = (unsigned int) (get_system_up_time() >> 10L);

    grab_kernel_lock();

    if (synthetic_key) {
      result = synthetic_key;
      synthetic_key = 0;
      return result;
    }

    add_to_waitqueue(&dkey_system_waitqueue, &dkey_entry);
    add_to_waitqueue(&synthetic_waitqueue, &synth_entry);
    do {
	dkey_old = dkey;
	wait_timeout(1024);
    
	result = dkey & ~dkey_old;

	if (synthetic_key) {
	  result = synthetic_key;
	  synthetic_key = 0;
	}
	if (nb_tasks > nb_system_tasks)
	    start_timer = (unsigned int) (get_system_up_time() >> 10L);
	else if (((unsigned int)(get_system_up_time() >> 10L) - start_timer)
	    >= DEFAULT_SHUTOFF_TIME && !result)
	  result = KEY_ONOFF;
    } while (!result && !shutdown_requested());
    remove_from_waitqueue(&dkey_entry);
    remove_from_waitqueue(&synth_entry);
    release_kernel_lock();
    return result;
}


//! handle key input (on/off, run, program)
int program_main(int argc, void *argv[]) {
    int c;

    show_program_num();
    while (!shutdown_requested()) {
	int clear=0;
	c=program_event();
	
gotkey:
	debugs("key "); debugw(c);
	debugs("task"); debugw(nb_tasks);
	
	switch(c) {
	case SYNTHKEY_PACKET:
	    answer_packet();
	    break;

	case KEY_RUN:
	case SYNTHKEY_START:
	    if (program_valid(cprog)) {
		program_run(cprog);
	    } else {
            // show that the current program is invalid
            cputc_native_user(CHAR_N, CHAR_O, CHAR_N, CHAR_E);  // NONE
        }
		clear = 1;
	    break;

	case KEY_ONOFF:
	    // Kindly request all tasks shutdown
	    shutdown_tasks(T_USER | T_KERNEL);
	    // Except for ourself :)
	    ctid->tflags &= (~T_SHUTDOWN);
	    // Wait a bit until only idle task and we are running
	    clear = 40;
	    while (--clear && (nb_tasks > 2)) {
        cputc_native_user(CHAR_O, CHAR_F, CHAR_F, 0);  // OFF
		msleep(100);
	    }
	    clear = 0;
	    // Wait no longer.
	    if (nb_tasks > 2)
		killall(T_USER | T_KERNEL);
	    // Now we shutdown ourself.
	    ctid->tflags |= T_SHUTDOWN;
	    break;
	    
	case KEY_PRGM:
        cprog = find_next_program();
        show_program_num();
        if (!program_valid(cprog)) {
            cputc_native_user(CHAR_N, CHAR_O, CHAR_N, CHAR_E);  // NONE
            clear = 1;
        }
	    break;

#ifdef CONF_VIEW_BUTTON
	case KEY_VIEW:
	    /*
	     * pressing the "view" button cycles through a display of the
	     * amount of the amount of free memory (in decimal and
	     * hexadecimal) and battery power. If a button other than "view"
	     * is pressed while cycling through, we handle that button
	     * ("goto gotkey").
	     */
        cputc_native_user(CHAR_A, CHAR_d, CHAR_r, 0); // Adr
	    cputc_hex(lnp_hostaddr >> 4, 1);
	    while ((c = program_event()) == KEY_PRGM) {
		lnp_hostaddr += 0x10;
		cputc_hex(lnp_hostaddr >> 4, 1);
	    }
	    if (c != KEY_VIEW) goto gotkey;

	    for(;;) {
		lcd_number(((mm_free_mem()>>7)*100)>>3, unsign, e_2);
		cputc_native_1(CHAR_F);
		c = program_event();
		    
		if (c == KEY_VIEW)
		    break;

		if (c != KEY_PRGM) 
		    goto gotkey;

		if (program_valid(cprog)) {
            cputc_native_user(CHAR_d, CHAR_e, CHAR_l, CHAR_QUESTION); // del?
		    c = program_event();
		    if (c == KEY_PRGM) {
			grab_kernel_lock();
			program_t *prog = programs+cprog;
			if (prog->text)
			    free(prog->text);
			memset(prog,0,sizeof(program_t));
			cprog = find_next_program();
            show_program_num();
			release_kernel_lock();
		    } else if (c != KEY_VIEW);
			goto gotkey;
		}
	    }

#if defined(CONF_DSENSOR)
	    lcd_number(get_battery_mv(), unsign, e_3);
	    if ((c = program_event()) != KEY_VIEW) goto gotkey;
#endif // CONF_DSENSOR
	    
	    clear=1;
	    break;
#endif // CONF_VIEW_BUTTON
    // Reset the shutoff timer
    //case SYNTHKEY_RESET_SHUTOFF_TIMER:
        // Nothing to do
        //break;
	}
	
	if(clear) {
	    dkey_wait(KEY_RELEASED, KEY_ANY);
	    cls();
	}
    }
    return 0;
}

#if defined(CONF_LR_HANDLER)
//! handle remote key input (P1-P5, A1-C1, A2-C2, stop, beep)
__TEXT_HI__ int lrkey_handler(unsigned int etype, unsigned int key) {
  unsigned char pnr = 0;

  // If a program is running, stop it
  //  NOTE: this LRKEY is allowed while a program is running!
  if (nb_tasks > nb_system_tasks) {
    if(key == LRKEY_STOP && etype == LREVT_KEYON) {
      synthetic_key = SYNTHKEY_STOP;
      wakeup(synthetic_waitqueue);
      return 1; // we consumed key
    }
  } else {
    // Keydown events dispatched here
    if (etype == LREVT_KEYON) {
      switch (key) {
#ifdef CONF_DSOUND
        case LRKEY_BEEP:
          // Need high pitched beep-beep
          dsound_system(0);
          break;
#endif // CONF_DSOUND
        
        case LRKEY_P5:
          pnr++;
          // ... Fallthru
        case LRKEY_P4:
          pnr++;
          // ... Fallthru
        case LRKEY_P3:
          pnr++;
          // ... Fallthru
        case LRKEY_P2:
          pnr++;
          // ... Fallthru
        case LRKEY_P1:
          // Start something?
          cprog = pnr;
          show_program_num();
          if(program_valid(pnr)) 
          {
            // launch Program(n)
	    synthetic_key = SYNTHKEY_START;
	    wakeup(synthetic_waitqueue);
          } else {
            cputc_native_user(CHAR_N, CHAR_O, CHAR_N, CHAR_E);  // NONE
          }
          break;

#if defined(CONF_DMOTOR)        
        // Motor on commands
        case LRKEY_A1:
          // A Motor fwd
          motor_a_dir(fwd);
          break;
        case LRKEY_A2:
          // A Motor rev
          motor_a_dir(rev);
          break;
        case LRKEY_B1:
          // B Motor fwd
          motor_b_dir(fwd);
          break;
        case LRKEY_B2:
          // B Motor rev
          motor_b_dir(rev);
          break;
        case LRKEY_C1:
          // C Motor fwd
          motor_c_dir(fwd);
          break;
        case LRKEY_C2:
          // C Motor rev
          motor_c_dir(rev);
          break;
#endif // CONF_DMOTOR
        default:
          // Not consumed
          return 0;
      }
#ifndef CONF_LCD_REFRESH
      lcd_refresh();
#endif
      // Reset the poweroff timer
      synthetic_key = SYNTHKEY_RESET_SHUTOFF_TIMER;
      wakeup(synthetic_waitqueue);
      // Key consumed
      return 1;
    }
  
    // Keyup events dispatched here
    if (etype == LREVT_KEYOFF) {
      switch (key) {
#if defined(CONF_DMOTOR)
        case LRKEY_A1:
        case LRKEY_A2:
          // Shut off A motor
          motor_a_dir(brake);
          break;
        case LRKEY_B1:
        case LRKEY_B2:
          // Shut off B motor
          motor_b_dir(brake);
          break;
        case LRKEY_C1:
        case LRKEY_C2:
          // Shut off C motor
          motor_c_dir(brake);
          break;
#endif // CONF_DMOTOR
        default:
          return 0;
      }
      // Reset the poweroff timer
      synthetic_key = SYNTHKEY_RESET_SHUTOFF_TIMER;
      wakeup(synthetic_waitqueue);
      // Used the key
      return 1;
    }
  }

#ifndef CONF_LCD_REFRESH
  lcd_refresh();
#endif
  // Didn't eat the key
  return 0;
}
#endif

//! initialize program support
/*! run in single tasking mode
*/
__TEXT_HI__ void program_init() {
    kexeci(&program_main, 0, 0, PRIO_HIGHEST, 96, T_KERNEL);
    lnp_addressing_set_handler(0, &packet_producer);

#ifdef CONF_LR_HANDLER
    // Setup kernel remote callback handler and dispatch thread
    lr_init();
    lr_set_handler(lrkey_handler);
#endif
}

//! shutdown program support
/*! run in single tasking mode
*/
__TEXT_HI__ void program_shutdown() {
    lnp_addressing_set_handler(0, LNP_DUMMY_ADDRESSING);
    if (buffer_ptr) {
	free(buffer_ptr);
	buffer_ptr = 0;
    }

#ifdef CONF_LR_HANDLER
    lr_shutdown();
#endif
}

#endif // CONF_PROGRAM
