program oopTempSensor;

uses
  conio, Sensor, TemperatureSensor;

(*
  TemperatureSensor t(Sensor::S2);

  while (1) {
    const int a(t.tenths());
    cputw(a);
    delay(100);
  }
*)

var
  t : TTemperatureSensor;
  a, i : integer;
begin
  t.Create(spS2);
  for i := 0 to 10 do
  begin
    a := t.Tenths;
    cputw(a);
    delay(100);
    cputw(1);
    delay(200);
  end;
  t.Destroy;
end.

//h8300-hms-gpc -O2 -fno-builtin -fomit-frame-pointer -Wall -I/brickos-0.2.6.07.nmChg/include -I/brickos-0.2.6.07.nmChg/include/lnp -I. -I../boot/  --extended-syntax --automake --unit-path=/brickos-0.2.6.07.nmChg/lib/p -c tempsens.pas -S
