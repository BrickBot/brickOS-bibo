unit dlcd;

interface

uses
  types, bitops;

const
// LCD segment control byte and bit locations
// 0xNNNN,0xM => Mth bit (value 1<<M) of byte 0xNNNN
// overall memory range: 0xef43-0xef4b (9 bytes)

// the man
  LCD_MAN_BYTE  = $ef43;
  LCD_ARMS_BIT  = $0;
  LCD_BODY_BIT  = $1;
  LCD_1LEG_BIT  = $2;
  LCD_2LEGS_BIT = $3;

// the digits
// numbered right to left
// dots are associated with the digit to their left
// only digit 2,3,4 have dots
// digit 5 can only be used as a minus sign
  LCD_0_BYTE    = $ef44;
  LCD_0_TOP     = $4;
  LCD_0_MID     = $5;
  LCD_0_BOT     = $7;
  LCD_0_R_BYTE  = $ef45;
  LCD_0_TOPR    = $5;
  LCD_0_BOTR    = $7;
  LCD_0_L_BYTE  = $ef47;
  LCD_0_TOPL    = $5;
  LCD_0_BOTL    = $7;

  LCD_1_BYTE    = $ef43;
  LCD_1_TOP     = $4;
  LCD_1_MID     = $5;
  LCD_1_BOT     = $7;
  LCD_1_R_BYTE  = $ef47;
  LCD_1_TOPR    = $1;
  LCD_1_BOTR    = $3;
  LCD_1_L_BYTE  = $ef48;
  LCD_1_TOPL    = $5;
  LCD_1_BOTL    = $7;

  LCD_2_BYTE    = $ef44;
  LCD_2_TOP     = $0;
  LCD_2_MID     = $1;
  LCD_2_BOT     = $3;
  LCD_2_R_BYTE  = $ef48;
  LCD_2_TOPR    = $1;
  LCD_2_BOTR    = $3;
  LCD_2_L_BYTE  = $ef49;
  LCD_2_TOPL    = $5;
  LCD_2_BOTL    = $7;
  LCD_2_DOT_BYTE= $ef48;
  LCD_2_DOT     = $6;

  LCD_3_BYTE    = $ef46;
  LCD_3_TOP     = $0;
  LCD_3_MID     = $1;
  LCD_3_BOT     = $3;
  LCD_3_R_BYTE  = $ef4b;
  LCD_3_TOPR    = $1;
  LCD_3_BOTR    = $3;
  LCD_3_L_BYTE  = $ef4b;
  LCD_3_TOPL    = $5;
  LCD_3_BOTL    = $7;
  LCD_3_DOT_BYTE= $ef49;
  LCD_3_DOT     = $6;

  LCD_4_BYTE    = $ef46;
  LCD_4_TOP     = $4;
  LCD_4_MID     = $5;
  LCD_4_BOT     = $7;
  LCD_4_R_BYTE  = $ef4a;
  LCD_4_TOPR    = $1;
  LCD_4_BOTR    = $3;
  LCD_4_L_BYTE  = $ef4a;
  LCD_4_TOPL    = $5;
  LCD_4_BOTL    = $7;
  LCD_4_DOT_BYTE= $ef4b;
  LCD_4_DOT     = $6;

  LCD_5_BYTE    = $ef4a;
  LCD_5_MID     = $6;

// the motor displays
  LCD_A_S_BYTE = $ef4a;
  LCD_A_SELECT = $2;
  LCD_A_L_BYTE = $ef46;
  LCD_A_LEFT   = $6;
  LCD_A_R_BYTE = $ef46;
  LCD_A_RIGHT  = $2;

  LCD_B_S_BYTE = $ef43;
  LCD_B_SELECT = $6;
  LCD_B_L_BYTE = $ef44;
  LCD_B_LEFT   = $2;
  LCD_B_R_BYTE = $ef47;
  LCD_B_RIGHT  = $2;

  LCD_C_S_BYTE = $ef44;
  LCD_C_SELECT = $6;
  LCD_C_L_BYTE = $ef47;
  LCD_C_LEFT   = $6;
  LCD_C_R_BYTE = $ef45;
  LCD_C_RIGHT  = $6;

// the sensor displays
  LCD_S1_S_BYTE = $ef49;
  LCD_S1_SELECT = $0;
  LCD_S1_A_BYTE = $ef49;
  LCD_S1_ACTIVE = $1;

  LCD_S2_S_BYTE = $ef48;
  LCD_S2_SELECT = $4;
  LCD_S2_A_BYTE = $ef48;
  LCD_S2_ACTIVE = $0;

  LCD_S3_S_BYTE = $ef47;
  LCD_S3_SELECT = $4;
  LCD_S3_A_BYTE = $ef45;
  LCD_S3_ACTIVE = $4;

// the quartered circle
// quarter numbering is counterclockwise
// starts at top right
  LCD_CIRCLE_BYTE = $ef45;
  LCD_CIRCLE_0    = $0;
  LCD_CIRCLE_1    = $1;
  LCD_CIRCLE_2    = $2;
  LCD_CIRCLE_3    = $3;

// the dotted line
// dots numbered right to left
  LCD_DOT_0_BYTE = $ef49;
  LCD_DOT_0      = $4;
  LCD_DOT_1_BYTE = $ef4b;
  LCD_DOT_1      = $0;
  LCD_DOT_2_BYTE = $ef4b;
  LCD_DOT_2      = $4;
  LCD_DOT_3_BYTE = $ef4a;
  LCD_DOT_3      = $0;
  LCD_DOT_4_BYTE = $ef4a;
  LCD_DOT_4      = $0;

// the IR display
  LCD_IR_BYTE  = $ef49;
  LCD_IR_LOWER = $2;
  LCD_IR_UPPER = $3;

// the crossed out battery
  LCD_BATTERY_BYTE = $ef47;
  LCD_BATTERY_X    = $0;

// two empty bits
  LCD_EMPTY_1_BYTE = $ef48;
  LCD_EMPTY_1      = $2;
  LCD_EMPTY_2_BYTE = $ef4b;
  LCD_EMPTY_2      = $2;


procedure dlcd_show(b : punsigned_char; const i : byte);
procedure dlcd_hide(b : punsigned_char; const i : byte);
procedure dlcd_store(b : punsigned_char; const i : byte);

implementation

procedure dlcd_show(b : punsigned_char; const i : byte);
begin
  bit_set(b, i);
end;

procedure dlcd_hide(b : punsigned_char; const i : byte);
begin
  bit_clear(b, i);
end;

procedure dlcd_store(b : punsigned_char; const i : byte);
begin
  bit_store(b, i);
end;

end.
