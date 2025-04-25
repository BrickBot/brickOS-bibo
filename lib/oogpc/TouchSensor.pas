unit TouchSensor;

interface

uses
  types, dsensor, Sensor;

type
  TTouchSensor = object(TSensor)
    constructor Create(const port : TSensorPort);
    function Pressed : boolean;
  end;

implementation

{ TTouchSensor }

constructor TTouchSensor.Create(const port: TSensorPort);
begin
  inherited Create(port, false);
end;

function TTouchSensor.Pressed: boolean;
begin
  result := TOUCH(f_sensor^);
end;

end.