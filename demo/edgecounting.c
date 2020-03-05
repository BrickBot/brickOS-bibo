#include <conio.h>
#include <unistd.h>
#include <dsensor.h>

int main(int argc, char **argv)
{
#ifdef CONF_DSENSOR_EDGECOUNT
  ds_passive(&SENSOR_1);
  ds_edgecount_on(&SENSOR_1);
  ds_edgecount_set_low(&SENSOR_1, 25);
  ds_edgecount_set_high(&SENSOR_1, 100);
  EDGECOUNT_1 = 0;

  while (!shutdown_requested())
  {
    lcd_int(EDGECOUNT_1);
    msleep(250);
  }

  cls();
  ds_edgecount_off(&SENSOR_1);
#endif
  return 0;
}
