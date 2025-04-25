unit Motor;

interface

uses
  types, conio, dmotor;

const
  m_min = 0;
  m_max = 255;

type
  TMotorPort = (mpA, mpB, mpC);
  ms_t = ^procedure(unsigned_char);
  md_t = ^procedure(MotorDirection);

  TMotor = object
  private
    f_ms : ms_t;
    f_md : md_t;
  public
    constructor Create(const port : TMotorPort);
    destructor  Destroy; virtual;
    procedure Speed(const s : unsigned_char);
    procedure Direction(const dir : MotorDirection);
    procedure SetForward;
    procedure ForwardSpeed(const s : unsigned_char);
    procedure Reverse;
    procedure ReverseSpeed(const s : unsigned_char);
    procedure Brake;
    procedure BrakeDuration(duration : int);
    procedure TurnOff;
  end;

implementation

{ TMotor }

procedure TMotor.Brake;
begin
  Direction(mdBrake);
end;

procedure TMotor.BrakeDuration(duration : int);
begin
  Brake;
  delay(duration);
end;

constructor TMotor.Create(const port: TMotorPort);
begin
  case port of
    mpA : begin
      f_ms := @motor_a_speed;
      f_md := @motor_a_dir;
    end;
    mpB : begin
      f_ms := @motor_b_speed;
      f_md := @motor_c_dir;
    end;
  else
    f_ms := @motor_c_speed;
    f_md := @motor_c_dir;
  end;
end;

destructor TMotor.Destroy;
begin
  TurnOff;
end;

procedure TMotor.Direction(const dir: MotorDirection);
begin
  f_md^(dir);
end;

procedure TMotor.SetForward;
begin
  Direction(mdFwd);
end;

procedure TMotor.ForwardSpeed(const s: unsigned_char);
begin
  SetForward;
  Speed(s);
end;

procedure TMotor.Reverse;
begin
  Direction(mdRev);
end;

procedure TMotor.ReverseSpeed(const s: unsigned_char);
begin
  Reverse;
  Speed(s);
end;

procedure TMotor.Speed(const s: unsigned_char);
begin
  f_ms^(s);
end;

procedure TMotor.TurnOff;
begin
  Direction(mdOff);
end;

end.