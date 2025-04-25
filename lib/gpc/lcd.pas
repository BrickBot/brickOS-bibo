unit lcd;

interface

uses
  types, bitops;

const
  LCD_SHORT_CMD   = 0;
  LCD_LONG_CMD    = 3;
  LCD_DATA_OFFSET = 5;
  LCD_DATA_SIZE   = 9;

  LCD_MODE_SET    = $40;
  LCD_ENABLE      = $08;
  LCD_DISABLE     = $00;

  LCD_DEV_ID      = $7c;

  I2C_WRITE       = 0;
  I2C_READ        = 1;

  SCL             = 5;
  SDA             = 6;

var
  display_memory : array[0..12] of unsigned_char; asmname 'display_memory'; external;


procedure lcd_init; asmname 'lcd_init';
procedure lcd_power_on; asmname 'lcd_power_on'; 
procedure lcd_power_off; asmname 'lcd_power_off';
procedure lcd_refresh; asmname 'lcd_refresh';
procedure lcd_refresh_next_byte; asmname 'lcd_refresh_next_byte';

implementation

end.