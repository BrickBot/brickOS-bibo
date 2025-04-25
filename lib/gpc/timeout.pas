unit timeout;

interface

uses
  types;

const
  DEFAULT_SHUTOFF_TIME = 15*60;

var
  auto_shutoff_counter : unsigned_int; asmname 'auto_shutoff_counter'; external;
  auto_shutoff_period : unsigned_int; asmname 'auto_shutoff_period'; external;
  auto_shutoff_secs : unsigned_int; asmname 'auto_shutoff_secs'; external;
  auto_shutoff_elapsed : unsigned_int; asmname 'auto_shutoff_elapsed'; external;
  idle_powerdown : unsigned_int; asmname 'idle_powerdown'; external;

procedure shutoff_init; asmname 'shutoff_init';
procedure shutoff_restart; asmname 'shutoff_restart';
procedure autoshutoff_check; asmname 'autoshutoff_check';

implementation

end.