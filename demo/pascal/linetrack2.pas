program linetrack2;

uses
  types, conio, tm, unistd, dsensor, dmotor;

{$DEFINE LIGHTSENS SENSOR_2}

const
  DARK_THRESH   = $40;
  BRIGHT_THRESH = $48;
  NORMAL_SPEED  = 2 * MAX_SPEED div 3;
  TURN_SPEED    = MAX_SPEED;

{.$DEFINE STRAIGHT_LINE}

function dark_found(data : wakeup_t) : wakeup_t;
begin
  if LIGHT(LIGHTSENS) < data then
    result := 1
  else
    result := 0;
end;

function bright_found(data : wakeup_t) : wakeup_t;
begin
  if LIGHT(LIGHTSENS) > data then
    result := 1
  else
    result := 0;
end;

procedure locate_line;
begin
  motor_a_speed(NORMAL_SPEED);
  motor_c_speed(NORMAL_SPEED);
  motor_a_dir(mdFwd);
  motor_c_dir(mdFwd);
  wait_event(@dark_found, DARK_THRESH);
end;

procedure follow_line;
var
  dir : integer;
begin
  dir := 0;
  while true do
  begin
    motor_a_speed(NORMAL_SPEED);
    motor_c_speed(NORMAL_SPEED);
    motor_a_dir(mdFwd);
    motor_c_dir(mdFwd);

    wait_event(@bright_found, BRIGHT_THRESH);

    if dir = 0 then
      motor_a_dir(mdRev)
    else
      motor_c_dir(mdRev);
{$IFDEF STRAIGHT_LINE}
    dir := not dir;
{$ENDIF}

    motor_a_speed(TURN_SPEED);
    motor_c_speed(TURN_SPEED);

    wait_event(@dark_found, DARK_THRESH);
  end;
end;

begin
  ds_active(@LIGHTSENS);

  locate_line;
  follow_line;
end.


