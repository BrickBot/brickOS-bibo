program oopLightSensor;

uses
  conio, unistd, Sensor, LightSensor;

var
  l : TLightSensor;
begin
  l.Create(spS2);

  while true do begin
    cputw(l.Get);
    delay(10);
  end;
end.

