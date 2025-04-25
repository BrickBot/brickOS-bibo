unit TemperatureSensor;

interface

uses
  types, Sensor;

type
  TTemperatureSensor = object(TSensor)
  public
    constructor Create(const port : TSensorPort);
    function C : double;
    function F : double;
    function Tenths : int;
    function Degrees : int;
  end;

implementation

{ TTemperatureSensor }

function TTemperatureSensor.C: double;
begin
  result := 93.8136 - 0.122241 * double(Get shr 6);
end;

constructor TTemperatureSensor.Create(const port: TSensorPort);
begin
  inherited Create(port, false);
end;

function TTemperatureSensor.Degrees: int;
begin
  result := trunc(C);
end;

function TTemperatureSensor.F: double;
begin
  result := (C * 1.8) + 32;
end;

function TTemperatureSensor.Tenths: int;
begin
  result := trunc(C * 10.0);
end;

end.