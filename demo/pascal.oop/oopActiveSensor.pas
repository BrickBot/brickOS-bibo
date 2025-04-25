program oopActiveSensor;

uses
  conio, Sensor;

var
  s : TSensor;
  i : integer;
begin
  s.Create(spS2, true);
  for i := 0 to 10 do
  begin
    cputw(s.Get);
    delay(10);
  end;
  cls;
  s.Destroy;
end.
