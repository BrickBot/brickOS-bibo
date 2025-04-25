program oop;

uses
  conio, unistd, dsound, Fred, JCH;

type
  // TFred is an integer
  // interface
  TStateChanger = abstract object
    f_a : TFred;
    procedure setState(state : boolean); virtual;
    procedure enable; virtual; abstract;
    procedure disable; virtual; abstract;
    procedure onOff;
    constructor Init(a : TFred);
    destructor Done; virtual;
  end;

  // implementation using display
  TDisplayState = object(TStateChanger)
  public
    procedure enable; virtual;
    procedure disable; virtual;
    destructor Done; virtual;
  end;

  // implementation using sound
  TSoundState = object(TStateChanger)
  public
    procedure enable; virtual;
    procedure disable; virtual;
    destructor Done; virtual;
  end;

constructor TStateChanger.Init(a : TFred);
begin
  f_a := a;
  cputw(f_a);
  msleep(250);
  ClearTheScreen2;
end;

destructor TStateChanger.Done;
begin
  cputs('base');
  msleep(250);
  cls;
end;

procedure TStateChanger.setState(state : boolean);
begin
  if state then enable
           else disable;
end;

procedure TStateChanger.onOff;
begin
  setState(true);
  sleep(1);
  setState(false);
  sleep(1);
end;

procedure TDisplayState.enable;
begin
  cputs('on');
end;

procedure TDisplayState.disable;
begin
  cls;
end;

destructor TDisplayState.Done;
begin
  cputs('disp');
  msleep(250);
  cls;
  inherited Done;
end;

procedure TSoundState.enable;
begin
  dsound_system(DSOUND_BEEP);
end;

procedure TSoundState.disable;
begin
  // do nothing
end;

destructor TSoundState.Done;
begin
  cputs('sound');
  msleep(250);
  cls;
  inherited Done;
end;

function fredFred : integer;
begin
  result := 0;
end;

procedure AmazingGrace;
begin
  // do nothing
end;

type
  PDisplayState = ^TDisplayState;
  PSoundState   = ^TSoundState;

var
  S : ^TStateChanger;

begin
  S := New(PDisplayState, Init(4));
  S^.onOff;
  Dispose(S, Done);

  sleep(1);

  S := New(PSoundState, Init(2));
  S^.onOff;
  Dispose(S, Done);
end.
