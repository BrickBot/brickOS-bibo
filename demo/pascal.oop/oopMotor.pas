program oopMotor;

uses
  conio, unistd, Motor, ObjSound;

var
  m : TMotor;
  s : TSound;
begin
  m.Create(mpA);
  m.SetForward;
  cputs('Fast');
  m.Speed(m_max);
  sleep(1);
  cputs('Med');
  m.Speed((m_max + m_min) div 2);
  m.Reverse;
  s.Beep;
  sleep(1);
  cputs('Slow');
  m.Speed(m_min);
  sleep(1);
  cls;
  m.Destroy;
end.
