unit types;

interface

type
  unsigned_short = Byte;     // 8 bit
  unsigned_char  = Byte;     // 8 bit
  unsigned       = word;     // 16 bit
  unsigned_int   = Word;     // 16 bit
  unsigned_long  = Longword; // 32 bit
//  unsigned_long  = Cardinal; // 32 bit
  long_int       = Longint;  // 32 bit signed
  int            = Smallint;  // 16 bit signed
//  int            = Integer;  // 16 bit signed
  signed_char    = Shortint;  // 8 bit signed

  punsigned_char = ^unsigned_char;
  punsigned      = ^unsigned;
  punsigned_int  = ^unsigned_int;
  pint           = ^int;

  PPChar         = ^PChar;

  time_t  = unsigned_long;
  size_t  = unsigned;
  Psize_t = ^size_t;

  priority_t = unsigned_char;
  tstate_t   = unsigned_char;
  tflags_t   = unsigned_char;

  tid_t = size_t;
  
  wakeup_t = unsigned_long;
  wakeupPtr = ^function(wakeup_t) : wakeup_t;

implementation

end.