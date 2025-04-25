program oopSound;

uses
  conio, unistd, ObjSound;

var
  S : TSound;

begin
  while true do
  begin
    S.Beep;
    cputs('Beep');
    sleep(1);
    cls;
  end;

  cls;
end.
