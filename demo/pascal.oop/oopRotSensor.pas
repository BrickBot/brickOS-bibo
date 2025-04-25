program oopRotSensor;

uses
  conio, unistd, Sensor, RotationSensor;

var
  R : TRotationSensor;

begin
  R.Create(spS2);

  while true do begin
    cputw(R.GetPos);
    delay(10);
  end;

  R.Destroy;
end.
