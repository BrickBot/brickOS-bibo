// Plays "wir sind die roboter" theme by Kraftwerk.

#include <config.h>
#if defined(CONF_DSOUND)

#include <dsound.h>
#include <tm.h>

//! "wir sind die roboter" theme by Kraftwerk
static const note_t robots[] = { 
  { PITCH_D4,  QUARTER } , { PITCH_C4,  EIGHTH } , { PITCH_D4,  EIGHTH },
  { PITCH_F4,  EIGHTH } , { PITCH_D4,  EIGHTH } , { PITCH_D4,  QUARTER },
  { PITCH_F4,  QUARTER } , { PITCH_G4,  EIGHTH } , { PITCH_C5,  EIGHTH },
  { PITCH_A4,  QUARTER } , { PITCH_D4,  QUARTER } , { PITCH_END, 0 }
};
    
int main(int argc,char *argv[]) {
  while (!shutdown_requested()) {
    dsound_play(robots);
    dsound_wait();
  }
  return 0;
}
#else // CONF_DSOUND
#warning robots.c requires CONF_DSOUND
#warning robots demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // CONF_DSOUND
