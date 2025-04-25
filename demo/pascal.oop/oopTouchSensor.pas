program oopTouchSensor;

uses
  conio, unistd, Sensor, TouchSensor;

var
  T : TTouchSensor;

begin
  T.Create(spS2);

  while true do begin
    cputw(T.Get);
    delay(10);
  end;

  T.Destroy;
end.
