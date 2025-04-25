program oopTouchPressedSensor;

uses
  conio, unistd, Sensor, TouchSensor;

var
  t : TTouchSensor;
begin
  t.Create(spS2);

  while true do begin
    if t.Pressed then cputw(1)
                 else cputw(0);
    delay(10);
  end;
end.

