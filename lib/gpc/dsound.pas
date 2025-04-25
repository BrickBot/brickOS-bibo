unit dsound;

interface

{$I config.inc}

{$IFDEF CONF_DSOUND}

uses
  types;

type
  Pnote_t = ^note_t;
  note_t = record
    pitch : unsigned_char;
    length : unsigned_char;
  end;
  PPnote_t = ^Pnote_t;

const
  // note length
  WHOLE   = 16;
  HALF    = 8;
  QUARTER = 4;
  EIGHTH  = 2;

  // pitches
  PITCH_A0   = 0;
  PITCH_Am0  = 1;
  PITCH_H0   = 2;
  PITCH_C1   = 3;
  PITCH_Cm1  = 4;
  PITCH_D1   = 5;
  PITCH_Dm1  = 6;
  PITCH_E1   = 7;
  PITCH_F1   = 8;
  PITCH_Fm1  = 9;
  PITCH_G1   = 10;
  PITCH_Gm1  = 11;
  PITCH_A1   = 12;
  PITCH_Am1  = 13;
  PITCH_H1   = 14;
  PITCH_C2   = 15;
  PITCH_Cm2  = 16;
  PITCH_D2   = 17;
  PITCH_Dm2  = 18;
  PITCH_E2   = 19;
  PITCH_F2   = 20;
  PITCH_Fm2  = 21;
  PITCH_G2   = 22;
  PITCH_Gm2  = 23;
  PITCH_A2   = 24;
  PITCH_Am2  = 25;
  PITCH_H2   = 26;
  PITCH_C3   = 27;
  PITCH_Cm3  = 28;
  PITCH_D3   = 29;
  PITCH_Dm3  = 30;
  PITCH_E3   = 31;
  PITCH_F3   = 32;
  PITCH_Fm3  = 33;
  PITCH_G3   = 34;
  PITCH_Gm3  = 35;
  PITCH_A3   = 36;
  PITCH_Am3  = 37;
  PITCH_H3   = 38;
  PITCH_C4   = 39;
  PITCH_Cm4  = 40;
  PITCH_D4   = 41;
  PITCH_Dm4  = 42;
  PITCH_E4   = 43;
  PITCH_F4   = 44;
  PITCH_Fm4  = 45;
  PITCH_G4   = 46;
  PITCH_Gm4  = 47;
  PITCH_A4   = 48;
  PITCH_Am4  = 49;
  PITCH_H4   = 50;
  PITCH_C5   = 51;
  PITCH_Cm5  = 52;
  PITCH_D5   = 53;
  PITCH_Dm5  = 54;
  PITCH_E5   = 55;
  PITCH_F5   = 56;
  PITCH_Fm5  = 57;
  PITCH_G5   = 58;
  PITCH_Gm5  = 59;
  PITCH_A5   = 60;
  PITCH_Am5  = 61;
  PITCH_H5   = 62;
  PITCH_C6   = 63;
  PITCH_Cm6  = 64;
  PITCH_D6   = 65;
  PITCH_Dm6  = 66;
  PITCH_E6   = 67;
  PITCH_F6   = 68;
  PITCH_Fm6  = 69;
  PITCH_G6   = 70;
  PITCH_Gm6  = 71;
  PITCH_A6   = 72;
  PITCH_Am6  = 73;
  PITCH_H6   = 74;
  PITCH_C7   = 75;
  PITCH_Cm7  = 76;
  PITCH_D7   = 77;
  PITCH_Dm7  = 78;
  PITCH_E7   = 79;
  PITCH_F7   = 80;
  PITCH_Fm7  = 81;
  PITCH_G7   = 82;
  PITCH_Gm7  = 83;
  PITCH_A7   = 84;
  PITCH_Am7  = 85;
  PITCH_H7   = 86;
  PITCH_C8   = 87;
  PITCH_Cm8  = 88;
  PITCH_D8   = 89;
  PITCH_Dm8  = 90;
  PITCH_E8   = 91;
  PITCH_F8   = 92;
  PITCH_Fm8  = 93;
  PITCH_G8   = 94;
  PITCH_Gm8  = 95;
  PITCH_A8   = 96;

  PITCH_PAUSE = 97;
  PITCH_MAX   = 98;
  PITCH_END   = 255;

  // system sounds
  DSOUND_BEEP    = 0;
  DSOUND_SYS_MAX = 1;

  // default duration of 1/16th note in ms
  DSOUND_DEFAULT_16th_ms = 200;

  // default duration internote spacing in ms
  DSOUND_DEFAULT_internote_ms = 15;

var
  dsound_16th_ms : unsigned; asmname 'dsound_16th_ms'; external;
  dsound_internote_ms : unsigned; asmname 'dsound_internote_ms'; external;
  dsound_next_note : Pnote_t; asmname 'dsound_next_note'; external;
  dsound_next_time : time_t; asmname 'dsound_next_time'; external;

//  dsound_system_sounds : PPnote_t; asmname 'dsound_system_sounds'; external;


procedure dsound_play(const notes : Pnote_t);
procedure dsound_system(nr : unsigned);
procedure dsound_set_duration(duration : unsigned);
procedure dsound_set_internote(duration : unsigned);
function dsound_playing : boolean;
function dsound_finished(data : wakeup_t) : wakeup_t; asmname 'dsound_finished';
procedure dsound_stop; asmname 'dsound_stop';

procedure dsound_handler; asmname 'dsound_handler';
procedure dsound_init; asmname 'dsound_init';
procedure dsound_shutdown; asmname 'dsound_shutdown';

{$ENDIF}

implementation

{$IFDEF CONF_DSOUND}

const
  sys_beep : array[0..1] of note_t = (
    ( PITCH_A4, 1 ),
    ( PITCH_END, 0 )
  );

  dsound_system_sounds : array[0..DSOUND_SYS_MAX-1] of Pnote_t = (
    @sys_beep[0]
  );


procedure dsound_play(const notes : Pnote_t);
begin
  dsound_next_note := notes;
  dsound_next_time := 0;
end;

procedure dsound_system(nr : unsigned);
begin
  if nr < DSOUND_SYS_MAX then
    dsound_play(dsound_system_sounds[nr]);
//    dsound_play( (dsound_system_sounds + (nr * sizeof(Pnote_t) ) )^ );
end;

procedure dsound_set_duration(duration : unsigned);
begin
  dsound_16th_ms := duration;
end;

procedure dsound_set_internote(duration : unsigned);
begin
  dsound_internote_ms := duration;
end;

function dsound_playing : boolean;
begin
  result := dsound_next_note <> nil;
end;

{$ENDIF}

end.
