program robots2;

uses
  unistd, dsound;

const
  robots : array[0..11] of note_t = (
  ( PITCH_D4,  2 ) , ( PITCH_C4,  1 ) , ( PITCH_D4,  1 ),
  ( PITCH_F4,  1 ) , ( PITCH_D4,  1 ) , ( PITCH_D4,  2 ),
  ( PITCH_F4,  2 ) , ( PITCH_G4,  1 ) , ( PITCH_C5,  1 ),
  ( PITCH_A4,  2 ) , ( PITCH_D4,  2 ) , ( PITCH_END, 0 )
  );

begin
  while true do begin
    dsound_play(@robots[0]);
    wait_event(@dsound_finished, 0);
  end;
end.
