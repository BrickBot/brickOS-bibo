unit MotorPair;

interface

uses
  types, conio, dmotor, Motor;

type

  TMotorPair = object
  private
    mLeft : TMotor;
    mRight : TMotor;
  public
    constructor Create(lport, rport : TMotorPort);
    destructor Destroy; virtual;
    procedure Speed(const s : int);
    procedure Direction(const dir : MotorDirection);
    procedure SetForward;
    procedure ForwardSpeed(const s : int);
    procedure Reverse;
    procedure ReverseSpeed(const s : int);
    procedure Brake;
    procedure BrakeDuration(const d : int);
    procedure TurnOff;
    procedure Left;
    procedure LeftSpeed(const s : int);
    procedure PivotLeft;
    procedure PivotLeftSpeed(const s : int);
    procedure Right;
    procedure RightSpeed(const s : int);
    procedure PivotRight;
    procedure PivotRightSpeed(const s : int);
  end;

implementation

{ TMotorPair }

procedure TMotorPair.Brake;
begin
  mLeft.Direction(mdBrake);
  mRight.Direction(mdBrake);
end;

procedure TMotorPair.BrakeDuration(const d: int);
begin
  Brake;
  delay(d);
end;

constructor TMotorPair.Create(lport, rport: TMotorPort);
begin
  mLeft.Create(lport);
  mRight.Create(rport);
end;

destructor TMotorPair.Destroy;
begin
  mLeft.Destroy;
  mRight.Destroy;
end;

procedure TMotorPair.Direction(const dir: MotorDirection);
begin
  case dir of
    mdFwd : begin
      mLeft.Direction(mdFwd);
      mRight.Direction(mdRev);
    end;
    mdRev : begin
      mLeft.Direction(mdRev);
      mRight.Direction(mdFwd);
    end;
  else
    mLeft.Direction(dir);
    mRight.Direction(dir);
  end;
end;

procedure TMotorPair.ForwardSpeed(const s: int);
begin
  SetForward;
  Speed(s);
end;

procedure TMotorPair.Left;
begin
  mLeft.Direction(mdFwd);
  mRight.Direction(mdFwd);
end;

procedure TMotorPair.LeftSpeed(const s : int);
begin
  Left;
  Speed(s);
end;

procedure TMotorPair.PivotLeft;
begin
  mLeft.Brake;
  mRight.Direction(mdRev);
end;

procedure TMotorPair.PivotLeftSpeed(const s : int);
begin
  PivotLeft;
  Speed(s);
end;

procedure TMotorPair.PivotRight;
begin
  mLeft.Direction(mdFwd);
  mRight.Brake;
end;

procedure TMotorPair.PivotRightSpeed(const s : int);
begin
  PivotRight;
  Speed(s);
end;

procedure TMotorPair.Reverse;
begin
  Direction(mdRev);
end;

procedure TMotorPair.ReverseSpeed(const s: int);
begin
  Reverse;
  Speed(s);
end;

procedure TMotorPair.Right;
begin
  mLeft.Direction(mdRev);
  mRight.Direction(mdRev);
end;

procedure TMotorPair.RightSpeed(const s : int);
begin
  Right;
  Speed(s);
end;

procedure TMotorPair.SetForward;
begin
  Direction(mdFwd);
end;

procedure TMotorPair.Speed(const s: int);
begin
  mLeft.Speed(s);
  mRight.Speed(s);
end;

procedure TMotorPair.TurnOff;
begin
  mLeft.Direction(mdOff);
  mRight.Direction(mdOff);
end;

end.