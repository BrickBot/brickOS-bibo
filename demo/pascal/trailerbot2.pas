program trailerbot2;

uses
  conio, stdlib, unistd, dmotor, dsensor, romlcd;

const
  ANGLES    = 6;
  MOVEMENTS = 6;

  FAR_RIGHT = 0;
  FAR_LEFT  = 4;

  HEAVEN    = 5;
  HEAVEN_REWARD = 20;

  TURN_MULTIPLIER = 2;

  EPSILON_MAX   = 0.60;
  EPSILON_MIN   = 0.10;
  EPSILON_DECAY = 0.02;

var
  epsilon_current : double = EPSILON_MAX;

const
  ALPHA = 0.20;
  GAMMA = 0.90;
  KAPPA = 10;

type
  movement = (mHARD_LEFT, mSOFT_LEFT, mFORWARD, mSOFT_RIGHT, mHARD_RIGHT, mREVERSE);

var
  steering_results : array[0..ANGLES-1, 0..MOVEMENTS-1, 0..ANGLES-1] of double;
  q_score : array[0..ANGLES-1, 0..MOVEMENTS-1] of double;

function norm_random : double;
var
  temp : double;
begin
  temp := random;
  temp := temp / 2147483647;
  result := temp;
end;

function norm_rotation(real_value : integer) : integer;
begin
  case real_value of
    -1, 0, 1, 2 : result := 0;
     3, 4, 5, 6 : result := 1;
     7, 8, 9, 10, 11, 12 : result := 2;
     13, 14, 15, 16 : result := 3;
     17, 18, 19, 20 : result := 4;
  else
    cputs('error');
    sleep(1);
    result := -1;
  end;
end;

procedure array_initialization;
var
  i, j, k : integer;
begin
  for i := 0 to ANGLES - 1 do begin
    for j := 0 to MOVEMENTS - 1 do begin
      for k := 0 to ANGLES - 1 do begin
        steering_results[i][j][k] := (1 / ANGLES);
      end;
    end;
  end;
  for i := 0 to ANGLES - 1 do begin
    for j := 0 to MOVEMENTS - 1 do begin
      q_score[i][j] := 0;
    end;
  end;
end;

procedure run_motors;
begin
  motor_a_speed(MAX_SPEED);
  motor_c_speed(MAX_SPEED);
end;

procedure go_forward;
begin
  motor_a_dir(mdFwd);
  motor_c_dir(mdFwd);
  run_motors;
  msleep(100*TURN_MULTIPLIER);
end;

procedure go_back;
begin
  motor_a_dir(mdRev);
  motor_c_dir(mdRev);
  run_motors;
  msleep(150*TURN_MULTIPLIER);
end;

procedure soft_left;
begin
  motor_a_dir(mdFwd);
  motor_c_dir(mdFwd);
  motor_a_speed(MAX_SPEED);
  motor_c_speed(MAX_SPEED div 2);
  msleep(75*TURN_MULTIPLIER);
end;

procedure soft_right;
begin
  motor_a_dir(mdFwd);
  motor_c_dir(mdFwd);
  motor_a_speed(MAX_SPEED div 2);
  motor_c_speed(MAX_SPEED);
  msleep(75*TURN_MULTIPLIER);
end;

procedure hard_right;
begin
  motor_a_dir(mdRev);
  motor_c_dir(mdFwd);
  motor_a_speed(MAX_SPEED);
  motor_c_speed(MAX_SPEED);
  msleep(100*TURN_MULTIPLIER);
end;

procedure hard_left;
begin
  motor_a_dir(mdFwd);
  motor_c_dir(mdRev);
  motor_a_speed(MAX_SPEED);
  motor_c_speed(MAX_SPEED);
  msleep(100*TURN_MULTIPLIER);
end;

procedure stop_motors;
begin
  motor_a_speed(0);
  motor_c_speed(0);
  motor_a_dir(mdBrake);
  motor_c_dir(mdBrake);

  msleep(500);
  motor_a_dir(mdOff);
  motor_c_dir(mdOff);
end;

procedure move;
var
  i, max_q_score, next_movement, initial_angle, new_angle : integer;
  temp : double;
begin
  max_q_score   := 0;
  next_movement := -1;
  initial_angle := norm_rotation(ROTATION_2);

  cputs('ANGLE');
  msleep(200);
  lcd_int(initial_angle);
  msleep(500);

  if norm_random > epsilon_current then begin
    cputs('real ');
    msleep(500);
    for i := 0 to MOVEMENTS - 1 do begin
      if q_score[initial_angle][i] > max_q_score then begin
        max_q_score := trunc(q_score[initial_angle][i]);
        next_movement := i;
      end;
    end;
  end
  else begin
    cputs('rand ');
    msleep(500);

    temp := norm_random;
    next_movement := trunc(temp * MOVEMENTS);

    lcd_int(next_movement);
    sleep(1);
  end;

  if next_movement = -1 then begin
    temp := norm_random;
    next_movement := trunc(temp * MOVEMENTS);
  end;

  case movement(next_movement) of
    mHARD_LEFT : begin
      cputs('HL   ');
      hard_left;
    end;
    mSOFT_LEFT : begin
      cputs('SL   ');
      soft_left;
    end;
    mFORWARD : begin
      cputs('FWD  ');
      go_forward;
    end;
    mSOFT_RIGHT : begin
      cputs('SR   ');
      soft_right;
    end;
    mHARD_RIGHT : begin
      cputs('HR   ');
      hard_right;
    end;
    mREVERSE : begin
      cputs('REV  ');
      go_back;
    end;
  else
    cputs('ERROR');
    sleep(1);
    stop_motors;
  end;

  stop_motors;

  msleep(500);

  new_angle := norm_rotation(ROTATION_2);

  cputs('NEW  ');
  msleep(200);
  lcd_int(new_angle);
  msleep(500);

  steering_results[initial_angle][next_movement][new_angle] :=
    steering_results[initial_angle][next_movement][new_angle] + ALPHA;

  for i := 0 to ANGLES - 1 do begin
    steering_results[initial_angle][next_movement][i] :=
      steering_results[initial_angle][next_movement][i] / (1 + ALPHA);
  end;

  if epsilon_current > EPSILON_MIN then
    epsilon_current := epsilon_current - EPSILON_DECAY;
end;

procedure q_score_update;
var
  i, j, k, l : integer;
  reward, q_sum, max_q_score : double;
begin
  for i := 0 to ANGLES - 1 do begin
    for j := 0 to MOVEMENTS - 1 do begin
      if (i >= FAR_LEFT) or (i <= FAR_RIGHT) or (j = Ord(mREVERSE)) then begin
        reward := 0;
      end
      else if i = HEAVEN then begin
        reward := HEAVEN_REWARD;
      end
      else begin
        reward := 1;
      end;

      q_sum := 0;

      for k := 0 to ANGLES - 1 do begin
        max_q_score := 0;
        for l := 0 to MOVEMENTS - 1 do begin
          if q_score[k][l] > max_q_score then
            max_q_score := q_score[k][l];
        end;
        q_sum := q_sum + (steering_results[i][j][k] * max_q_score);
      end;

      q_score[i][j] := reward + (GAMMA*q_sum);
    end;
  end;
end;

var
  k : integer;
begin
  ds_active(@SENSOR_1);
  msleep(100);
  srandom(LIGHT_1);
  ds_passive(@SENSOR_1);

  ds_active(@SENSOR_2);
  ds_rotation_on(@SENSOR_2);

  cputs('start');
  msleep(500);
  cputs('at   ');
  msleep(500);
  cputs('right');
  sleep(2);

  ds_rotation_set(@SENSOR_2, FAR_RIGHT);
  msleep(50);

  cputs('now  ');
  msleep(500);
  cputs('centr');
  sleep(1);

  array_initialization;

  while true do begin
    cputs('move');
    msleep(500);
    move;

    cputs('score');

    for k := 0 to KAPPA-1 do begin
      q_score_update;
    end;
  end;

  cls;

end.
