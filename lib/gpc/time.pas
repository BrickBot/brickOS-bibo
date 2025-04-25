unit time;

interface

{$I config.inc}

{$IFDEF CONF_TIME}
uses
  types;

const
  TICK_IN_MS    = 1;
  TICKS_PER_SEC = 1000;

function SECS_TO_TICKS(s : unsigned_long) : unsigned_long;
function MSECS_TO_TICKS(ms : unsigned_long) : unsigned_long;

function get_system_up_time : time_t; asmname 'get_system_up_time'; external;
//var
//  sys_time : time_t; asmname 'sys_time'; external;

{$IFDEF CONF_TM}
const
  TM_DEFAULT_SLICE = 20;

var
  systime_tm_return : Pointer; asmname 'systime_tm_return'; external;
{$ENDIF}

procedure systime_init; asmname 'systime_init';
procedure systime_shutdown; asmname 'systime_shutdown';

{$IFDEF CONF_TM}
procedure systime_set_switcher(switcher : pointer); asmname 'systime_set_switcher';
procedure systime_set_timeslice(slice : unsigned_char); asmname 'systime_set_timeslice';
{$ENDIF}

{$ENDIF}

implementation

{$IFDEF CONF_TIME}

function SECS_TO_TICKS(s : unsigned_long) : unsigned_long;
begin
  result := s * TICKS_PER_SEC;
end;

function MSECS_TO_TICKS(ms : unsigned_long) : unsigned_long;
begin
  result := ms div TICK_IN_MS;
end;

{$ENDIF}
end.