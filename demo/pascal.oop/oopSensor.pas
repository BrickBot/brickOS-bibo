program oopSensor;

uses
  conio, unistd, Sensor;

var
  S : TSensor;

begin
  S.Create(spS2, false);

  while true do begin
    cputw(S.Get);
    delay(10);
  end;

  S.Destroy;
end.
