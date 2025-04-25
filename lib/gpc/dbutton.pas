unit dbutton;

interface

uses
  types;

const
  BUTTON_ONOFF   = $0002;
  BUTTON_RUN     = $0004;
  BUTTON_VIEW    = $4000;
  BUTTON_PROGRAM = $8000;

function RELEASED(state, button : unsigned) : boolean;
function PRESSED(state, button : unsigned) : boolean;

function ButtonState : integer;
inline function dbutton : integer;

implementation

function RELEASED(state, button : unsigned) : boolean;
begin
  result := (state and button) <> 0;
end;

function PRESSED(state, button : unsigned) : boolean;
begin
  result := not RELEASED(state, button);
end;

function ButtonState : integer;
begin
  result := dbutton;
end;

inline function dbutton : integer;
begin
  asm ("mov.b @_PORT4, %0l
        mov.b @_PORT7, %0h"
       : "=r" (result) // output
       :               // input
       : "cc"          // clobbered
      );
end;

end.