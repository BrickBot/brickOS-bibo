///////////////////////////////////////////////////////////////////////////////
// onscreen.c
//
// "On Screen" programmer for RCX
// version 0.01 Copyright © 1999 by David R. Van Wagner ALL RIGHTS RESERVED
//
// Adapted April 3, 2020, for brickOS-bibo by Matthew Sheets
//
// allows simple steps to be defined directly on the RCX for test purposes
///////////////////////////////////////////////////////////////////////////////

#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dbutton.h>
#include <dlcd.h>
#include <dmotor.h>
#include <sys/tm.h>
#include <sys/time.h>

//#include <malloc.h>
//#include <stdio.h>

//#define DEBUG(s) //cputs(s),lcd_refresh(),delay(100)

#include "abstractcmd.h"
#include "macro.h"
#include "rcx.h"
#include "cursor.h"
#include "rcxstate.h"

RCX MyRCX;

wakeup_t button_press_wakeup(wakeup_t data) {
  return PRESSED(dbutton(),data);
}

wakeup_t button_release_wakeup(wakeup_t data) {
  return RELEASED(dbutton(),data);
}

wakeup_t shutdown_requested_wakeup(wakeup_t data) {
  // wait_event processing includes a built-in "shutdown requested" check,
  //   so no additional checking is necessary.
  return NULL;
}

//int taskButtonRun()
//{
//  do
//  {
//    wait_event(&button_release_wakeup,BUTTON_RUN);  // wait for release
//    msleep(200); // debounce
//    wait_event(&button_press_wakeup,BUTTON_RUN);    // wait for press
//    msleep(200); // debounce

//    MyRCX.myState = MyRCX.myState->run(&MyRCX);
//  } while (!shutdown_requested());
  
//  return 0;
//}

int taskButtonView()
{
  do
  {
    wait_event(&button_press_wakeup,BUTTON_VIEW);    // wait for press
//    msleep(200); // debounce
    wait_event(&button_release_wakeup,BUTTON_VIEW);  // wait for release
//    msleep(200); // debounce

    if (!shutdown_requested()) {
      MyRCX.myState = MyRCX.myState->view(&MyRCX);
    }
  } while (!shutdown_requested());
  
  return 0;
}

int taskButtonPrgm()
{
  do
  {
    wait_event(&button_press_wakeup,BUTTON_PROGRAM);    // wait for press
//    msleep(200); // debounce
    wait_event(&button_release_wakeup,BUTTON_PROGRAM);  // wait for release
//    msleep(200); // debounce

    if (!shutdown_requested()) {
      MyRCX.myState = MyRCX.myState->prgm(&MyRCX);
    }
  } while (!shutdown_requested());
  
  return 0;
}

int taskShutdownRequested()
{
  wait_event(&shutdown_requested_wakeup, (wakeup_t)NULL);  // wait for release
  MyRCX.myState = MyRCX.myState->onOff(&MyRCX);
  
  return 0;
}

//int taskButtonOnOff()
//{
//  do
//  {
//    wait_event(&button_press_wakeup,BUTTON_ONOFF);    // wait for press
//    msleep(200); // debounce
//    wait_event(&button_release_wakeup,BUTTON_ONOFF);  // wait for release
//    msleep(200); // debounce

//    MyRCX.myState = MyRCX.myState->onOff(&MyRCX);
//  } while (!shutdown_requested());
  
//  return 0;
//}


unsigned int timer_interval = 167;
  
static void taskTimer(void* dummy)
{
//  do
//  {
//    msleep(167);
//    MyRCX.myState = MyRCX.myState->tick(&MyRCX);
//  } while (1);
  if (!shutdown_requested()) {
    MyRCX.myState = MyRCX.myState->tick(&MyRCX);
    add_timer(timer_interval, &tTaskTimer);
  }
}

int taskRun()
{
  RCXexecutor(&MyRCX);
  tRun = NULL;
  return 0;
}

#define MOTOR_A 1
#define MOTOR_B 2
#define MOTOR_C 4
#define MOTOR_AB (MOTOR_A+MOTOR_B)
#define MOTOR_AC (MOTOR_A+MOTOR_C)
#define MOTOR_BC (MOTOR_B+MOTOR_C)
#define MOTOR_ABC (MOTOR_A+MOTOR_B+MOTOR_C)

#define FWD(m,p) OUTPUT(m,p,m)
#define BACK(m,p) OUTPUT(m,p,m^7)
#define BRAKE(m) OUTPUT(m,-1,m)

void DELAY(int d)
{
  DelayCommand* cmd = (DelayCommand*)malloc(sizeof(DelayCommand));
  DelayCommand_constructor(cmd, d);
  MyRCX.M_currentMacro->add(MyRCX.M_currentMacro, (AbstractCommand*)cmd);
}

void OUTPUT(int iMotors, int iPower, int iFwds)
{
  OutputCommand* cmd = (OutputCommand*)malloc(sizeof(OutputCommand));
  OutputCommand_constructor(cmd, iMotors, iPower, iFwds);
  MyRCX.M_currentMacro->add(MyRCX.M_currentMacro, (AbstractCommand*)cmd);
}

void GOTO(int iGoto)
{
  GotoCommand* cmd = (GotoCommand*)malloc(sizeof(GotoCommand));
  GotoCommand_constructor(cmd, iGoto);
  MyRCX.M_currentMacro->add(MyRCX.M_currentMacro, (AbstractCommand*)cmd);
}

int main()
{
  RCXStateConstructors();

  RCX_constructor(&MyRCX);

  DELAY(20);
  FWD(MOTOR_ABC,8);
  DELAY(20);
  BRAKE(MOTOR_ABC);
  DELAY(20);
  BACK(MOTOR_ABC,8);
  DELAY(20);
  BRAKE(MOTOR_ABC);
  DELAY(20);
  GOTO(0);
  DELAY(0);
  DELAY(0);
  DELAY(0);
  DELAY(0);
  DELAY(0);
  DELAY(0);

//  lcd_clear();

  MyRCX.myState = &RCXBrowseState;
  // cputs("READY");
  cputc_native_user(CHAR_S, CHAR_t, CHAR_r, CHAR_t);  // Strt  (Start)

  MyRCX.run(&MyRCX);
  RCXCursor.set(-1);

  // An old name for tm_init()???  If so, perhaps not needed since running within brickOS ???
  // tm_start();

  return 0;
}

