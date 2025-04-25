unit LightSensor;

interface

uses
  types, dsensor, Sensor;

type
  TLightSensor = object(TSensor)
  public
    constructor Create(const port : TSensorPort);
    function  Get : unsigned; virtual;
  end;

implementation

{ TLightSensor }

constructor TLightSensor.Create(const port: TSensorPort);
begin
  inherited Create(port, true);
end;

function TLightSensor.Get: unsigned;
begin
  result := LIGHT(f_sensor^);
end;

end.