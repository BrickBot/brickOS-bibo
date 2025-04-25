program sound2;

uses
  dsound, unistd;

const
  devil : array[0..40] of note_t = (
  ( PITCH_G4, QUARTER ),
  ( PITCH_G4, QUARTER ),
  ( PITCH_G4, QUARTER ),
  ( PITCH_G4, QUARTER ),
  ( PITCH_G4, HALF ),
  ( PITCH_G4, HALF ),
  ( PITCH_G4, HALF ),
  ( PITCH_G4, HALF ),
  ( PITCH_G4, HALF ),
  ( PITCH_G4, HALF ),
  ( PITCH_F4, QUARTER ),
  ( PITCH_F4, QUARTER ),
  ( PITCH_F4, QUARTER ),
  ( PITCH_F4, QUARTER ),
  ( PITCH_F4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_F4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_E4, HALF ),
  ( PITCH_E4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_E4, QUARTER ),
  ( PITCH_F4, HALF ),
  ( PITCH_F4, HALF ),
  ( PITCH_E4, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_PAUSE, HALF ),
  ( PITCH_END, 0 )
  );

begin
//  dsound_set_duration(40);

  while true do
  begin
    dsound_play(@devil[0]);
    wait_event(@dsound_finished, 0);
    sleep(1);
  end;
end.

