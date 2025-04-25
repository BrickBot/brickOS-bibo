unit ObjBattery;

interface

uses
  types, battery;

type
  TBattery = object
    function Get : int;
  end;

implementation

function TBattery.Get : int;
begin
  result := get_battery_mv;
end;

end.