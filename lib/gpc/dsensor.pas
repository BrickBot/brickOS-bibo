unit dsensor;

interface

{$I config.inc}

{$IFDEF CONF_DSENSOR}

uses
  types, bitops;

var
  SENSOR_1 : unsigned; asmname 'AD_C'; external;
  SENSOR_2 : unsigned; asmname 'AD_B'; external;
  SENSOR_3 : unsigned; asmname 'AD_A'; external;
  BATTERY  : unsigned; asmname 'AD_D'; external;

var
  ds_activation : unsigned_char; asmname 'ds_activation'; external;
{$IFDEF CONF_DSENSOR_ROTATION}
  ds_rotation : unsigned_char; asmname 'ds_rotation'; external;
  ds_rotations : array[0..2] of int; asmname 'ds_rotations'; external;
{$ENDIF}
{$IFDEF CONF_DSENSOR_VELOCITY}
  ds_velocities : array[0..2] of int; asmname 'ds_velocities'; external;
{$ENDIF}
{$IFDEF CONF_DSENSOR_MUX}
  ds_mux : unsigned_char; asmname 'ds_mux'; external;
  ds_muxs : array[0..2,0..2] of int; asmname 'ds_muxs'; external;
{$ENDIF}

const
  LIGHT_RAW_BLACK = $ffc0;
  LIGHT_RAW_WHITE = $5080;

function ds_scale(x : unsigned) : unsigned;
function ds_unscale(x : unsigned) : unsigned;

function LIGHT(a : unsigned) : unsigned;
function LIGHT_MAX : unsigned;

function LIGHT_1 : unsigned;
function LIGHT_2 : unsigned;
function LIGHT_3 : unsigned;

{$IFDEF CONF_DSENSOR_ROTATION}
function ROTATION_1 : unsigned;
function ROTATION_2 : unsigned;
function ROTATION_3 : unsigned;
{$ENDIF}

{$IFDEF CONF_DSENSOR_VELOCITY}
function VELOCITY_1 : unsigned;
function VELOCITY_2 : unsigned;
function VELOCITY_3 : unsigned;
{$ENDIF}

{$IFDEF CONF_DSENSOR_MUX}
function SENSOR_1A : int;
function SENSOR_1B : int;
function SENSOR_1C : int;
function SENSOR_2A : int;
function SENSOR_2B : int;
function SENSOR_2C : int;
function SENSOR_3A : int;
function SENSOR_3B : int;
function SENSOR_3C : int;
{$ENDIF}

function TOUCH(a : unsigned) : boolean;

function TOUCH_1 : boolean;
function TOUCH_2 : boolean;
function TOUCH_3 : boolean;

procedure ds_active(sensor : punsigned);
procedure ds_passive(sensor : punsigned);
{$IFDEF CONF_DSENSOR_ROTATION}
procedure ds_rotation_set(sensor : punsigned; p : Integer); asmname 'ds_rotation_set';
procedure ds_rotation_on(sensor : punsigned);
procedure ds_rotation_off(sensor : punsigned);
{$ENDIF}

procedure ds_init; asmname 'ds_init';
procedure ds_shutdown; asmname 'ds_shutdown';
{$ENDIF}

implementation

{$IFDEF CONF_DSENSOR}
function ds_scale(x : unsigned) : unsigned;
begin
  result := x shr 6;
end;

function ds_unscale(x : unsigned) : unsigned;
begin
  result := x shl 6;
end;

function LIGHT(a : unsigned) : unsigned;
begin
  result := 147 - (ds_scale(a) div 7);
end;

function LIGHT_MAX : unsigned;
begin
  result := LIGHT(LIGHT_RAW_WHITE);
end;

function LIGHT_1 : unsigned;
begin
  result := LIGHT(SENSOR_1);
end;

function LIGHT_2 : unsigned;
begin
  result := LIGHT(SENSOR_2);
end;

function LIGHT_3 : unsigned;
begin
  result := LIGHT(SENSOR_3);
end;

{$IFDEF CONF_DSENSOR_ROTATION}
function ROTATION_1 : unsigned;
begin
  result := ds_rotations[2];
end;

function ROTATION_2 : unsigned;
begin
  result := ds_rotations[1];
end;

function ROTATION_3 : unsigned;
begin
  result := ds_rotations[0];
end;
{$ENDIF}

{$IFDEF CONF_DSENSOR_VELOCITY}
function VELOCITY_1 : unsigned;
begin
  result := ds_velocities[2];
end;

function VELOCITY_2 : unsigned;
begin
  result := ds_velocities[2];
end;

function VELOCITY_3 : unsigned;
begin
  result := ds_velocities[2];
end;
{$ENDIF}

{$IFDEF CONF_DSENSOR_MUX}
function SENSOR_1A : int;
begin
  result := ds_muxs[2][0];
end;

function SENSOR_1B : int;
begin
  result := ds_muxs[2][1];
end;

function SENSOR_1C : int;
begin
  result := ds_muxs[2][2];
end;

function SENSOR_2A : int;
begin
  result := ds_muxs[1][0];
end;

function SENSOR_2B : int;
begin
  result := ds_muxs[1][1];
end;

function SENSOR_2C : int;
begin
  result := ds_muxs[1][2];
end;

function SENSOR_3A : int;
begin
  result := ds_muxs[0][0];
end;

function SENSOR_3B : int;
begin
  result := ds_muxs[0][1];
end;

function SENSOR_3C : int;
begin
  result := ds_muxs[0][2];
end;

{$ENDIF}


function TOUCH(a : unsigned) : boolean;
begin
  result :=  a < $8000;
end;

function TOUCH_1 : boolean;
begin
  Result := TOUCH(SENSOR_1);
end;

function TOUCH_2 : boolean;
begin
  Result := TOUCH(SENSOR_2);
end;

function TOUCH_3 : boolean;
begin
  Result := TOUCH(SENSOR_3);
end;

procedure ds_active(sensor : punsigned);
begin
  if sensor = @SENSOR_3 then
    bit_set(@ds_activation, 0)
  else if sensor = @SENSOR_2 then
    bit_set(@ds_activation, 1)
  else if sensor = @SENSOR_1 then
    bit_set(@ds_activation, 2);
end;

procedure ds_passive(sensor : punsigned);
begin
  if sensor = @SENSOR_3 then
    bit_clear(@ds_activation, 0)
  else if sensor = @SENSOR_2 then
    bit_clear(@ds_activation, 1)
  else if sensor = @SENSOR_1 then
    bit_clear(@ds_activation, 2);
end;

{$IFDEF CONF_DSENSOR_ROTATION}
procedure ds_rotation_on(sensor : punsigned);
begin
  if sensor = @SENSOR_3 then
    bit_set(@ds_rotation, 0)
  else if sensor = @SENSOR_2 then
    bit_set(@ds_rotation, 1)
  else if sensor = @SENSOR_1 then
    bit_set(@ds_rotation, 2);
end;

procedure ds_rotation_off(sensor : punsigned);
begin
  if sensor = @SENSOR_3 then
    bit_clear(@ds_rotation, 0)
  else if sensor = @SENSOR_2 then
    bit_clear(@ds_rotation, 1)
  else if sensor = @SENSOR_1 then
    bit_clear(@ds_rotation, 2);
end;
{$ENDIF}

{$ENDIF}

end.
