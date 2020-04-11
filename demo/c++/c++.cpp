/*! \file   c++.cpp
    \brief  Demonstrates inheritance, virtual methods and new/delete.
    \author Markus L. Noga <markus@noga.de>

    A StateChanger interface class is introduced. Two subclasses implement
    different behaviours with virtual methods, adressing display and beeper,
    respectively.

    main() successively generates StateChanger objects. Their virtual
    methods are are invoked via interface wrappers onOff()
    and setState(s).
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

#include <config.h>

#include <conio.h>
#include <unistd.h>
#include <dsound.h>

//! An interface class.
class StateChanger {
public:
  virtual void setState(bool state) {
    if(state)
      enable();
    else
      disable();
  }

  virtual void enable()  = 0;
  virtual void disable() = 0;

  void onOff();
};

//! An implementation using the display.
class DisplayState : public StateChanger {
public:
  void enable() {
    cputc_native_user(0, CHAR_o, CHAR_n, 0);  // on
  }

  void disable() {
    cls();
  }
};

#ifdef CONF_DSOUND
//! An implementation using sound.
class SoundState : public StateChanger {
public:
  void enable() {
    dsound_system(DSOUND_BEEP);
  }

  void disable() {
  }
};
#endif // CONF_DSOUND

void StateChanger::onOff() {
  setState(1);
  sleep(1);
  setState(0);
  sleep(1);
}

int main(int argc, char **argv) {
  StateChanger *s;

  s=new DisplayState;
  s->onOff();
  delete s;

#ifdef CONF_DSOUND
  s=new SoundState;
  s->onOff();
  delete s;
#endif // CONF_DSOUND

  return 0;
}
