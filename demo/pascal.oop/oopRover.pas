program oopRover;

uses
  conio, unistd, Motor, MotorPair, ObjSound;

var
  M : TMotorPair;
  S : TSound;
  i : integer;
begin
  M.Create(mpA, mpC);

  for i := 0 to 9 do begin
    if (i mod 2) <> 0 then begin
      M.SetForward;
      S.Beep;
      cputs('FWD');
    end
    else begin
      M.Reverse;
      cputs('REV');
    end;
    M.Speed(m_max);
    sleep(5);
  end;
  M.TurnOff;
  cls;
end.
