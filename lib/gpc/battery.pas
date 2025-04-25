unit battery;

interface

uses
  types;

const
  BATTERY_NORMAL_THRESHOLD_MV = 6700;
  BATTERY_LOW_THRESHOLD_MV    = 6300;

function get_battery_mv : int; asmname 'get_battery_mv';
procedure battery_refresh; asmname 'battery_refresh';

implementation

end.
