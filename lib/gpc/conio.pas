unit conio;

interface

uses
  types;

{$I config.inc}

{$IFDEF CONF_CONIO}

var
  hex_display_codes : array[0..15] of byte; asmname 'hex_display_codes'; external;
{$IFDEF CONF_ASCII}
  ascii_display_codes : array[0..127] of byte; asmname 'ascii_display_codes'; external;
{$ENDIF}

{$ENDIF}

procedure delay(d : unsigned); asmname 'delay';

{$IFDEF CONF_CONIO}
procedure cputc_native_0(mask : byte); asmname 'cputc_native_0';
procedure cputc_native_1(mask : byte); asmname 'cputc_native_1';
procedure cputc_native_2(mask : byte); asmname 'cputc_native_2';
procedure cputc_native_3(mask : byte); asmname 'cputc_native_3';
procedure cputc_native_4(mask : byte); asmname 'cputc_native_4';
procedure cputc_native_5(mask : byte); asmname 'cputc_native_5';

procedure cputc_native(mask : byte; p : integer); asmname 'cputc_native';

procedure cputc_hex_0(nibble : unsigned);
procedure cputc_hex_1(nibble : unsigned);
procedure cputc_hex_2(nibble : unsigned);
procedure cputc_hex_3(nibble : unsigned);
procedure cputc_hex_4(nibble : unsigned);
procedure cputc_hex_5(nibble : unsigned);

procedure cputc_hex(c : byte; p : integer);

procedure cputw(w : unsigned); asmname 'cputw';

{$IFDEF CONF_ASCII}
procedure cputc_0(c : char);
procedure cputc_1(c : char);
procedure cputc_2(c : char);
procedure cputc_3(c : char);
procedure cputc_4(c : char);
procedure cputc_5(c : char);

procedure cputc(c : char; p : integer);

procedure cputs(const s : pchar); asmname 'cputs';
procedure cls; asmname 'cls';
{$ELSE}
procedure cls;
{$ENDIF}
{$ELSE}
procedure cls;
{$ENDIF}

implementation

{$IFDEF CONF_CONIO}

procedure cputc_hex_0(nibble : unsigned);
begin
  cputc_native_0(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex_1(nibble : unsigned);
begin
  cputc_native_1(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex_2(nibble : unsigned);
begin
  cputc_native_2(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex_3(nibble : unsigned);
begin
  cputc_native_3(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex_4(nibble : unsigned);
begin
  cputc_native_4(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex_5(nibble : unsigned);
begin
  cputc_native_5(hex_display_codes[nibble and $0F]);
end;

procedure cputc_hex(c : byte; p : integer);
begin
  cputc_native(hex_display_codes[c and $7f], p);
end;

{$IFDEF CONF_ASCII}
procedure cputc_0(c : char);
begin
  cputc_native_0(ascii_display_codes[byte(c) and $7f]);
end;

procedure cputc_1(c : char);
begin
  cputc_native_1(ascii_display_codes[byte(c) and $7f]);
end;

procedure cputc_2(c : char);
begin
  cputc_native_2(ascii_display_codes[byte(c) and $7f]);
end;

procedure cputc_3(c : char);
begin
  cputc_native_3(ascii_display_codes[byte(c) and $7f]);
end;

procedure cputc_4(c : char);
begin
  cputc_native_4(ascii_display_codes[byte(c) and $7f]);
end;

procedure cputc_5(c : char);
begin
  cputc_native_5(ascii_display_codes[byte(c) and $7f]);
end;


procedure cputc(c : char; p : integer);
begin
  cputc_native(ascii_display_codes[byte(c) and $7f], p);
end;

{$ELSE}

procedure cls;
begin
  lcd_clear;
end;

{$ENDIF}

{$ELSE}

procedure cls;
begin
  lcd_clear;
end;

{$ENDIF}

end.