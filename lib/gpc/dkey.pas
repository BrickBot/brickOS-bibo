unit dkey;

interface

uses
  types;

const
  KEY_ONOFF = $01;
  KEY_RUN   = $02;
  KEY_VIEW  = $04;
  KEY_PRGM  = $08;
  KEY_ANY   = $0f;

var
  dkey_multi : unsigned_char; asmname 'dkey_multi'; external;
  dkey : unsigned_char; asmname 'dkey'; external;

function dkey_pressed(data : wakeup_t) : wakeup_t; asmname 'dkey_pressed';
function dkey_released(data : wakeup_t) : wakeup_t; asmname 'dkey_released';
function getchar : int; asmname 'getchar';

implementation

end.