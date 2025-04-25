program oopRaceTrack;

uses
  conio, unistd, Motor, MotorPair, Sensor, LightSensor;

var
  m : TMotorPair;
  l : TLightSensor;
  speed : integer;
  value, lightLevel, threshold : word;
begin

  m.Create(mpA, mpC);
  l.Create(spS2);

  speed := (m_max + m_min) div 3;
  m.TurnOff;

  lightLevel := l.SampleDefault;
  threshold  := lightLevel + 5;

  cputw(lightLevel);
  sleep(2);

  while true do begin
    m.ForwardSpeed(speed);
    while true do begin
      value := l.Get;
      cputw(value);
      if value > threshold then
        break;
    end;
    m.BrakeDuration(100);
    m.LeftSpeed(m_max);

    while true do begin
      value := l.Get;
      cputw(value);
      if value < threshold then
        break;
    end;
    m.BrakeDuration(100);
  end;

end.
