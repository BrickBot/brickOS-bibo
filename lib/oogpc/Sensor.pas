unit Sensor;

interface

uses
  types, conio, dsensor;

type
  TSensorPort = (spS1, spS2, spS3, spBattery);

  TSensor = object
  protected
    f_sensor : punsigned;
  public
    constructor Create(const port : TSensorPort; makeActive : boolean);
    destructor Destroy; virtual;
    function  Get : unsigned; virtual;
    procedure Mode(makeActive : boolean);
    procedure Passive;
    procedure Active;
    procedure TurnOn; virtual;
    procedure TurnOff; virtual;
    procedure Strobe;
    procedure StrobeDelay(const ms : int);
    function  SampleDefault : unsigned_int; virtual;
    function  Sample(size : unsigned_int; wait : int) : unsigned_int; virtual;
  end;

implementation

procedure TSensor.Active;
begin
  ds_active(f_sensor);
end;

constructor TSensor.Create(const port : TSensorPort; makeActive : boolean);
begin
  case port of
    spS1 : f_sensor := @SENSOR_1;
    spS2 : f_sensor := @SENSOR_2;
    spS3 : f_sensor := @SENSOR_3;
  else
    f_sensor := @BATTERY;
  end;
  if makeActive then Active else Passive;
end;

destructor TSensor.Destroy;
begin
  TurnOff;
end;

function TSensor.Get : unsigned;
begin
  result := f_sensor^;
end;

procedure TSensor.Mode(makeActive: boolean);
begin
  if makeActive then Active else Passive;
end;

procedure TSensor.Passive;
begin
  ds_passive(f_sensor);
end;

procedure TSensor.TurnOn;
begin
  Active;
end;

procedure TSensor.TurnOff;
begin
  Passive;
end;

procedure TSensor.Strobe;
begin
  TurnOff;
  TurnOn;
end;

procedure TSensor.StrobeDelay(const ms : int);
begin
  TurnOff;
  delay(ms);
  TurnOn;
end;

function TSensor.SampleDefault : unsigned_int;
begin
  result := Sample(10, 2);
end;

function TSensor.Sample(size : unsigned_int; wait : int) : unsigned_int;
var
  sum, i : unsigned_int;
begin
  if size = 0 then
    size := 1;
  sum := Get;
  for i := 1 to size - 1 do
  begin
    inc(sum, Get);
    delay(wait);
  end;
  result := sum div size;
end;

end.