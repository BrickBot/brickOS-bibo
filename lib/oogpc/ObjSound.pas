unit ObjSound;

interface

uses
  types, dsound;

type
  TSound = object
    function Playing : boolean;
    procedure Beep;
    procedure Stop;
    procedure Duration(const len : unsigned_int);
    procedure Internote(const len : unsigned_int);
  end;

implementation

// TSound

function TSound.Playing : boolean;
begin
  result := dsound_playing;
end;

procedure TSound.Beep;
begin
  dsound_system(DSOUND_BEEP);
end;

procedure TSound.Stop;
begin
  dsound_stop;
end;

procedure TSound.Duration(const len : unsigned_int);
begin
  dsound_set_duration(len);
end;

procedure TSound.Internote(const len : unsigned_int);
begin
  dsound_set_internote(len);
end;

end.