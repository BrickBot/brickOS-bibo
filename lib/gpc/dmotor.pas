unit dmotor;

interface

{$I config.inc}

{$IFDEF CONF_DMOTOR}
uses
  types;

type
  MotorDirection = (mdOff, mdFwd, mdRev, mdBrake);

  CPart = record
    delta, sum : unsigned_char;
  end;

  AccessPart = record
    case Boolean of
      True : (assembler_part : unsigned);
      False : ( c_part : CPart);
  end;

  MotorState = record
    access : AccessPart;
    dir : unsigned_char;
  end;

const
  MIN_SPEED = 0;
  MAX_SPEED = 255;

var
  dm_a_pattern : array[MotorDirection] of unsigned_char; asmname 'dm_a_pattern'; external;
  dm_b_pattern : array[MotorDirection] of unsigned_char; asmname 'dm_b_pattern'; external;
  dm_c_pattern : array[MotorDirection] of unsigned_char; asmname 'dm_c_pattern'; external;

  dm_a : MotorState; asmname 'dm_a'; external;
  dm_b : MotorState; asmname 'dm_b'; external;
  dm_c : MotorState; asmname 'dm_c'; external;
  motor_controller : unsigned_char; asmname 'motor_controller'; external;

{$IFDEF CONF_VIS}
procedure motor_a_dir(dir : MotorDirection); asmname 'motor_a_dir';
procedure motor_b_dir(dir : MotorDirection); asmname 'motor_b_dir';
procedure motor_c_dir(dir : MotorDirection); asmname 'motor_c_dir';
{$ELSE}
procedure motor_a_dir(dir : MotorDirection);
procedure motor_b_dir(dir : MotorDirection);
procedure motor_c_dir(dir : MotorDirection);
{$ENDIF}

procedure motor_a_speed(speed : unsigned_char);
procedure motor_b_speed(speed : unsigned_char);
procedure motor_c_speed(speed : unsigned_char);

procedure dm_init; asmname 'dm_init';
procedure dm_shutdown; asmname 'dm_shutdown';

{$ENDIF}
implementation

{$IFDEF CONF_DMOTOR}

{$IFDEF CONF_VIS}
{$ELSE}
procedure motor_a_dir(dir : MotorDirection);
begin
  dm_a.dir := dm_a_pattern[dir];
end;

procedure motor_b_dir(dir : MotorDirection);
begin
  dm_b.dir := dm_b_pattern[dir];
end;

procedure motor_c_dir(dir : MotorDirection);
begin
  dm_c.dir := dm_c_pattern[dir];
end;
{$ENDIF}

procedure motor_a_speed(speed : unsigned_char);
begin
  dm_a.access.c_part.delta := speed;
end;

procedure motor_b_speed(speed : unsigned_char);
begin
  dm_b.access.c_part.delta := speed;
end;

procedure motor_c_speed(speed : unsigned_char);
begin
  dm_c.access.c_part.delta := speed;
end;

{$ENDIF}

end.