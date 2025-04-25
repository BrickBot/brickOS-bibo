program oopBatt;

uses
  conio, unistd, romlcd, ObjBattery;

var
  b : TBattery;
begin
  while true do begin
    cputs('batt');
    sleep(1);
    lcd_int(b.Get);
    sleep(1);
  end;
end.

