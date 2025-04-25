unit RotationSensor;

interface

uses
  types, dsensor, Sensor;

type
  TRotationSensor = object(TSensor)
  private
    f_rsensor : pint;
{$IFDEF CONF_DSENSOR_VELOCITY}
    f_rvelocity : pint;
{$ENDIF}
  public
    constructor Create(const port : TSensorPort);
    constructor CreatePos(const port : TSensorPort; position : int);
    destructor Destroy; virtual;
    procedure TurnOn; virtual;
    procedure TurnOff; virtual;
    procedure SetPos(position : int);
    function  GetPos : int;
{$IFDEF CONF_DSENSOR_VELOCITY}
    function  GetVelocity : int;
{$ENDIF}
  end;

implementation

{ TRotationSensor }

constructor TRotationSensor.Create(const port: TSensorPort);
begin
  CreatePos(port, 0);
end;

constructor TRotationSensor.CreatePos(const port: TSensorPort; position: int);
begin
  inherited Create(port, true);
  case port of
    spS1 : f_rsensor := @ds_rotations[2];
    spS2 : f_rsensor := @ds_rotations[1];
  else
    f_rsensor := @ds_rotations[0];
  end;
{$ifdef CONF_DSENSOR_VELOCITY}
  case port of
    spS1 : f_rvelocity := @ds_velocities[2];
    spS2 : f_rvelocity := @ds_velocities[1];
  else
    f_rvelocity := @ds_velocities[0];
  end;
{$endif}
  TurnOn;
  SetPos(position);
end;

destructor TRotationSensor.Destroy;
begin
  inherited Destroy;
end;

function TRotationSensor.GetPos: int;
begin
  result := f_rsensor^;
end;

{$IFDEF CONF_DSENSOR_VELOCITY}
function TRotationSensor.GetVelocity: int;
begin
  result := f_rvelocity^;
end;
{$ENDIF}

procedure TRotationSensor.SetPos(position: int);
begin
  ds_rotation_set(f_sensor, position);
end;

procedure TRotationSensor.TurnOff;
begin
  inherited TurnOff;
  ds_rotation_off(f_sensor);
end;

procedure TRotationSensor.TurnOn;
begin
  inherited TurnOn;
  ds_rotation_on(f_sensor);
end;

end.