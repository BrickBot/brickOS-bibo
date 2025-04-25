program rover2;

uses
  types, tm, conio, unistd, dsensor, dmotor, lcd;

function sensor_press_wakeup(data : wakeup_t) : wakeup_t;
begin
  lcd_refresh;
  if (SENSOR_1 < $F000) or (SENSOR_3 < $F000) then
    result := 1
  else
    result := 0;
end;

var
  dir : integer;
begin
  dir := 0;

  while true do
  begin
    motor_a_speed(2*MAX_SPEED div 3);
    motor_c_speed(2*MAX_SPEED div 3);

    motor_a_dir(mdFwd);
    motor_c_dir(mdFwd);

    cputs('fwwd ');

    wait_event(@sensor_press_wakeup, 0);

    if SENSOR_1 < $F000 then
      dir := 0
    else
      dir := 1;

    // back up
    motor_a_dir(mdRev);
    motor_c_dir(mdRev);

    cputs('rev  ');
    msleep(500);

    motor_a_speed(MAX_SPEED);
    motor_c_speed(MAX_SPEED);

    if dir = 1 then begin
      motor_c_dir(mdFwd);
      cputs('left ');
    end
    else begin
      motor_a_dir(mdFwd);
      cputs('right');
    end;

    msleep(500);

  end;

end.
